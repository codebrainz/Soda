bin_PROGRAMS = sodac

sodac_CXXFLAGS = $(LLVM_CFLAGS)
sodac_LDADD = $(LLVM_LDFLAGS) $(LLVM_LIBS) $(LLVM_SYSTEM_LIBS)

sodac_SOURCES = \
	Ast.h \
	CodeGen.cpp CodeGen.h \
	Compiler.cpp Compiler.h \
	DotGenerator.cpp DotGenerator.h \
	LLVM.h \
	Logger.h \
	Main.cpp \
	Operators.cpp Operators.h \
	Options.cpp Options.h \
	Parser.cpp Parser.h \
	Platform.h \
	ScopeBuilder.cpp ScopeBuilder.h \
	ScopeVisitor.h \
	Sema.cpp Sema.h \
	Soda.h \
	SourceFile.cpp SourceFile.h \
	Symbol.cpp Symbol.h \
	SymbolResolver.cpp SymbolResolver.h \
	SymbolTable.cpp SymbolTable.h \
	Tokenizer.cpp Tokenizer.h \
	Visitor.cpp Visitor.h
