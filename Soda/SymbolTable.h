#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace Soda
{

	struct AstDecl;

	enum SymbolKind
	{
		SK_BUILTIN,
		SK_TYPEDEF,
		SK_USING,
		SK_FUNCTION,
		SK_VARIABLE,
		SK_NAMESPACE,
		SK_STRUCT,
		SK_ENUMERATOR,
		SK_ENUM,
	};

	struct Symbol
	{
		SymbolKind kind;
		AstDecl *decl;
		Symbol(SymbolKind kind, AstDecl *decl = nullptr) 
			: kind(kind), decl(decl) {}
	};

	typedef std::unique_ptr<Symbol> SymbolPtr;

	class SymbolTable
	{
	public:
		SymbolTable *parent;
		SymbolTable(SymbolTable *parent = nullptr);
		bool define(SymbolKind kind, AstDecl *decl);
		Symbol *lookup(const std::string &name, bool rec = true);
		bool isDefined(const std::string &name, bool rec = true)
		{
			return (lookup(name, rec) != nullptr);
		}

	private:
		std::unordered_map<std::string, SymbolPtr> table;
	};

} // namespace Soda