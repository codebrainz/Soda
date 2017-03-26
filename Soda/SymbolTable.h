#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace SODA
{

	struct AST_DECL;

	enum SYMBOL_KIND
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

	struct SYMBOL
	{
		SYMBOL_KIND Kind;
		AST_DECL *Decl;
		SYMBOL(SYMBOL_KIND kind, AST_DECL *decl = nullptr) 
			: Kind(kind), Decl(decl) {}
	};

	typedef std::unique_ptr<SYMBOL> SYMBOL_PTR;

	class SYMBOL_TABLE
	{
	public:
		SYMBOL_TABLE *Parent;
		SYMBOL_TABLE(SYMBOL_TABLE *parent = nullptr);
		bool Define(SYMBOL_KIND kind, AST_DECL *decl);
		SYMBOL *Lookup(const std::string &name, bool rec = true);
		bool IsDefined(const std::string &name, bool rec = true)
		{
			return (Lookup(name, rec) != nullptr);
		}

	private:
		std::unordered_map<std::string, SYMBOL_PTR> Table;
	};

} // namespace SODA