#include "Ast.h"
#include "SymbolTable.h"
#include <cassert>

namespace Soda
{

    SymbolTable::SymbolTable(SymbolTable *parent)
        : parent(parent)
    {
    }

    Symbol *SymbolTable::define(AstDecl *decl)
    {
        assert(decl);
        auto sym = lookup(decl->name, false);
        if (sym) {
            if (sym->isOverloadable()) {
                sym->addOverload(decl);
                return sym;
            }
        } else {
            table.emplace(decl->name, std::make_unique< Symbol >(decl));
            return lookup(decl->name, false);
        }
        return nullptr;
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
