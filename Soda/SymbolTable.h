#pragma once

#include "Symbol.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Soda
{

    struct AstDecl;

    class SymbolTable
    {
    public:
        typedef std::unordered_map< std::string, SymbolPtr > TableType;

        SymbolTable *parent;
        SymbolTable(SymbolTable *parent = nullptr);
        Symbol *define(AstDecl *decl);
        Symbol *lookup(const std::string &name, bool rec = true);
        bool isDefined(const std::string &name, bool rec = true)
        {
            return (lookup(name, rec) != nullptr);
        }
        TableType::iterator begin()
        {
            return table.begin();
        }
        TableType::iterator end()
        {
            return table.end();
        }

    private:
        TableType table;
    };

} // namespace Soda
