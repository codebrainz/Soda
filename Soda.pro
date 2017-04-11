TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle qt

SOURCES += \
    Soda/DotGenerator.cpp \
    Soda/Main.cpp \
    Soda/Options.cpp \
    Soda/Parser.cpp \
    Soda/ScopeBuilder.cpp \
    Soda/SourceFile.cpp \
    Soda/SymbolResolver.cpp \
    Soda/SymbolTable.cpp \
    Soda/Tokenizer.cpp \
    Soda/Visitor.cpp \
    Soda/Operators.cpp \
    Soda/Sema.cpp \
    Soda/Symbol.cpp \
    Soda/Compiler.cpp \
    Soda/CodeGen.cpp

HEADERS += \
    Soda/Ast.h \
    Soda/Compiler.h \
    Soda/DotGenerator.h \
    Soda/Logger.h \
    Soda/Options.h \
    Soda/Parser.h \
    Soda/ScopeBuilder.h \
    Soda/ScopeVisitor.h \
    Soda/Soda.h \
    Soda/SourceFile.h \
    Soda/SymbolResolver.h \
    Soda/SymbolTable.h \
    Soda/Tokenizer.h \
    Soda/Visitor.h \
    Soda/Operators.h \
    Soda/Sema.h \
    Soda/Symbol.h \
    Soda/LLVM.h \
    Soda/CodeGen.h \
    Soda/Platform.h

QMAKE_CXXFLAGS += `llvm-config-3.8 --cflags`
QMAKE_LFLAGS += `llvm-config-3.8 --ldflags --libs`
