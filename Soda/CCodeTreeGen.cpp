#include "CCodeTreeGen.h"
#include "Compiler.h"
#include "Visitor.h"
#include <cassert>
#include <stack>
#include <type_traits>

namespace Soda
{

    struct CCodeTreeGenHeader : public AstDefaultVisitor
    {
        Compiler &compiler;
        CFile &file;
        std::stack< CNodeList * > contextStack;

        CCodeTreeGenHeader(Compiler &compiler, CFile &file)
            : compiler(compiler)
            , file(file)
        {
            openContext(file.children);
        }

        void openContext(CNodeList &nodeList)
        {
            contextStack.emplace(&nodeList);
        }

        void closeContext(CNodeList &nodeList)
        {
            assert(!contextStack.empty());
            assert(contextStack.top() == &nodeList);
            contextStack.pop();
        }

        CNode *add(CNodePtr node)
        {
            assert(!contextStack.empty());
            auto ptr = node.get();
            contextStack.top()->push_back(std::move(node));
            return ptr;
        }

        template < class T, class... Args >
        CNode *emplace(Args &&... args)
        {
            return add(std::make_unique< T >(std::forward< Args >(args)...));
        }

        // todo: handle base types
        virtual void visit(AstStructDecl &n) override final
        {
            auto str = emplace< CStructDecl >(n.mangledName, n.start, n.end);
            openContext(str->members);
            n.acceptChildren(*this);
            closeContext(str->members);
        }

        virtual void visit(AstEnumDecl &n) override final
        {
            auto enu = emplace< CEnumDecl >(n.mangledName, n.start, n.end);
            openContext(enu->enumerators);
            for (auto &etor : enu->enumerators) {
                // todo: compute and use init exprs
                enu->enumerators.push_back(std::make_unique< CEnumeratorDecl >(
                    etor->mangledName, nullptr, etor->start, etor->end));
            }
            closeContext(enu->enumerators);
        }

        virtual void visit(AstModule &n) override final
        {
            file.children.push_back(std::make_unique< CIfMacro >(
                "!defined(" + n.identifierName() + "_INCLUDED__)", n.start,
                n.end));
            file.children.push_back(std::make_unique< CDefineMacro >(
                n.identifierName() + "_INCLUDED__", "", n.start, n.end));
            n.acceptChildren(*this);
            file.children.push_back(
                std::make_unique< CEndifMacro >(n.start, n.end));
        }
    };

    struct CCodeTreeGenSource : public AstDefaultVisitor
    {
        Compiler &compiler;
        CFile &file;
        CCodeTreeGenSource(Compiler &compiler, CFile &file)
            : compiler(compiler)
            , file(file)
        {
        }
    };

    CFilePtr generateCTree(
        Compiler &compiler, AstNode &node, CCodeTreeKind kind)
    {
        using UT = std::underlying_type< CCodeTreeKind >::type;
        auto file = std::make_unique< CFile >();
        if (static_cast< UT >(kind)
            & static_cast< UT >(CCodeTreeKind::HEADER)) {
            CCodeTreeGenHeader hdrGen(compiler, file);
            astNode.accept(hdrGen);
        }
        if (static_cast< UT >(kind)
            & static_cast< UT >(CCodeTreeKind::SOURCE)) {
            CCodeTreeGenSource srcGen(compiler, file);
            astNode.accept(compiler, file);
        }
        return file;
    }

} // namespace Soda
