#include "Symbol.h"
#include "Ast.h"
#include <cassert>

namespace Soda
{

    std::string BasicSymbol::name() const
    {
        assert(decl);
        return decl->name;
    }

    std::string BasicSymbol::mangledName() const
    {
        assert(decl);
        return decl->mangledName;
    }

    std::string OverloadedSymbol::name() const
    {
        assert(!decls.empty());
        return decls[0]->name;
    }

    std::string OverloadedSymbol::mangledName() const
    {
        assert(!decls.empty());
        return decls[0]->mangledName;
    }

} // namespace Soda
