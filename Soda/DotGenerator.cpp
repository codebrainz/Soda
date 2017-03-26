#include "DotGenerator.h"
#include "Visitor.h"
#include <unordered_map>

namespace SODA
{

	struct DOT_ID_TABLE
	{
		size_t idCounter;
		std::unordered_map<AST_NODE*, size_t> table;
		DOT_ID_TABLE() : idCounter(0) {}
		size_t nodeId(AST_NODE *n)
		{
			auto found = table.find(n);
			if (found != table.end())
				return found->second;
			auto id = idCounter++;
			table.emplace(n, id);
			return id;
		}
	};

	struct DOT_NODE_GENERATOR : public AST_VISITOR
	{
		std::ostream &os;
		DOT_ID_TABLE &idTable;
		DOT_NODE_GENERATOR(std::ostream &os, DOT_ID_TABLE &idTable) : os(os), idTable(idTable) {}
	};

	struct DOT_EDGE_GENERATOR : public AST_VISITOR
	{
		std::ostream &os;
		DOT_ID_TABLE &idTable;
		DOT_EDGE_GENERATOR(std::ostream &os, DOT_ID_TABLE &idTable) : os(os), idTable(idTable) {}
	};

	void generateDot(std::unique_ptr<AST_MODULE> &m, std::ostream &os)
	{
		DOT_ID_TABLE idTable;
		DOT_NODE_GENERATOR nodeGenerator(os, idTable);
		DOT_EDGE_GENERATOR edgeGenerator(os, idTable);
		os << "graph AstGraph {\n";
		m->accept(nodeGenerator);
		m->accept(edgeGenerator);
		os << "}\n";
	}

} // namespace SODA