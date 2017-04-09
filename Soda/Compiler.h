#pragma once

#include "Ast.h"
#include "Logger.h"
#include "NameMangler.h"
#include "Parser.h"
#include "ScopeBuilder.h"
#include "Sema.h"
#include "SourceFile.h"
#include "SymbolResolver.h"
#include "SymbolTable.h"
#include "Tokenizer.h"
#include <iostream>
#include <vector>

namespace Soda
{

    class Compiler
    {
    public:
        void addFile(const std::string &fn);
        bool parse();
        unsigned int analyze();

        const TokenList &getTokens() const
        {
            return tokens;
        }

        TokenList &getTokens()
        {
            return tokens;
        }

        SymbolTable &getGlobalScope()
        {
            return globalScope;
        }

        const SymbolTable &getGlobalScope() const
        {
            return globalScope;
        }

        AstModuleList &GetModules()
        {
            return modules;
        }

        const AstModuleList &GetModules() const
        {
            return modules;
        }

        template < class... Args >
        void error(Args &&... args)
        {
            logger.error(std::forward< Args >(args)...);
        }

        template < class... Args >
        void warning(Args &&... args)
        {
            logger.warning(std::forward< Args >(args)...);
        }

        template < class... Args >
        void note(Args &&... args)
        {
            logger.note(std::forward< Args >(args)...);
        }

        template < class... Args >
        void debug(Args &&... args)
        {
            logger.debug(std::forward< Args >(args)...);
        }

        bool reportDiagnostics(
            std::ostream &os = std::cerr, size_t limit = size_t(-1))
        {
            return logger.outputDiagnostics(os, limit);
        }

    private:
        SymbolTable globalScope;
        AstModuleList modules;
        TokenList tokens;
        SourceFileList sourceFiles;
        Logger logger;
    };

} // namespace Soda
