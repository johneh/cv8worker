CXX := clang++-3.8
LD := ${CXX}
LLVM_CONFIG := llvm-config-3.8
CXXFLAGS := `${LLVM_CONFIG} --cxxflags`
LLVM_LDFLAGS := `${LLVM_CONFIG} --ldflags`
LLVM_LIBS := `${LLVM_CONFIG} --libs`
LLVM_LIBS += `${LLVM_CONFIG} --system-libs`

CXXFLAGS += -fno-rtti

# List of Clang libraries to link.
CLANG_LIBS := \
	-Wl,--start-group \
	-lclangAST \
	-lclangASTMatchers \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangSerialization \
	-lclangToolingCore \
	-lclangTooling \
	-lclangFormat \
	-Wl,--end-group

#	-lclangIndex \

all: gencode

gencode : gencode.cc
	$(CXX) $(CXXFLAGS) $^ $(CLANG_LIBS) $(LLVM_LDFLAGS) $(LLVM_LIBS) -o $@

clean :
	rm -f gencode

