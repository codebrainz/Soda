#pragma once

#include "Operators.h"
#include "Tokenizer.h"
#include <memory>
#include <string>
#include <vector>

namespace Soda
{

    enum CNodeKind
    {
        CK_NULL_LIT,
        CK_BOOL_LIT,
        CK_INT_LIT,
        CK_FLOAT_LIT,
        CK_CHAR_LIT,
        CK_STRING_LIT,
        CK_IDENTIFIER,
        CK_UNARY_EXPR,
        CK_BINARY_EXPR,
        CK_CAST_EXPR,
        CK_CALL_EXPR,
        CK_INDEX_EXPR,
        CK_MEMBER_EXPR,
        CK_EXPR_STMT,
        CK_BLOCK_STMT,
        CK_RETURN_STMT,
        CK_GOTO_STMT,
        CK_IF_STMT,
        CK_DO_STMT,
        CK_WHILE_STMT,
        CK_VAR_DECL,
        CK_PARAM_DECL,
        CK_FUNC_DECL,
        CK_ENUMERATOR_DECL,
        CK_ENUM_DECL,
        CK_STRUCT_DECL,
        CK_DEFINE_MACRO,
        CK_UNDEF_MACRO,
        CK_IF_MACRO,
        CK_ELIF_MACRO,
        CK_ELSE_MACRO,
        CK_ENDIF_MACRO,
        CK_FILE,
    };

    template < class... Args >
    using CNodePtrType = std::unique_ptr< Args >;

    template < class... Args >
    using CNodeListType = std::vector< Args >;

    struct CNode
    {
        CNodeKind kind;
        Token *start;
        Token *end;

        CNode(CNodeKind kind, Token *start, Token *end)
            : kind(kind)
            , start(start)
            , end(end)
        {
        }

        virtual ~CNode()
        {
        }
    };

    struct CExpr : public CNode
    {
        CExpr(CNodeKind kind, Token *start, Token *end)
            : CNode(kind, start, end)
        {
        }
    };

    struct CStmt : public CNode
    {
        CStmt(CNodeKind kind, Token *start, Token *end)
            : CNode(kind, start, end)
        {
        }
    };

    struct CDecl : public CStmt
    {
        std::string name;
        CDecl(CNodeKind kind, std::string name, Token *start, Token *end)
            : CStmt(kind, start, end)
            , name(std::move(name))
        {
        }
    };

    struct CMacro : public CStmt
    {
        CMacro(CNodeKind kind, Token *start = nullptr, Token *end = nullptr)
            : CStmt(kind, start, end)
        {
        }
    };

    typedef CNodePtrType< CNode > CNodePtr;
    typedef CNodePtrType< CExpr > CExprPtr;
    typedef CNodePtrType< CStmt > CStmtPtr;
    typedef CNodePtrType< CDecl > CDeclPtr;

    typedef CNodeListType< CNodePtr > CNodeList;
    typedef CNodeListType< CExprPtr > CExprList;
    typedef CNodeListType< CStmtPtr > CStmtList;
    typedef CNodeListType< CDeclPtr > CDeclList;

    struct CNullLit : public CExpr
    {
    };

    struct CBoolLit : public CExpr
    {
        bool value;
        CBoolLit(bool value)
            : value(value)
        {
        }
    };

    struct CIntLit : public CExpr
    {
        unsigned long long value;
        CIntLit(unsigned long long value, Token *start = nullptr,
            Token *end = nullptr)
            : CExpr(CK_INT_LIT, start, end)
            , value(value)
        {
        }
    };

    struct CFloatLit : public CExpr
    {
        long double value;
        CFloatLit(long double value, Token *start, Token *end)
            : CExpr(CK_FLOAT_LIT, start, end)
            , value(value)
        {
        }
    };

    struct CCharLit : public CExpr
    {
        std::string value;
        CCharLit(
            std::string value, Token *start = nullptr, Token *end = nullptr)
            : CExpr(CK_CHAR_LIT, start, end)
            , value(std::move(value))
        {
        }
    };

    struct CStringLit : public CExpr
    {
        std::string value;
        CStringLit(
            std::string value, Token *start = nullptr, Token *end = nullptr)
            : CExpr(CK_STRING_LIT, start, end)
            , value(std::move(value))
        {
        }
    };

    struct CIdentifier : public CExpr
    {
        std::string name;
        CIdentifier(
            std::string name, Token *start = nullptr, Token *end = nullptr)
            : CExpr(CK_IDENTIFIER, start, end)
            , name(std::move(name))
        {
        }
    };

    struct CUnaryExpr : public CExpr
    {
        UnaryOperator op;
        CExprPtr operand;
        CUnaryExpr(UnaryOperator op, CExprPtr operand, Token *start = nullptr,
            Token *end = nullptr)
            : CExpr(CK_UNARY_EXPR, start, end)
            , op(op)
            , operand(std::move(operand))
        {
        }
    };

    struct CBinaryExpr : public CExpr
    {
        BinaryOperator op;
        CExprPtr left;
        CExprPtr right;
    };

    struct CTypeRef : public CNode
    {
        std::string name;
        CTypeRef typeRef;
        bool isPointer;
        bool isConst;
        bool isArray;
        CTypeRef(std::string name, CTypeRef typeRef, Token *start = nullptr,
            Token *end = nullptr)
            : CNode(CK_TYPEREF, start, end)
            , name(std::move(name))
            , typeRef(std::move(typeRef))
            , isPointer(false)
            , isConst(false)
            , isArray(false)
        {
        }
    };

    typedef CNodePtrType< CTypeRef > CTypeRefPtr;

    struct CCastExpr : public CExpr
    {
        CTypeRef *typeRef;
        CExprPtr expr;
        CCastExpr(CTypeRef *typeRef, CExprPtr expr)
            : CExpr(CK_CAST_EXPR, start, end)
            , typeRef(typeRef)
            , expr(std::move(expr))
        {
        }
    };

    struct CCallExpr : public CExpr
    {
        CExprPtr callee;
        CNodeList arguments;
        CCallExpr(CExprPtr callee, CExprList arguments, Token *start = nullptr,
            Token *end = nullptr)
            : CExpr(CK_CALL_EXPR, start, end)
            , callee(std::move(callee))
            , arguments(std::move(arguments))
        {
        }
    };

    struct CIndexExpr : public CExpr
    {
        CExprPtr object;
        CExprPtr index;
        CIndexExpr(CExprPtr object, CExprPtr index, Token *start = nullptr,
            Token *end = nullptr)
            : CExpr(CK_INDEX_EXPR, start, end)
            , object(std::move(object))
            , index(std::move(index))
        {
        }
    };

    struct CMemberExpr : public CExpr
    {
        CExprPtr object;
        std::string member;
        CMemberExpr(CExprPtr object, std::string member, Token *start = nullptr,
            Token *end = nullptr)
            : CExpr(CK_MEMBER_EXPR, start, end)
            , object(std::move(object))
            , member(std::move(member))
        {
        }
    };

    struct CExprStmt : public CStmt
    {
        CExprPtr expr;
        CExprStmt(CExprPtr expr, Token *start = nullptr, Token *end = nullptr)
            : CStmt(CK_EXPR_STMT, start, end)
            , expr(std::move(expr))
        {
        }
    };

    struct CBlockStmt : public CStmt
    {
        CNodeList stmts;
        CBlockStmt(Token *start = nullptr, Token *end = nullptr)
            : CStmt(CK_BLOCK_STMT, start, end)
        {
        }
        CBlockStmt(
            CStmtList stmts, Token *start = nullptr, Token *end = nullptr)
            : CBlockStmt(start, end)
            , stms(std::move(stmts))
        {
        }
    };

    struct CReturnStmt : public CStmt
    {
        CExprPtr expr;
        CReturnStmt(CExprPtr expr, Token *start = nullptr, Token *end = nullptr)
            : CStmt(CK_RETURN_STMT, start, end)
            , expr(std::move(expr))
        {
        }
    };

    struct CGotoStmt : public CStmt
    {
        std::string label;
        CGotoStmt(
            std::string label, Token *start = nullptr, Token *end = nullptr)
            : CStmt(CK_GOTO_STMT, start, end)
            , label(std::move(label))
        {
        }
    };

    struct CIfStmt : public CStmt
    {
        CExprPtr condExpr;
        CStmtPtr thenStmt;
        CStmtPtr elseStmt;
        CIfStmt(CExprPtr condExpr, CStmtPtr thenStmt, CStmtPtr elseStmt,
            Token *start = nullptr, Token *end = nullptr)
            : CStmt(CK_IF_STMT, start, end)
            , condExpr(std::move(condExpr))
            , thenStmt(std::move(thenStmt))
            , elseStmt(std::move(elseStmt))
        {
        }
    };

    struct CDoStmt : public CStmt
    {
        CStmtPtr stmt;
        CExprPtr expr;
        CDoStmt(CStmtPtr stmt, CExprPtr expr, Token *start = nullptr,
            Token *end = nullptr)
            : CStmt(CK_DO_STMT, start, end)
            , stmt(std::move(stmt))
            , expr(std::move(expr))
        {
        }
    };

    struct CWhileStmt : public CStmt
    {
        CExprPtr expr;
        CStmtPtr stmt;
        CWhileStmt(CExprPtr expr, CStmtPtr stmt, Token *start = nullptr,
            Token *end = nullptr)
            : CStmt(CK_WHILE_STMT, start, end)
            , expr(std::move(expr))
            , stmt(std::move(stmt))
        {
        }
    };

    struct CVarDecl : public CDecl
    {
        CTypeRefPtr typeRef;
        CExprPtr initExpr;
        CVarDecl(std::string name, CTypeRefPtr typeRef, CExprPtr initExpr,
            Token *start = nullptr, Token *end = nullptr)
            : CDecl(CK_VAR_DECL, std::move(name), start, end)
            , typeRef(std::move(typeRef))
            , initExpr(std::move(initExpr))
        {
        }
    };

    struct CParamDecl : public CDecl
    {
        CTypeRefPtr typeRef;
        CParamDecl(std::string name, CTypeRefPtr typeRef,
            Token *start = nullptr, Token *end = nullptr)
            : CDecl(CK_PARAM_DECL, std::move(name), start, end)
            , typeRef(std::move(typeRef))
        {
        }
    };

    struct CFuncDecl : public CDecl
    {
        CNodeList parameters;
        CNodeList statements;
        CFuncDecl(
            std::string name, Token *start = nullptr, Token *end = nullptr)
            : CDecl(CK_FUNC_DECL, std::move(name), start, end)
        {
        }
        CFuncDecl(std::string name, CDeclList parameters,
            Token *start = nullptr, Token *end = nullptr)
            : CFuncDecl(std::move(name), start, end)
            , parameters(std::move(parameters))
        {
        }
        CFuncDecl(std::string name, CStmtList statements,
            Token *start = nullptr, Token *end = nullptr)
            : CFuncDecl(std::move(name), start, end)
            , statements(std::move(statements))
        {
        }
        CFuncDecl(std::string name, CDeclList parameters, CStmtList statements,
            Token *start = nullptr, Token *end = nullptr)
            : CFuncDecl(std::move(name), start, end)
            , parameters(std::move(parameters))
            , statements(std::move(statements))
        {
        }
    };

    struct CEnumeratorDecl : public CDecl
    {
        CExprPtr initExpr;
        CEnumeratorDecl(std::string name, CExprPtr initExpr,
            Token *start = nullptr, Token *end = nullptr)
            : CDecl(CK_ENUMERATOR_DECL, std::move(name), start, end)
            , initExpr(std::move(initExpr))
        {
        }
    };

    struct CEnumDecl : public CDecl
    {
        CNodeList enumerators;
        CEnumDecl(
            std::string name, Token *start = nullptr, Token *end = nullptr)
            : CDecl(CK_ENUM_DECL, std::move(name), start, end)
        {
        }
        CEnumDecl(std::string name, CNodeList enumerators,
            Token *start = nullptr, Token *end = nullptr)
            : CEnumDecl(std::move(name), start, end)
            , enumerators(std::move(enumerators))
        {
        }
    };

    struct CStructDecl : public CDecl
    {
        CNodeList members;
        CStructDecl(
            std::string name, Token *start = nullptr, Token *end = nullptr)
            : CDecl(CK_STRUCT_DECL, std::move(name), start, end)
        {
        }
        CStructDecl(std::string name, CNodeList members, Token *start = nullptr,
            Token *end = nullptr)
            : CStructDecl(std::move(name), start, end)
            , members(std::move(members))
        {
        }
    };

    struct CDefineMacro : public CMacro
    {
        std::string name;
        std::string code;
        std::vector< std::string > params;
        CDefineMacro(std::string name, std::string code, Node *start = nullptr,
            Node *end = nullptr)
            : CMacro(CK_DEFINE_MACRO, start, end)
            , name(std::move(name))
            , code(std::move(code))
        {
        }
        CDefineMacro(std::string name, std::string code,
            std::vector< std::string > params, Node *start = nullptr,
            Node *end = nullptr)
            : CDefineMacro(std::move(name), std::move(code), start, end)
            , params(std::move(params))
        {
        }
    };

    struct CUndefMacro : public CMacro
    {
        std::string name;
        CUndefMacro(
            std::string name, Node *start = nullptr, Node *end = nullptr)
            : CMacro(CK_UNDEF_MACRO, start, end)
            , name(std::move(name))
        {
        }
    };

    struct CIfMacro : public CMacro
    {
        std::string condText;
        CIfMacro(
            std::string condText, Node *start = nullptr, Node *end = nullptr)
            : CMacro(CK_IF_MACRO, start, end)
            , condText(std::move(condText))
        {
        }
    };

    struct CElifMacro : public CMacro
    {
        std::string condText;
        std::string code;
        CElifMacro(std::string condText, std::string code,
            Node *start = nullptr, Node *end = nullptr)
            : CMacro(CK_ELIF_MACRO, start, end)
            , condText(std::move(condText))
            , code(std::move(code))
        {
        }
    };

    struct CElseMacro : public CMacro
    {
        CElseMacro(Token *start = nullptr, Token *end = nullptr)
            : CMacro(CK_ELSE_MACRO, start, end)
        {
        }
    };

    struct CEndifMacro : public CMacro
    {
        CEndifMacro(Token *start = nullptr, Token *end = nullptr)
            : CMacro(CK_ENDIF_MACRO, start, end)
        {
        }
    };

    struct CFile : CNode
    {
        CNodeList children;
        CFile(Token *start = nullptr, Token *end = nullptr)
            : CNode(CK_FILE, start, end)
        {
        }
    };

    typedef CNodePtrType< CFile > CFilePtr;

} // namespace Soda
