#include "DotGenerator.h"
#include "Visitor.h"
#include <unordered_map>

namespace Soda
{

	struct DotIdTable
	{
		size_t idCounter;
		std::unordered_map<AstNode*, size_t> table;
		DotIdTable() : idCounter(0) {}
		size_t nodeId(AstNode *n)
		{
			auto found = table.find(n);
			if (found != table.end())
				return found->second;
			auto id = idCounter++;
			table.emplace(n, id);
			return id;
		}
	};

	struct DotNodeGenerator : public AstDefaultVisitor
	{
		std::ostream &os;
		DotIdTable &idTable;
		DotNodeGenerator(std::ostream &os, DotIdTable &idTable) : os(os), idTable(idTable) {}
	};

	struct DotEdgeGenerator : public AstDefaultVisitor
	{
		std::ostream &os;
		DotIdTable &idTable;
		DotEdgeGenerator(std::ostream &os, DotIdTable &idTable) : os(os), idTable(idTable) {}
	};

	void generateDot(std::unique_ptr<AstModule> &m, std::ostream &os)
	{
		DotIdTable idTable;
		DotNodeGenerator nodeGenerator(os, idTable);
		DotEdgeGenerator edgeGenerator(os, idTable);
		os << "graph AstGraph {\n";
		m->accept(nodeGenerator);
		m->accept(edgeGenerator);
		os << "}\n";
	}

} // namespace Soda