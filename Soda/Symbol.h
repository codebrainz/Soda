#pragma once

#include <memory>
#include <vector>

namespace Soda
{

    struct AstDecl;

    struct Symbol
    {
        std::vector< AstDecl * > overloads;
        Symbol(AstDecl *decl);
        bool isOverloadable() const;
        bool isOverloaded() const;
        bool isTypeSymbol() const;
        const std::string &name() const;
        bool addOverload(AstDecl *decl);
        AstDecl *primaryDecl() const;
    };

    typedef std::unique_ptr< Symbol > SymbolPtr;

} // namespace Soda
