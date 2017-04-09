#include "Ast.h"
#include "Compiler.h"
#include "ScopeVisitor.h"
#include "SymbolResolver.h"
#include "SymbolTable.h"
#include <cassert>
#include <stack>

#include <iostream>

namespace Soda
{

    struct SymbolResolver : public ScopeVisitor
    {

        SymbolResolver(Compiler &compiler)
            : ScopeVisitor(compiler)
        {
        }

        virtual void visit(AstIdentifier &n) override final
        {
            n.refSymbol = lookup(n, n.name, true);
            // TODO: lookup name/type members like Foo.Bar.x
        }

        virtual void visit(AstBlockStmt &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }

        virtual void visit(AstGotoStmt &n) override final
        {
            n.refSymbol = lookup(n, n.label, false);
        }

        virtual void visit(AstSwitchStmt &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }

        virtual void visit(AstForStmt &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }

        virtual void visit(AstUsingDecl &) override final
        {
            // todo: import symbols from referred namespace into current scope
        }

        virtual void visit(AstNamespaceDecl &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }

        virtual void visit(AstFuncDecl &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }

        virtual void visit(AstDelegateDecl &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }

        virtual void visit(AstStructDecl &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }

        virtual void visit(AstEnumDecl &n) override final
        {
            openScope(n);
            n.acceptChildren(*this);
            closeScope(n);
        }
    };

    unsigned int resolveSymbols(Compiler &compiler, AstNode &node)
    {
        SymbolResolver resolver(compiler);
        node.accept(resolver);
        return resolver.errorCount;
    }

} // namespace Soda
