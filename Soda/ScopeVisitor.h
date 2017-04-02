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
		std::stack<SymbolTable*> scopeStack;
		unsigned int errorCount;

		ScopeVisitor(Compiler &compiler)
			: compiler(compiler), errorCount(0)
		{
			scopeStack.push(&compiler.getGlobalScope());
		}

		SymbolTable *currentScope()
		{
			assert(!scopeStack.empty());
			return scopeStack.top();
		}

		template< class NodeT >
		void openScope(NodeT &n)
		{
			n.ownerScope = currentScope();
			n.scope.parent = currentScope();
			scopeStack.push(&n.scope);
		}

		template< class NodeT >
		void closeScope(NodeT &n)
		{
			assert(!scopeStack.empty());
			assert(currentScope() == &n.scope);
			scopeStack.pop();
		}

		void define(SymbolKind kind, AstDecl &decl)
		{
			assert(currentScope());
			if (!currentScope()->isDefined(decl.name, false))
				currentScope()->define(kind, &decl);
			else
			{
				compiler.error(decl, "multiple definitions of '%'", decl.name);
				errorCount++;
			}
		}

		Symbol *lookup(AstNode &n, const std::string &name, bool rec = true)
		{
			assert(currentScope());
			if (auto sym = currentScope()->lookup(name, rec))
				return sym;
			else
			{
				compiler.error(n, "undefined symbol '%'", name);
				errorCount++;
				return nullptr;
			}
		}
	};

} // namespace Soda