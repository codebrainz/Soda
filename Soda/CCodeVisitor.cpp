#include "CCodeVisitor.h"
#include "CCode.h"

namespace Soda
{

    void CCodeDefaultVisitor::visit(CNullLit &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CBoolLit &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CIntLit &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CFloatLit &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CCharLit &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CStringLit &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CIdentifier &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CUnaryExpr &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CBinaryExpr &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CTypeRef &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CCastExpr &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CIfExpr &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CCallExpr &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CIndexExpr &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CMemberExpr &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CExprStmt &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CBlockStmt &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CReturnStmt &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CGotoStmt &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CIfStmt &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CDoStmt &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CWhileStmt &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CTypedef &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CVarDecl &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CParamDecl &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CFuncDecl &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CEnumeratorDecl &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CEnumDecl &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CStructDecl &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CDefineMacro &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CUndefMacro &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CIfMacro &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CElifMacro &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CElseMacro &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CEndifMacro &n)
    {
        n.acceptChildren(*this);
    }
    void CCodeDefaultVisitor::visit(CFile &n)
    {
        n.acceptChildren(*this);
    }

} // namespace Soda
