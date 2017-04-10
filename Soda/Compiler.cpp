#include "Compiler.h"

namespace Soda
{

    void Compiler::addFile(const std::string &fn)
    {
        sourceFiles.push_back(std::make_unique< SourceFile >(*this, fn));
    }

    bool Compiler::parse()
    {
        for (auto &sourceFile : sourceFiles)
            sourceFile->tokenize(tokens);
        modules.emplace_back(parseTokens(*this, tokens));
        return true;
    }

    unsigned int Compiler::analyze()
    {
        unsigned int failures = 0;
        for (auto &mod : modules)
            failures += buildScopes(*this, *mod);
        for (auto &mod : modules)
            failures += resolveSymbols(*this, *mod);
        for (auto &mod : modules)
            analyzeSemantics(*this, *mod);
        return failures;
    }

} // namespace Soda
