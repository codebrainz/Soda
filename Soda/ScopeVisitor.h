#pragma once

#include "Ast.h"
#include "Compiler.h"
#include "SymbolTable.h"
#include "Visitor.h"
#include <cassert>
#include <iostream>
#include <stack>

namespace Soda
{

    class ScopeVisitor : public AstDefaultVisitor
    {
    public:
        Compiler &compiler;
        std::stack< SymbolTable * > scopeStack;
        unsigned int errorCount;

        ScopeVisitor(Compiler &compiler)
            : compiler(compiler)
            , errorCount(0)
        {
            scopeStack.push(&compiler.getGlobalScope());
        }

        SymbolTable *currentScope()
        {
            assert(!scopeStack.empty());
            return scopeStack.top();
        }

        template < class NodeT >
        void openScope(NodeT &n)
        {
            n.ownerScope = currentScope();
            n.scope.parent = currentScope();
            scopeStack.push(&n.scope);
        }

        template < class NodeT >
        void closeScope(NodeT &n)
        {
            assert(!scopeStack.empty());
            assert(currentScope() == &n.scope);
            scopeStack.pop();
        }

        void define(SymbolKind kind, AstDecl &decl)
        {
            assert(currentScope());
            if (!currentScope()->isDefined(decl.name, false)) {
                currentScope()->define(kind, &decl);
            } else if (kind == SK_FUNCTION || kind == SK_CONSTRUCTOR) {
                auto sym = currentScope()->lookup(decl.name, false);
                assert(sym);
                static_cast< OverloadedSymbol * >(sym)->addOverload(&decl);
            } else {
                compiler.error(decl, "multiple definitions of '%'", decl.name);
                errorCount++;
            }
        }

        auto splitName(const std::string &name)
        {
            std::vector< std::string > parts;
            size_t index = 0;
            parts.push_back("");
            for (auto &ch : name) {
                if (ch != '.')
                    parts[index] += ch;
                else {
                    index++;
                    parts.push_back("");
                }
            }
            return parts;
        }

        Symbol *lookup(AstNode &n, const std::string &name, bool rec = true)
        {
            assert(currentScope());
            auto nameParts = splitName(name);
            if (nameParts.size() == 1) {
                if (auto sym = currentScope()->lookup(nameParts[0], rec))
                    return sym;
                else {
                    compiler.error(n, "undefined symbol '%'", nameParts[0]);
                    errorCount++;
                    return nullptr;
                }
            } else if (nameParts.size() == 0) {
                compiler.error(n, "cannot lookup empty symbol");
                errorCount++;
                return nullptr;
            } else /* if (nameParts.size() > 1) */ {
                if (auto sym = currentScope()->lookup(nameParts[0])) {
                    for (size_t i = 1; sym != nullptr && i < nameParts.size();
                         i++) {
                        switch (sym->kind) {
                        case SK_NAMESPACE:
                            sym = static_cast< AstNamespaceDecl * >(
                                      static_cast< BasicSymbol * >(sym)->decl)
                                      ->scope.lookup(nameParts[i]);
                            break;
                        case SK_STRUCT:
                            sym = static_cast< AstStructDecl * >(
                                      static_cast< BasicSymbol * >(sym)->decl)
                                      ->scope.lookup(nameParts[i]);
                            break;
                        case SK_ENUM:
                            sym = static_cast< AstEnumDecl * >(
                                      static_cast< BasicSymbol * >(sym)->decl)
                                      ->scope.lookup(nameParts[i]);
                            break;
                        default:
                            i = nameParts.size();
                            break;
                        }
                    }
                    assert(sym);
                    return sym;
                }
                return nullptr;
            }
        }
    };

} // namespace Soda
