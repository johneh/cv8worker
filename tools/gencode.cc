#include <string>
#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

static llvm::cl::OptionCategory toolCategory("gencode options");

static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static llvm::cl::extrahelp MoreHelp(
    "Example Usage:\n"
    "\n"
    "\t./gencode test.h --\n"
    "\n"
    "\t./gencode -strip cairo_ /usr/include/cairo/cairo.h --\n"
    "\n"
);

//static cl::opt<bool>      boolOpt ("b", llvm::cl::desc("..."));
//static cl::opt<unsigned int> uiOpt ("u",  llvm::cl::desc("..."));
static llvm::cl::opt<std::string>  stripPrefix ("strip",
        llvm::cl::desc("prefix to remove from names"));
//static llvm::cl::opt<std::string>  beginCode("start",
//        llvm::cl::desc("code to add before starting output"));

#define ASSERT(x) \
    do {\
        if (!(x)) {\
            fprintf(stderr, "Assert failed: " #x " (%s:%d)\n",\
                __FILE__, __LINE__);\
            fflush(stderr);\
            abort();\
        }\
    } while (0)


enum class JsType {
    Void,
    Int,
    Uint,
    Double,
    String,     // char *
    VoidPtr,    // void *
    Ptr         // int *, struct foo *, void **, char ** etc.
};

static JsType ParseBuiltinType(const BuiltinType* bltIn) {
    switch(bltIn->getKind()) {
    case BuiltinType::Void:
        return JsType::Void;
    case BuiltinType::Bool:
    case BuiltinType::SChar: /* signed char */
    case BuiltinType::Char_S: /* char */
    case BuiltinType::UChar:
    case BuiltinType::Char_U:

    case BuiltinType::Short:
    case BuiltinType::UShort:
    case BuiltinType::Int:
        return JsType::Int;
    case BuiltinType::UInt:
        return JsType::Uint;

    case BuiltinType::Long:
        if (sizeof (long) == 4)
            return JsType::Int;
        return JsType::Double;
    case BuiltinType::ULong:
        if (sizeof (unsigned long) == 4)
            return JsType::Uint; 
    case BuiltinType::LongLong:
    case BuiltinType::ULongLong:
    case BuiltinType::Float:
    case BuiltinType::Double:
        return JsType::Double;

    default: break;
    }

    ASSERT(0 && "Unhandled builtin type");
}

static JsType ParseType(QualType qType) {

    ASSERT(!qType.isNull() && "Expected non-null QualType");
    const Type* cType = qType.getTypePtr();
    ASSERT(cType && "Expected a valid type");

    switch(cType->getTypeClass()) {
    case Type::Builtin: {
        const BuiltinType *bltIn = cType->getAs<clang::BuiltinType>();
        return ParseBuiltinType(bltIn);
    }
    case Type::Enum: {
        // const EnumType *enumType = cType->getAs<clang::EnumType>();
        // EnumDecl* enumDecl = enumType->getDecl();
        // cout << "Type::Enum ..."<< enumDecl->getIntegerType().getAsString()<<"\n";
        return JsType::Int;
    }
    case Type::Pointer: {
        const PointerType *ptrType = cType->getAs<clang::PointerType>();
        QualType pointee = ptrType->getPointeeType();
        const Type *pointeeType = pointee.getTypePtr();
        if (pointeeType->getTypeClass() == Type::Builtin) {
            // int *. char *, void * but not void **
            const BuiltinType *bltIn = pointeeType->getAs<clang::BuiltinType>();
            if (bltIn->getKind() == BuiltinType::SChar
                || bltIn->getKind() == BuiltinType::Char_S
            ) {
                //cout << "pointee is char\n";
                return JsType::String;
            }
            else if (bltIn->getKind() == BuiltinType::Void) {
                //cout << "pointee is void\n";
                return JsType::VoidPtr;
            }
        } else if (pointeeType->getTypeClass() == Type::Pointer) {
            // pointee itself is a pointer
            ;
        }
        return JsType::Ptr;
    }
    case Type::Typedef: {
        const TypedefType *tyType = cType->getAs<clang::TypedefType>();
        QualType uQType = tyType->getDecl()->getUnderlyingType();
        return ParseType(uQType);
    }
    case Type::Elaborated: {
        const ElaboratedType *elaboratedType = cType->getAs<clang::ElaboratedType>();
        return ParseType(elaboratedType->getNamedType());
    }
    case Type::Record:
        cerr<<"error: struct or union as argument or return value.\n";
        exit(1);
    default:
        cerr << "error: unsupported typeclass: "
                    << cType->getTypeClassName()<<"\n";
        exit(1);
    }
}

struct CEnumItem {
    std::string name;
    int value;
    CEnumItem(std::string n, int v): name(n), value(v) {}
};

struct CEnum {
    std::string enumName;
    std::string typedefName;
    std::vector<CEnumItem *>items;
    CEnum(): enumName(), typedefName(), items() {}
    ~CEnum() {
        for(unsigned k = 0; k < items.size(); k++) {
            delete items[k];
        }
    }
};

// RecursiveASTVisitor does a pre-order depth-first traversal of the AST
class CVisitor : public RecursiveASTVisitor<CVisitor> {
public:
    explicit CVisitor(SourceManager &s, std::vector<CEnum *> &e,
                std::string &fns, std::string &file)
        : m_sm(s), m_enums(e), m_fns(fns), m_file(file) {}

    bool VisitDecl(Decl *decl) {
        FullSourceLoc fullLoc(decl->getLocStart(), m_sm);
        if (!fullLoc.isValid())
            return true;
        const std::string &fileName = m_sm.getFilename(fullLoc);
        if (fileName != m_file) // skip included headers
            return true;

        if (decl->getKind() == Decl::Enum) {
            const EnumDecl *enumDecl = dyn_cast<EnumDecl>(decl);
#if 0
            const std::string &fileName = m_sm.getFilename(fullLoc);
            cout << "enum " << fileName <<":"<<fullLoc.getSpellingLineNumber()<<"\n";
#endif
            CEnum *ce = FindEnumByName(enumDecl->getNameAsString());
            if (!ce) {
                ce = new CEnum();
                ce->enumName = enumDecl->getNameAsString();
                m_enums.push_back(ce);
            }
            for (EnumDecl::enumerator_iterator enumerator = enumDecl->enumerator_begin();
                        enumerator != enumDecl->enumerator_end(); ++enumerator) {
                // const QualType type(enumerator->getType());
                // cout << type.getAsString(); // => "int"
                // enumerator->getInitVal().toString(10) <<"\n";
                ce->items.push_back(
                    new CEnumItem(enumerator->getNameAsString(),
                            static_cast<int>(enumerator->getInitVal().getExtValue())
                        )
                );
            }
        }
        else if (decl->getKind() == Decl::Typedef) {
            const TypedefDecl *typedefDecl = dyn_cast<TypedefDecl>(decl);
#if 0
            const std::string &fileName = m_sm.getFilename(fullLoc);
            cout << "typedef "<< fileName <<":"<<fullLoc.getSpellingLineNumber()<<"\n";
#endif

            QualType qType = typedefDecl->getUnderlyingType();
            const Type *cType = qType.getTypePtrOrNull();
            if (cType) {
                TagDecl *tagDecl = cType->getAsTagDecl();
                if (tagDecl && tagDecl->getKind() == Decl::Enum) {
                    const EnumDecl *enumDecl = dyn_cast<clang::EnumDecl>(tagDecl);
                    //cout << "typedef enum ... "<< enumDecl->getNameAsString()<<"\n";
                    CEnum *ce = FindEnumByName(enumDecl->getNameAsString());
                    if (!ce && m_enums.size() > 0) {
                        // e.g.: typedef enum { .. } color_t;
                        // must be the last entry with empty name?
                        ce = m_enums[m_enums.size() - 1];
                        ASSERT(ce->enumName == "" && ce->typedefName == "");
                    }
                    ASSERT(ce);
                    ce->typedefName = typedefDecl->getNameAsString();
                }
            }
        }
        else if (decl->getKind() == Decl::Function) {
            const FunctionDecl *funcDecl = dyn_cast<FunctionDecl>(decl);
            if (!funcDecl->isThisDeclarationADefinition()
                    && funcDecl->getStorageClass() != clang::StorageClass::SC_Static
            ) {
#if 0
                const std::string &fileName = m_sm.getFilename(fullLoc);
                cout << fileName <<":"<<fullLoc.getSpellingLineNumber()<<"\n";
#endif
                const std::string fName = funcDecl->getNameAsString();
                // cout << funcDecl->getReturnType().getAsString() << "\n";
                MakeWrapStart(fName, funcDecl->getNumParams());
                for (unsigned i = 0, j = funcDecl->getNumParams(); i != j; ++i) {
                    const ParmVarDecl *paramDecl = funcDecl->getParamDecl(i);
                    // parameter name paramDecl->getName(), maybe empty ( == "")
                    MakeWrapParam(ParseType(paramDecl->getOriginalType()), i);
                }
                MakeWrapEnd(ParseType(funcDecl->getReturnType()),
                            fName, funcDecl->getNumParams());
            }
        }
        return true;
    }

protected:
    CEnum *FindEnumByName(const std::string& name) {
        if (name == "")
            return nullptr;
        for (unsigned k = 0; k < m_enums.size(); k++) {
            CEnum *ce = m_enums[k];
            if (ce->enumName == name)
                return ce;
        }
        return nullptr;
    }

    CEnum *FindEnumByTypedefName(const std::string& name) {
        ASSERT(name != "");
        for (unsigned k = 0; k < m_enums.size(); k++) {
            CEnum *ce = m_enums[k];
            if (ce->typedefName == name)
                return ce;
        }
        return nullptr;
    }

    void MakeWrapStart(const std::string& fName, unsigned int numParams) {
        const char *fname = fName.c_str();
        const char *cname = fname;
        if (stripPrefix != "") {
            if (fName.find(stripPrefix) == 0)
                cname = fname + stripPrefix.size();
        }

        fprintf(stdout,
"static void do_%s(js_vm *vm, int argc, js_val argv) {\n",
            cname);

        // the struct entry for the table of functions
        char buf[256];
        size_t n = snprintf(buf, 256, "{ %d, do_%s, \"%s\" },\n",
                            numParams, cname, cname);
        ASSERT(n < 256);
        m_fns.append(buf);
    }

    void MakeWrapParam(JsType jsType, int paramNum) {
        switch (jsType) {
        case JsType::Int:
            fprintf(stdout, "int p%d = js_dl->to_int(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::Uint:
            fprintf(stdout, "unsigned int p%d = js_dl->to_uint(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::Double:
            fprintf(stdout, "double p%d = js_dl->to_double(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::String:
            fprintf(stdout, "char *p%d = js_dl->to_string(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::VoidPtr:
        case JsType::Ptr:
            fprintf(stdout, "void *p%d = js_dl->to_pointer(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        default:
            ASSERT(0 && "MakeWrapParam(): received unexpected type");
        }
    }

    void MakeWrapEnd(JsType jsType,
            const std::string& fName, unsigned int numParams) {
        std::string ends;
        const char *fname = fName.c_str();
        fprintf(stdout, "if (js_dl->errstr(vm)) return;\n");
        switch (jsType) {
        case JsType::Void:
            fprintf(stdout, "%s(", fname);
            ends = "";
            break;
        case JsType::Int:
            fprintf(stdout, "int r = (int) %s(", fname);
            ends = "js_dl->from_int(vm, r, argv);\n";
            break;
        case JsType::Uint:
            fprintf(stdout, "unsigned int r = (unsigned int) %s(", fname);
            ends = "js_dl->from_uint(vm, r, argv);\n";
            break;
        case JsType::Double:
            fprintf(stdout, "double r = (double) %s(", fname);
            ends = "js_dl->from_double(vm, r, argv);\n";
            break;
        case JsType::String:
        case JsType::VoidPtr:
        case JsType::Ptr:
            fprintf(stdout, "void *r = (void *) %s(", fname);
            ends = "js_dl->from_pointer(vm, r, argv);\n";
            break;
        default:
            ASSERT(0 && "MakeWrapEnd(): received unexpected type");
        }
        for (unsigned int i = 0; i < numParams; i++) {
            if (i > 0) fprintf(stdout, ",");
            fprintf(stdout, "p%d", i);
        }
        fprintf(stdout, ");\n%s}\n", ends.c_str());
    }

private:
    SourceManager &m_sm;
    std::vector<CEnum *>&m_enums;
    std::string &m_fns;
    std::string m_file;
};


class CConsumer : public ASTConsumer {
public:
    explicit CConsumer(SourceManager &s, std::vector<CEnum *> &enums,
                std::string &fns, std::string file)
        : m_visitor(CVisitor(s, enums, fns, file)) {}

    // Called by the parser for each top-level declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef dg) override {
        for (Decl *decl : dg) {
            m_visitor.TraverseDecl(decl);
        }
        return true;
    }

private:
    CVisitor m_visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class CFrontendAction : public ASTFrontendAction {
public:
    CFrontendAction() {}

    bool BeginSourceFileAction(CompilerInstance &CI, StringRef Filename) override {
        StringRef hName = Filename;
        size_t pos = Filename.rfind('/');
        if (pos != StringRef::npos)
            hName = Filename.substr(pos+1);

        fprintf(stdout, "#define JS_DLL 1\n#include \"jsv8dlfn.h\"\n");
//        if (beginCode != "") {
//            fprintf(stdout, "%s\n", beginCode.c_str());
//        }
        if (hName.endswith(".h"))
            fprintf(stdout, "#include <%s>\n", hName.data());        
        fprintf(stdout, "\n");
        return true;
    }
 
    void EndSourceFileAction() override {
        fprintf(stdout, "\nstatic js_ffn fntab_[] = {\n%s{0}\n};\n", m_fns.c_str());
        fprintf(stdout, "static const char source_str_[] = \"(function(){\\\n");
        for (unsigned k = 0; k < m_enums.size(); k++) {
            CEnum *ce = m_enums[k];
            if (ce->items.size() == 0)
                continue;
            const std::string& eName = ce->typedefName != "" ? ce->typedefName
                            : ce->enumName;
            const char *name = eName.c_str();
            if (stripPrefix != "") {
                if (eName.find(stripPrefix) == 0)
                    name = name + stripPrefix.size();
            }
#if 0
            // enum type as object with enumerators as properties
            fprintf(stdout, "this.%s = {\\\n", name);
            for (unsigned i = 0; i < ce->items.size(); i++) {
                fprintf(stdout, "%s : %d,\\\n",
                    ce->items[i]->name.c_str(), ce->items[i]->value);
            }

            fprintf(stdout, "};\\\n");
#endif
            for (unsigned i = 0; i < ce->items.size(); i++) {
                fprintf(stdout, "this.%s = %d;\\\n",
                    ce->items[i]->name.c_str(), ce->items[i]->value);
            }
        }
        fprintf(stdout, "});\";\n\n");

        fprintf(stdout, "int JS_LOAD(js_vm *vm, js_val vobj) {\n");
        fprintf(stdout, "js_dl = dl_;\n");
        fprintf(stdout, "int rc = dl_->call_str(vm, source_str_, vobj);\n");
        fprintf(stdout, "if (!rc) return -1;\n");
        fprintf(stdout, "JS_EXPORT(fntab_);\n}\n");
    }

    // Create the ASTConsumer that will be used by this action.
    // The StringRef parameter is the current input filename.
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci,
                                llvm::StringRef file) override {
        return llvm::make_unique<CConsumer>(
                        ci.getSourceManager(), m_enums, m_fns, file.str());
    }

    ~CFrontendAction() {
        for (unsigned k = 0; k < m_enums.size(); k++) {
            delete m_enums[k];
        }
    }

private:
    std::vector<CEnum *> m_enums;
    std::string m_fns;
};


// N.B.: run with a trailing -- (./gencode filename --) to avoid seeing the
// garbage spewed by clang.

int main(int argc, const char **argv) {
    CommonOptionsParser op(argc, argv, toolCategory);
    ClangTool tool(op.getCompilations(), op.getSourcePathList());
    return tool.run(newFrontendActionFactory<CFrontendAction>().get());
}


