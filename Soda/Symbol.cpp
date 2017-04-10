#include "Symbol.h"
#include "Ast.h"
#include <cassert>

namespace Soda
{

    Symbol::Symbol(AstDecl *decl)
    {
        assert(decl);
        overloads.emplace_back(decl);
    }

    bool Symbol::isOverloadable() const
    {
        return (primaryDecl()->kind == NK_FUNC_DECL
            || primaryDecl()->kind == NK_CONSTRUCTOR_DECL);
    }

    bool Symbol::isOverloaded() const
    {
        if (isOverloadable())
            return (overloads.size() > 1);
        return false;
    }

    bool Symbol::isTypeSymbol() const
    {
        return primaryDecl()->isTypeDecl();
    }

    const std::string &Symbol::name() const
    {
        return primaryDecl()->name;
    }

    bool Symbol::addOverload(AstDecl *decl)
    {
        assert(decl);
        if (!isOverloadable())
            return false;
        overloads.emplace_back(decl);
        return true;
    }

    AstDecl *Symbol::primaryDecl() const
    {
        assert(!overloads.empty());
        return overloads.at(0);
    }

} // namespace Soda
