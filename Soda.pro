TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle qt

SOURCES += \
    Soda/DotGenerator.cpp \
    Soda/Main.cpp \
    Soda/NameMangler.cpp \
    Soda/Options.cpp \
    Soda/Parser.cpp \
    Soda/ScopeBuilder.cpp \
    Soda/SourceFile.cpp \
    Soda/SymbolResolver.cpp \
    Soda/SymbolTable.cpp \
    Soda/Tokenizer.cpp \
    Soda/Visitor.cpp

HEADERS += \
    Soda/Ast.h \
    Soda/Compiler.h \
    Soda/DotGenerator.h \
    Soda/Logger.h \
    Soda/NameMangler.h \
    Soda/Options.h \
    Soda/Parser.h \
    Soda/ScopeBuilder.h \
    Soda/ScopeVisitor.h \
    Soda/Soda.h \
    Soda/SourceFile.h \
    Soda/SymbolResolver.h \
    Soda/SymbolTable.h \
    Soda/Tokenizer.h \
    Soda/Visitor.h
