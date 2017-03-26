#include "SymbolTable.h"
#include "Ast.h"
#include <cassert>

namespace SODA
{

	SYMBOL_TABLE::SYMBOL_TABLE(SYMBOL_TABLE *parent)
		: Parent(parent)
	{
	}

	bool SYMBOL_TABLE::Define(SYMBOL_KIND kind, AST_DECL *decl)
	{
		assert(decl);
		if (IsDefined(decl->Name, false))
			return false;
		Table.emplace(decl->Name, std::make_unique<SYMBOL>(kind, decl));
		return true;
	}

	SYMBOL *SYMBOL_TABLE::Lookup(const std::string &name, bool rec)
	{
		auto found = Table.find(name);
		if (found != Table.end())
			return found->second.get();
		else if (rec && Parent)
			return Parent->Lookup(name, rec);
		return nullptr;
	}

} // namespace SODA