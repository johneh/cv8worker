#include <string>
#include <set>
#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/RecordLayout.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

static std::set<std::string> split(const char *str, char c = ' ');

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
static llvm::cl::opt<bool>  useLong ("long",
        llvm::cl::desc("use 64 bit integer"));
static llvm::cl::opt<std::string>  excludeFuncs ("nowrap",
        llvm::cl::desc("do not create wrapper for function (use : to seperate names)"));
std::set<std::string> skipFuncs;

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
    Long,
    Ulong,
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
        if (useLong)
            return JsType::Long;
        return JsType::Double;
    case BuiltinType::ULong:
        if (sizeof (unsigned long) == 4)
            return JsType::Uint;
        if (useLong)
            return JsType::Ulong;
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

    switch(qType->getTypeClass()) {
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
                return JsType::String;
            }
            else if (bltIn->getKind() == BuiltinType::Void) {
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

struct CRecord {
    std::string recName;
    std::string typedefName;
    unsigned size;
    bool isanon;
    std::string offsets;
    CRecord(): recName(), typedefName(), size(0), isanon(false), offsets() {}
    CRecord(const CRecord *cr1, const std::string &ts) : recName(cr1->recName),
            typedefName(ts), size(cr1->size),
            isanon(cr1->isanon), offsets(cr1->offsets) {}
};

// RecursiveASTVisitor does a pre-order depth-first traversal of the AST
class CVisitor : public RecursiveASTVisitor<CVisitor> {
public:
    explicit CVisitor(SourceManager &s, std::vector<CEnum *> &e,
                std::vector<CRecord *> &r,
                std::string &fns, std::string &file)
        : m_sm(s), m_enums(e), m_records(r), m_fns(fns), m_file(file) {}

    bool VisitDecl(Decl *decl) {
        FullSourceLoc fullLoc(decl->getLocStart(), m_sm);
        if (!fullLoc.isValid())
            return true;
        const std::string &fileName = m_sm.getFilename(fullLoc);
        if (fileName != m_file) // Skip all included headers.
            return true;

        if (decl->getKind() == Decl::Enum) {
            const EnumDecl *enumDecl = dyn_cast<EnumDecl>(decl);
#if 0
            const std::string &fileName = m_sm.getFilename(fullLoc);
            cout << "enum " << fileName <<":"<<fullLoc.getSpellingLineNumber()<<"\n";
#endif

            // Uses typedef name for an anonymous enum.
            CEnum *ce = getCEnum(enumDecl);

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
            if (TagDecl *tagDecl = qType->getAsTagDecl()) {
                if (tagDecl->getKind() == Decl::Enum) {
                    CEnum *ce = findCEnumByName(tagDecl->getNameAsString());
                    if (! ce) {
                        ce = findCEnumByTypedefName(typedefDecl->getNameAsString());
                        ASSERT(ce);
                    } else {
                        ce->typedefName = typedefDecl->getNameAsString();
                    }
                } else if (tagDecl->getKind() == Decl::Record) {
                    // XXX: class TagDecl: isUnion(), isStruct(), isEnum()
                    if (findCRecordByTypedefName(typedefDecl->getNameAsString())) {
                        return true;
                    }

                    if (tagDecl->getNameAsString() == "") {
                        // Anonymous record:
                        //  typedef { .. } t1; typedef t1 t2;
                        // FIXME -- definition need not be complete (seen yet).
                        TypedefNameDecl *tD1 = tagDecl->getTypedefNameForAnonDecl();
                        ASSERT(tD1);
                        CRecord *cr1 = findCRecordByTypedefName(tD1->getNameAsString());
                        ASSERT(cr1);
                        m_records.push_back(
                                new CRecord(cr1, typedefDecl->getNameAsString())
                        );
                        return true;
                    }
                    CRecord *cr = getCRecord(tagDecl->getNameAsString());
                    // FIXME -- multiple typedef names.
                    cr->typedefName = typedefDecl->getNameAsString();
                }
            }
        }
        else if (decl->getKind() == Decl::Function) {
            const FunctionDecl *funcDecl = dyn_cast<FunctionDecl>(decl);
            if (!funcDecl->isThisDeclarationADefinition()
                    && funcDecl->getStorageClass() != clang::StorageClass::SC_Static
            ) {
#if 0
                // __attribute__((annotate("coroutine"))) .. 
                if (decl->hasAttrs()) {
        for (clang::Decl::attr_iterator attr = decl->attr_begin(),
			 aEnd = decl->attr_end(); attr != aEnd; ++attr) {

			if (const clang::AnnotateAttr *aa = dyn_cast<clang::AnnotateAttr>(*attr)) {
                // StringRef shit: instead of str() for std::string,
                // use data() and size() for C string
                //
				cout << '"' << aa->getAnnotation().str() << "\" [" <<
				  aa->getAnnotationLength() << "]\t";
			}
		}
        cout<<"\n\n";
                }
#endif

#if 0
                const std::string &fileName = m_sm.getFilename(fullLoc);
                cout << fileName <<":"<<fullLoc.getSpellingLineNumber()<<"\n";
#endif
                const std::string fName = funcDecl->getNameAsString();

                if (skipFuncs.count(fName) > 0)
                    return true;

                MakeWrapStart(fName, funcDecl->getNumParams());
                for (unsigned i = 0, j = funcDecl->getNumParams(); i != j; ++i) {
                    const ParmVarDecl *paramDecl = funcDecl->getParamDecl(i);
                    // parameter name paramDecl->getName(), maybe empty ( == "")
                    MakeWrapParam(ParseType(paramDecl->getOriginalType()), i);
                }
                MakeWrapEnd(ParseType(funcDecl->getReturnType()),
                            fName, funcDecl->getNumParams());
            }
        } else if (decl->getKind() == Decl::Record) {
            RecordDecl *recDecl = dyn_cast<RecordDecl>(decl);
            std::string recName = recDecl->getNameAsString();
            int isAnonRecord = false;
            if (recName == "") {
                recName = generateAnonName(recDecl);
                isAnonRecord = true;
            }
            CRecord *cr = getCRecord(recName);
            cr->isanon = isAnonRecord;
            if (isAnonRecord) {
                if (TypedefNameDecl *tD = recDecl->getTypedefNameForAnonDecl()) {
                    cr->typedefName = tD->getNameAsString();
                }
            }
            recDecl = recDecl->getDefinition();
            if (!recDecl)
                return true;
            const ASTRecordLayout &recLayout = recDecl
                        ->getASTContext().getASTRecordLayout(recDecl);
            cr->size = recLayout.getSize().getQuantity();
            cr->offsets = getFieldOffsets(recDecl, recLayout);
        }
        return true;
    }

protected:
    CEnum *findCEnumByName(const std::string& name) {
        if (name == "")
            return nullptr;
        for (unsigned k = 0; k < m_enums.size(); k++) {
            CEnum *ce = m_enums[k];
            if (ce->enumName == name)
                return ce;
        }
        return nullptr;
    }

    CEnum *findCEnumByTypedefName(const std::string& name) {
        ASSERT(name != "");
        for (unsigned k = 0; k < m_enums.size(); k++) {
            CEnum *ce = m_enums[k];
            if (ce->typedefName == name)
                return ce;
        }
        return nullptr;
    }

    CEnum *getCEnum(const EnumDecl *eD) {
        CEnum *ce = nullptr;
        std::string name = eD->getNameAsString();
        bool isTypedefName = false;
        if (name != "") {
            if ((ce = findCEnumByName(name)))
                return ce;
        } else {
            TypedefNameDecl *tD = eD->getTypedefNameForAnonDecl();
            if (tD && (name = tD->getNameAsString()) != ""
                    && (ce = findCEnumByTypedefName(name)))
                return ce;
            isTypedefName = true;
        }
        ASSERT(name != "");
        ce = new CEnum();
        if (isTypedefName)
            ce->typedefName = name;
        else
            ce->enumName = name;
        m_enums.push_back(ce);
        return ce;
    }

    CRecord *getCRecord(const std::string& name) {
        ASSERT(name != "");
        CRecord *cr;
        for (unsigned k = 0; k < m_records.size(); k++) {
            cr = m_records[k];
            if (cr->recName == name)
                return cr;
        }
        cr = new CRecord();
        cr->recName = name;
        m_records.push_back(cr);
        return cr;
    }

    CRecord *findCRecordByTypedefName(const std::string& name) {
        ASSERT(name != "");
        for (unsigned k = 0; k < m_records.size(); k++) {
            CRecord *cr = m_records[k];
            if (cr->typedefName == name)
                return cr;
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
"static void do_%s(v8_state vm, int argc, v8_val argv) {\n",
            cname);

        // the struct entry for the table of functions
        char buf[256];
        size_t n = snprintf(buf, 256, "{ %d, do_%s, \"%s\", V8_DLFUNC },\n",
                            numParams, cname, cname);
        ASSERT(n < 256);
        m_fns.append(buf);
    }

    void MakeWrapParam(JsType jsType, int paramNum) {
        switch (jsType) {
        case JsType::Int:
            fprintf(stdout, "int p%d = v8dl->to_int(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::Uint:
            fprintf(stdout, "unsigned int p%d = v8dl->to_uint(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::Long:
            fprintf(stdout, "int64_t p%d = v8dl->to_long(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::Ulong:
            fprintf(stdout, "uint64_t p%d = v8dl->to_uint(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::Double:
            fprintf(stdout, "double p%d = v8dl->to_double(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::String:
            fprintf(stdout, "char *p%d = v8dl->to_string(vm, %d, argv);\n",
                paramNum, paramNum+1);
            break;
        case JsType::VoidPtr:
        case JsType::Ptr:
            fprintf(stdout, "void *p%d = v8dl->to_pointer(vm, %d, argv);\n",
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
        fprintf(stdout, "if (v8dl->errstr(vm)) return;\n");
        switch (jsType) {
        case JsType::Void:
            fprintf(stdout, "%s(", fname);
            ends = "";
            break;
        case JsType::Int:
            fprintf(stdout, "int r = (int) %s(", fname);
            ends = "v8dl->from_int(vm, r, argv);\n";
            break;
        case JsType::Uint:
            fprintf(stdout, "unsigned int r = (unsigned int) %s(", fname);
            ends = "v8dl->from_uint(vm, r, argv);\n";
            break;
        case JsType::Long:
            fprintf(stdout, "int64_t r = (int64_t) %s(", fname);
            ends = "v8dl->from_long(vm, r, argv);\n";
            break;
        case JsType::Ulong:
            fprintf(stdout, "uint64_t r = (uint64_t) %s(", fname);
            ends = "v8dl->from_ulong(vm, r, argv);\n";
            break;
        case JsType::Double:
            fprintf(stdout, "double r = (double) %s(", fname);
            ends = "v8dl->from_double(vm, r, argv);\n";
            break;
        case JsType::String:
        case JsType::VoidPtr:
        case JsType::Ptr:
            fprintf(stdout, "void *r = (void *) %s(", fname);
            ends = "v8dl->from_pointer(vm, r, argv);\n";
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

    std::string generateAnonName(const NamedDecl* d) {
        assert(d->getDeclName().isEmpty());
        FullSourceLoc loc(d->getLocStart(), m_sm);
        if (loc.isValid()) {
            const std::string &fileName = m_sm.getFilename(loc);
            return "@" + fileName + ":"
                + std::to_string(loc.getSpellingLineNumber());
        }
        return "";
    }

    CRecord *getEmbeddedRecord(FieldDecl *fieldDecl) {
        //  fieldDecl->getDeclName().dump();
        std::string rName;
        bool isAnonymous = false;
        if (fieldDecl->isAnonymousStructOrUnion()) {
            // Anonymous union without a member name.
            ASSERT(fieldDecl->getDeclName().isEmpty());
            rName = generateAnonName(fieldDecl);
            isAnonymous = true;
        } else {
            const Type* fType = fieldDecl->getType().getTypePtr();
            assert(fType);
            if (fType->getTypeClass() == Type::Elaborated) {
                const ElaboratedType *eType = fType->getAs<clang::ElaboratedType>();
                const Type* namedType = eType->getNamedType().getTypePtr();
                ASSERT(namedType);
                if (namedType->getTypeClass() == Type::Record) {
                    RecordDecl *rD = namedType->getAs<clang::RecordType>()->getDecl();
                    ASSERT(rD);
                    rD = rD->getDefinition();
                    ASSERT(rD);
                    rName = rD->getNameAsString();
                    if (rName == "") {
                        // Anonymous record with a member name.
                        rName = generateAnonName(rD);
                        isAnonymous = true;
                    }
                }
            }
        }
        if (rName == "")
            return nullptr;
        CRecord *cr = getCRecord(rName);
        cr->isanon = isAnonymous;
        return cr;
    }

    std::string getFieldOffsets(RecordDecl *recDecl, const ASTRecordLayout &recLayout) {
        int fieldNo = 0;
        std::string s("{ ");
        for (RecordDecl::field_iterator field = recDecl->field_begin(),
                end = recDecl->field_end(); field != end; ++field, ++fieldNo) {

            // field->getType()
            // field->isAnonymousStructOrUnion()
            // field->isBitField()
            // field->isUnnamedBitField()
            // field->getFieldIndex() == fieldNo

            CRecord *cr = getEmbeddedRecord(*field);
            // if (! cr) ParseType(field->getType());
            cr = nullptr;   // Avoid warning

            uint64_t offsetInBits = recLayout.getFieldOffset(fieldNo); // in bits
            int offset = recDecl->getASTContext().toCharUnitsFromBits(
                                                offsetInBits).getQuantity();
            if (fieldNo > 0)
                s += ", ";

            std::string fieldName = field->getNameAsString();
#if 0
            if (fieldName == "") {
                // Embedded anonymous union without a field name.
                // struct { ..; union { int i; double d; }; .. }
                if (cr) {
                    // Using the generarted union name as the identifier
                    fieldName = cr->recName;
                }
            }
#endif
            if (fieldName == "") {
                // Embedded anonymous union without a field name.
                // struct { ..; union { int i; double d; }; .. }
                if (const RecordType *rT = field->getType()->getAs<RecordType>()) {
                    const RecordDecl *rD = rT->getDecl();
                    if (rD->isUnion()) {
                        if (const FieldDecl *fD = rD->findFirstNamedDataMember()) {
                            fieldName = fD->getNameAsString();
                        }
                    }
                }
            }

            ASSERT((fieldName != "") && "Field without a name");

            s += fieldName;
            s += " : ";
            s += std::to_string(offset);
        }
        return s + " }";
    }

private:
    SourceManager &m_sm;
    std::vector<CEnum *> &m_enums;
    std::vector<CRecord *> &m_records;  // struct and unions
    std::string &m_fns;
    std::string m_file;
};


class CConsumer : public ASTConsumer {
public:
    explicit CConsumer(CompilerInstance &ci, std::vector<CEnum *> &enums,
                std::vector<CRecord *> &recs,
                std::string &fns, std::string file)
        : m_ci(ci), m_visitor(CVisitor(ci.getSourceManager(),
                enums, recs, fns, file)) {}

    // Called by the parser for each top-level declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef dg) override {
        for (Decl *decl : dg) {
            if (m_ci.getDiagnostics().hasErrorOccurred()) {
                // TODO: Bail out ...
                // cout << "[[[[[[[[[[[[[[[[[ ERROR ]]]]]]]]]]]]]]]]]\n";
                ;
            }
            m_visitor.TraverseDecl(decl);
        }
        return true;
    }

private:
    CompilerInstance &m_ci;
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

        fprintf(stdout, "#include \"jsv8dlfn.h\"\n");
//        if (beginCode != "") {
//            fprintf(stdout, "%s\n", beginCode.c_str());
//        }
        if (hName.endswith(".h"))
            fprintf(stdout, "#include <%s>\n", hName.data());        
        fprintf(stdout, "\n");
        return true;
    }
 
    void EndSourceFileAction() override {
        fprintf(stdout, "\nstatic v8_ffn fntab_[] = {\n%s{0}\n};\n", m_fns.c_str());
        fprintf(stdout, "static const char source_str_[] = \"(function(){\\\n");
        fprintf(stdout, "var _tags = {}, _types = {}, _s;\\\n");
        for (unsigned k = 0; k < m_enums.size(); k++) {
            CEnum *ce = m_enums[k];
            if (ce->items.size() == 0)  // not defined
                continue;
            const std::string& eName = ce->typedefName != "" ? ce->typedefName
                            : ce->enumName;
            const char *name = eName.c_str();
            if (stripPrefix != "") {
                if (eName.find(stripPrefix) == 0)
                    name = name + stripPrefix.size();
            }
#if 0
            // enum type as tag object with enumerators as properties.
            // FIXME -- seperate _tags and _types entries for enum name and typedef
            fprintf(stdout, "_tags['%s'] = {\\\n", name);
            for (unsigned i = 0; i < ce->items.size(); i++) {
                fprintf(stdout, "%s : %d,\\\n",
                    ce->items[i]->name.c_str(), ce->items[i]->value);
            }

            fprintf(stdout, "};\\\n");
#else
            for (unsigned i = 0; i < ce->items.size(); i++) {
                fprintf(stdout, "this.%s = %d;\\\n",
                    ce->items[i]->name.c_str(), ce->items[i]->value);
            }
#endif
        }

        for (unsigned k = 0; k < m_records.size(); k++) {
            CRecord *r = m_records[k];
            if (r->offsets == "")
                continue;
            if (r->isanon && r->typedefName == "")
                continue;
            fprintf(stdout, "_s = %s; _s['#size'] = %d;\\\n",
                            r->offsets.c_str(), r->size);
            if (!r->isanon)
                fprintf(stdout, "_tags['%s'] = _s;\\\n", r->recName.c_str());
            if (r->typedefName != "")
                fprintf(stdout, "_types['%s'] = _s;\\\n", r->typedefName.c_str());
        }

        fprintf(stdout, "this['#tags'] = _tags;this['#types'] = _types;});\";\n\n");

        fprintf(stdout, "int JS_LOAD(v8_state vm, v8_handle hobj) {\n");
        fprintf(stdout, "int rc = v8->callstr(vm, source_str_, hobj, (v8_args){0});\n");
        fprintf(stdout, "if (!rc) return -1;\n");
        fprintf(stdout, "JS_EXPORT(fntab_);\n}\n");
    }

    // The StringRef parameter is the current input filename.
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci,
                                llvm::StringRef file) override {
        return llvm::make_unique<CConsumer>(
                        ci,  m_enums, m_records, m_fns, file.str());
    }

    ~CFrontendAction() {
        for (unsigned k = 0; k < m_enums.size(); k++) {
            delete m_enums[k];
        }
        for (unsigned k = 0; k < m_records.size(); k++) {
            delete m_records[k];
        }
    }

private:
    std::vector<CEnum *> m_enums;
    std::vector<CRecord *> m_records;
    std::string m_fns;
};


static std::set<std::string> split(const char *str, char c) {
    std::set<std::string> s;
    do {
        const char *begin = str;
        while (*str && *str != c)
            str++;
        s.insert(std::string(begin, str));
    } while (*str++);
    return s;
}

// N.B.: run with a trailing -- (./gencode filename --) to avoid seeing the
// garbage spewed by clang.

int main(int argc, const char **argv) {
    CommonOptionsParser op(argc, argv, toolCategory);
    ClangTool tool(op.getCompilations(), op.getSourcePathList());
    if (excludeFuncs != "")
        skipFuncs = split(excludeFuncs.c_str(), ':');
    return tool.run(newFrontendActionFactory<CFrontendAction>().get());
}

