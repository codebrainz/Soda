#pragma once

#include <memory>
#include <vector>

namespace Soda
{

    struct AstDecl;

    enum SymbolKind
    {
        SK_BUILTIN,
        SK_LABEL,
        SK_TYPEDEF,
        SK_USING,
        SK_FUNCTION,
        SK_DELEGATE,
        SK_VARIABLE,
        SK_NAMESPACE,
        SK_STRUCT,
        SK_ENUMERATOR,
        SK_ENUM,
    };

    struct Symbol
    {
        SymbolKind kind;
        Symbol(SymbolKind kind)
            : kind(kind)
        {
        }
        virtual ~Symbol()
        {
        }
        virtual bool isOverloadable() const
        {
            return false;
        }
        virtual std::string name() const = 0;
        virtual std::string mangledName() const = 0;
    };

    struct BasicSymbol final : public Symbol
    {
        AstDecl *decl;
        BasicSymbol(SymbolKind kind, AstDecl *decl)
            : Symbol(kind)
            , decl(decl)
        {
        }
        virtual std::string name() const override final;
        virtual std::string mangledName() const override final;
    };

    struct OverloadedSymbol final : public Symbol
    {
        std::vector< AstDecl * > decls;
        OverloadedSymbol(SymbolKind kind, AstDecl *firstOverload)
            : Symbol(kind)
        {
            addOverload(firstOverload);
        }
        void addOverload(AstDecl *decl)
        {
            decls.emplace_back(decl);
        }
        virtual bool isOverloadable() const override final
        {
            return true;
        }
        virtual std::string name() const override final;
        virtual std::string mangledName() const override final;
    };

    typedef std::unique_ptr< Symbol > SymbolPtr;

} // namespace Soda
