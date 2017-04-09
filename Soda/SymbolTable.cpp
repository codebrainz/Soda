#include "Ast.h"
#include "SymbolTable.h"
#include <cassert>

namespace Soda
{

    SymbolTable::SymbolTable(SymbolTable *parent)
        : parent(parent)
    {
    }

    bool SymbolTable::define(SymbolKind kind, AstDecl *decl)
    {
        assert(decl);
        if (isDefined(decl->name, false))
            return false;
        if (kind == SK_FUNCTION)
            table.emplace(
                decl->name, std::make_unique< OverloadedSymbol >(kind, decl));
        else
            table.emplace(
                decl->name, std::make_unique< BasicSymbol >(kind, decl));
        return true;
    }

    Symbol *SymbolTable::lookup(const std::string &name, bool rec)
    {
        auto found = table.find(name);
        if (found != table.end())
            return found->second.get();
        else if (rec && parent)
            return parent->lookup(name, rec);
        return nullptr;
    }

} // namespace Soda
