#include "CCodeTreeGen.h"
#include "Compiler.h"
#include "Visitor.h"
#include <cassert>
#include <stack>
#include <type_traits>

namespace Soda
{

    struct CCodeTreeGenBase : public AstDefaultVisitor
    {
        Compiler &compiler;
        CFile &file;
        std::stack< CNodeList * > contextStack;

        CCodeTreeGenBase(Compiler &compiler, CFile &file)
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

        template < class T >
        T *add(CNodePtrType< T > node)
        {
            assert(!contextStack.empty());
            auto ptr = node.get();
            contextStack.top()->push_back(std::move(node));
            return ptr;
        }

        template < class T, class... Args >
        T *emplace(Args &&... args)
        {
            return add(std::make_unique< T >(std::forward< Args >(args)...));
        }
    };

    struct CCodeTreeGenHeader : public CCodeTreeGenBase
    {
        CCodeTreeGenHeader(Compiler &compiler, CFile &file)
            : CCodeTreeGenBase(compiler, file)
        {
        }

        virtual void visit(AstNil &n) override final
        {
            emplace< CNullLit >(n.start, n.end);
        }

        virtual void visit(AstBool &n) override final
        {
            emplace< CBoolLit >(n.value, n.start, n.end);
        }

        virtual void visit(AstInt &n) override final
        {
            emplace< CIntLit >(n.value, n.start, n.end);
        }

        virtual void visit(AstFloat &n) override final
        {
            emplace< CFloatLit >(n.value, n.start, n.end);
        }

        virtual void visit(AstChar &n) override final
        {
            emplace< CCharLit >(n.value, n.start, n.end);
        }

        virtual void visit(AstString &n) override final
        {
            emplace< CStringLit >(n.value, n.start, n.end);
        }

        virtual void visit(AstIdentifier &n) override final
        {
            emplace< CIdentifier >(
                n.refSymbol->decl->mangledName, n.start, n.end);
        }

        virtual void visit(AstUnary &n) override final
        {
            auto exp = emplace< CUnaryExpr >(n.op, n.start, n.end);
            openContext(exp->children);
            n.acceptChildren(*this);
            closeContext(exp->children);
        }

        virtual void visit(AstBinary &n) override final
        {
            auto exp = emplace< CBinaryExpr >(n.op, n.start, n.end);
            openContext(exp->children);
            n.acceptChildren(*this);
            closeContext(exp->children);
        }

        virtual void visit(AstTypeRef &n) override final
        {
            auto tref = emplace< CTypeRef >(n.mangledName, n.start, n.end);
            tref->isConst = n.typeFlags & TF_CONST;
            tref->isPointer = n.typeFlags & TF_POINTER;
            tref->isArray = n.typeFlags & TF_ARRAY;
            openContext(tref->children);
            n.acceptChildren(*this);
            closeContext(tref->children);
        }

        virtual void visit(AstCast &n) override final
        {
            auto cast = emplace< CCastExpr >(n.start, n.end);
            openContext(cast->children);
            n.acceptChildren(*this);
            closeContext(cast->children);
        }

        virtual void visit(AstIfExpr &n) override final
        {
            auto ifexp = emplace< CIfExpr >(n.start, n.end);
            openContext(ifexp->children);
            n.acceptChildren(*this);
            closeContext(ifexp->children);
        }

        virtual void visit(AstCallExpr &n) override final
        {
            auto call = emplace< CCallExpr >(n.start, n.end);
            openContext(call->children);
            n.acceptChildren(*this);
            closeContext(call->children);
        }

        virtual void visit(AstIndexExpr &n) override final
        {
            auto ind = emplace< CIndexExpr >(n.start, n.end);
            openContext(ind->children);
            n.acceptChildren(*this);
            closeContext(ind->children);
        }

        virtual void visit(AstMemberExpr &n) override final
        {
            auto mem = emplace< CMemberExpr >(n.start, n.end);
            openContext(mem->children);
            n.acceptChildren(*this);
            closeContext(mem->children);
        }

        virtual void visit(AstTypedef &n) override final
        {
            auto tdef = emplace< CTypedef >(n.mangledName, n.start, n.end);
            openContext(tdef->children);
            n.acceptChildren(*this);
            closeContext(tdef->children);
        }

        virtual void visit(AstVarDecl &n) override final
        {
            auto var = emplace< CVarDecl >(n.mangledName, n.start, n.end);
            openContext(var->children);
            n.typeRef->accept(*this);
            if (n.initExpr)
                n.initExpr->accept(*this);
            closeContext(var->children);
        }

        virtual void visit(AstParamDecl &n) override final
        {
            auto param = emplace< CParamDecl >(n.mangledName, n.start, n.end);
            openContext(param->children);
            n.acceptChildren(*this);
            closeContext(param->children);
            assert(!param->children.size());
        }

        virtual void visit(AstFuncDecl &n) override final
        {
            auto fun = emplace< CFuncDecl >(n.mangledName, n.start, n.end);
            openContext(fun->children);
            n.typeRef->accept(*this);
            for (auto &param : n.params)
                param->accept(*this);
            closeContext(fun->children);
        }

        virtual void visit(AstEnumeratorDecl &n) override final
        {
            auto etor
                = emplace< CEnumeratorDecl >(n.mangledName, n.start, n.end);
            openContext(etor->children);
            n.acceptChildren(*this);
            closeContext(etor->children);
        }

        virtual void visit(AstEnumDecl &n) override final
        {
            auto enu = emplace< CEnumDecl >(n.mangledName, n.start, n.end);
            openContext(enu->children);
            n.acceptChildren(*this);
            closeContext(enu->children);
        }

        // todo: handle base types
        virtual void visit(AstStructDecl &n) override final
        {
            auto str = emplace< CStructDecl >(n.mangledName, n.start, n.end);
            openContext(str->children);
            n.acceptChildren(*this);
            closeContext(str->children);
        }

        virtual void visit(AstModule &n) override final
        {
            emplace< CIfMacro >(
                "!defined(" + n.identifierName() + "_HEADER_INCLUDED__)",
                n.start, n.end);
            emplace< CDefineMacro >(
                n.identifierName() + "_HEADER_INCLUDED__", "", n.start, n.end);
            n.acceptChildren(*this);
            emplace< CEndifMacro >(n.start, n.end);
        }
    };

    struct CCodeTreeGenSource : public CCodeTreeGenBase
    {
        CCodeTreeGenSource(Compiler &compiler, CFile &file)
            : CCodeTreeGenBase(compiler, file)
        {
        }

        virtual void visit(AstModule &n) override final
        {
            emplace< CIfMacro >(
                "!defined(" + n.identifierName() + "_SOURCE_INCLUDED__)",
                n.start, n.end);
            emplace< CDefineMacro >(
                n.identifierName() + "_SOURCE_INCLUDED__", "", n.start, n.end);
            n.acceptChildren(*this);
            emplace< CEndifMacro >(n.start, n.end);
        }
    };

    CFilePtr generateCTree(
        Compiler &compiler, AstNode &node, CCodeTreeKind kind)
    {
        using UT = std::underlying_type< CCodeTreeKind >::type;
        auto file = std::make_unique< CFile >();
        if (static_cast< UT >(kind)
            & static_cast< UT >(CCodeTreeKind::HEADER)) {
            CCodeTreeGenHeader hdrGen(compiler, *file);
            node.accept(hdrGen);
        }
        if (static_cast< UT >(kind)
            & static_cast< UT >(CCodeTreeKind::SOURCE)) {
            CCodeTreeGenSource srcGen(compiler, *file);
            node.accept(srcGen);
        }
        return file;
    }

} // namespace Soda
