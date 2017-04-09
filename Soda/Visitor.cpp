#include "Ast.h"
#include "Visitor.h"

namespace Soda
{

    void AstDefaultVisitor::visit(AstAmbiguityStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstNil &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstBool &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstInt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstFloat &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstChar &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstString &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstIdentifier &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstUnary &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstBinary &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstTypedef &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstTypeRef &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstCast &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstIfExpr &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstCallExpr &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstIndexExpr &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstMemberExpr &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstEmptyStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstCommentStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstExprStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstBlockStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstReturnStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstBreakStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstContinueStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstGotoStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstIfStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstCaseStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstSwitchStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstForStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstDoStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstWhileStmt &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstEmptyDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstCommentDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstLabelDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstUsingDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstNamespaceDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstVarDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstParamDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstFuncDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstDelegateDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstStructDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstEnumeratorDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstEnumDecl &n)
    {
        n.acceptChildren(*this);
    }
    void AstDefaultVisitor::visit(AstModule &n)
    {
        n.acceptChildren(*this);
    }

} // namespace Soda
