#include "Ast.h"
#include "DotGenerator.h"
#include "Visitor.h"
#include <cassert>
#include <unordered_map>
#include <vector>

namespace Soda
{

    struct DotIdTable
    {
        size_t idCounter;
        std::unordered_map< AstNode *, size_t > table;
        DotIdTable()
            : idCounter(0)
            , table()
        {
        }
        size_t nodeId(AstNode &n)
        {
            auto found = table.find(&n);
            if (found != table.end())
                return found->second;
            auto id = idCounter++;
            table.emplace(&n, id);
            return id;
        }
    };

    struct DotNodeGenerator : public AstDefaultVisitor
    {
        std::ostream &os;
        DotIdTable &idTable;
        DotNodeGenerator(std::ostream &os, DotIdTable &idTable)
            : os(os)
            , idTable(idTable)
        {
        }

        void handleNode(AstNode &n)
        {
            auto id = idTable.nodeId(n);
            os << "\tnode_" << id << " [label=\"" << n.kindName() << " (" << id
               << ")\"];\n";
            n.acceptChildren(*this);
        }

        void handleTypeRefNode(AstTypeRef &n)
        {
            auto id = idTable.nodeId(n);
            os << "\tnode_" << id << " [label=\"" << n.kindName() << " (" << id
               << ")";
            if (n.typeFlags != TF_NONE)
                os << "\\n";
            if (n.typeFlags & TF_ARRAY)
                os << "a ";
            if (n.typeFlags & TF_CONST)
                os << "c ";
            if (n.typeFlags & TF_POINTER)
                os << "p";
            if (!n.name.empty())
                os << "\\n" << n.name;
            os << "\"];\n";
            n.acceptChildren(*this);
        }

        template < class NodeT >
        void handleNamedNode(NodeT &n)
        {
            auto id = idTable.nodeId(n);
            os << "\tnode_" << id << " [label=\"" << n.kindName() << " (" << id
               << ")\\n"
               << n.name << "\"];\n";
            n.acceptChildren(*this);
        }

        void handleIdentNode(AstIdentifier &n)
        {
            auto id = idTable.nodeId(n);
            os << "\tnode_" << id << " [label=\"" << n.kindName() << " (" << id
               << ")\\n"
               << n.name << "\"];\n";
        }

        virtual void visit(AstAmbiguityStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstNil &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstBool &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstInt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstFloat &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstChar &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstString &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstIdentifier &n) override final
        {
            handleIdentNode(n);
        }
        virtual void visit(AstUnary &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstBinary &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstTypeRef &n) override final
        {
            handleTypeRefNode(n);
        }
        virtual void visit(AstCast &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstIfExpr &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstCallExpr &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstIndexExpr &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstMemberExpr &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstEmptyStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstCommentStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstExprStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstBlockStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstReturnStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstBreakStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstContinueStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstGotoStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstIfStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstWhileStmt &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstEmptyDecl &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstCommentDecl &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstUsingDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstTypedef &n) override final
        {
            handleNode(n);
        }
        virtual void visit(AstNamespaceDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstVarDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstParamDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstFuncDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstDelegateDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstStructDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstEnumeratorDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstEnumDecl &n) override final
        {
            handleNamedNode(n);
        }
        virtual void visit(AstModule &n) override final
        {
            handleNode(n);
        }
    };

    struct DotEdgeGenerator : public AstDefaultVisitor
    {
        std::ostream &os;
        DotIdTable &idTable;
        std::vector< AstNode * > nodeStack;

        DotEdgeGenerator(std::ostream &os, DotIdTable &idTable)
            : os(os)
            , idTable(idTable)
        {
        }

        void makeEdge(AstNode &n)
        {
            if (!nodeStack.empty()) {
                auto &parentNode = *nodeStack.back();
                os << "\tnode_" << idTable.nodeId(parentNode) << " -- "
                   << "node_" << idTable.nodeId(n) << ";\n";
            }
        }

        void handleEdge(AstNode &n)
        {
            makeEdge(n);
            nodeStack.emplace_back(&n);
            n.acceptChildren(*this);
            nodeStack.pop_back();
        }

        virtual void visit(AstAmbiguityStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstNil &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstBool &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstInt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstFloat &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstChar &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstString &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstIdentifier &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstUnary &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstBinary &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstTypeRef &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstCast &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstIfExpr &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstCallExpr &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstIndexExpr &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstMemberExpr &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstEmptyStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstCommentStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstExprStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstBlockStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstReturnStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstBreakStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstContinueStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstGotoStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstIfStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstWhileStmt &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstEmptyDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstCommentDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstUsingDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstTypedef &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstNamespaceDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstVarDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstParamDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstFuncDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstDelegateDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstStructDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstEnumeratorDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstEnumDecl &n) override final
        {
            handleEdge(n);
        }
        virtual void visit(AstModule &n) override final
        {
            handleEdge(n);
        }
    };

    void generateDot(AstNode &node, std::ostream &os)
    {
        DotIdTable idTable;
        DotNodeGenerator nodeGenerator(os, idTable);
        DotEdgeGenerator edgeGenerator(os, idTable);
        os << "graph AstGraph {\n";
        node.accept(nodeGenerator);
        node.accept(edgeGenerator);
        os << "}\n";
    }

} // namespace Soda
