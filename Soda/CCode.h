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
        CK_IF_EXPR,
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
        CK_TYPEDEF_DECL,
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
        CK_TYPEREF,
        CK_FILE,
    };

    template < class... Args >
    using CNodePtrType = std::unique_ptr< Args... >;

    template < class... Args >
    using CNodeListType = std::vector< Args... >;

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
        CNullLit(Token *start, Token *end)
            : CExpr(CK_NULL_LIT, start, end)
        {
        }
    };

    struct CBoolLit : public CExpr
    {
        bool value;
        CBoolLit(bool value, Token *start, Token *end)
            : CExpr(CK_BOOL_LIT, start, end)
            , value(value)
        {
        }
    };

    struct CIntLit : public CExpr
    {
        unsigned long long value;
        CIntLit(unsigned long long value, Token *start, Token *end)
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
        CCharLit(std::string value, Token *start, Token *end)
            : CExpr(CK_CHAR_LIT, start, end)
            , value(std::move(value))
        {
        }
    };

    struct CStringLit : public CExpr
    {
        std::string value;
        CStringLit(std::string value, Token *start, Token *end)
            : CExpr(CK_STRING_LIT, start, end)
            , value(std::move(value))
        {
        }
    };

    struct CIdentifier : public CExpr
    {
        std::string name;
        CIdentifier(std::string name, Token *start, Token *end)
            : CExpr(CK_IDENTIFIER, start, end)
            , name(std::move(name))
        {
        }
    };

    struct CUnaryExpr : public CExpr
    {
        UnaryOperator op;
        CNodeList children;
        CUnaryExpr(UnaryOperator op, Token *start, Token *end)
            : CExpr(CK_UNARY_EXPR, start, end)
            , op(op)
        {
        }
    };

    struct CBinaryExpr : public CExpr
    {
        BinaryOperator op;
        CNodeList children;
        CBinaryExpr(BinaryOperator op, Token *start, Token *end)
            : CExpr(CK_BINARY_EXPR, start, end)
            , op(op)
        {
        }
    };

    struct CTypeRef : public CNode
    {
        std::string name;
        CNodeList children;
        bool isPointer;
        bool isConst;
        bool isArray;
        CTypeRef(std::string name, Token *start, Token *end)
            : CNode(CK_TYPEREF, start, end)
            , name(std::move(name))
            , isPointer(false)
            , isConst(false)
            , isArray(false)
        {
        }
    };

    typedef CNodePtrType< CTypeRef > CTypeRefPtr;

    struct CCastExpr : public CExpr
    {
        CNodeList children;
        CCastExpr(Token *start, Token *end)
            : CExpr(CK_CAST_EXPR, start, end)
        {
        }
    };

    struct CIfExpr : public CExpr
    {
        CNodeList children;
        CIfExpr(Token *start, Token *end)
            : CExpr(CK_IF_EXPR, start, end)
        {
        }
    };

    struct CCallExpr : public CExpr
    {
        CNodeList children;
        CCallExpr(Token *start, Token *end)
            : CExpr(CK_CALL_EXPR, start, end)
        {
        }
    };

    struct CIndexExpr : public CExpr
    {
        CNodeList children;
        CIndexExpr(Token *start, Token *end)
            : CExpr(CK_INDEX_EXPR, start, end)
        {
        }
    };

    struct CMemberExpr : public CExpr
    {
        CNodeList children;
        CMemberExpr(Token *start, Token *end)
            : CExpr(CK_MEMBER_EXPR, start, end)
        {
        }
    };

    struct CExprStmt : public CStmt
    {
        CNodeList children;
        CExprStmt(Token *start, Token *end)
            : CStmt(CK_EXPR_STMT, start, end)
        {
        }
    };

    struct CBlockStmt : public CStmt
    {
        CNodeList children;
        CBlockStmt(Token *start, Token *end)
            : CStmt(CK_BLOCK_STMT, start, end)
        {
        }
    };

    struct CReturnStmt : public CStmt
    {
        CNodeList children;
        CReturnStmt(Token *start, Token *end)
            : CStmt(CK_RETURN_STMT, start, end)
        {
        }
    };

    struct CGotoStmt : public CStmt
    {
        std::string label;
        CGotoStmt(std::string label, Token *start, Token *end)
            : CStmt(CK_GOTO_STMT, start, end)
            , label(std::move(label))
        {
        }
    };

    struct CIfStmt : public CStmt
    {
        CNodeList children;
        CIfStmt(Token *start, Token *end)
            : CStmt(CK_IF_STMT, start, end)
        {
        }
    };

    struct CDoStmt : public CStmt
    {
        CNodeList children;
        CDoStmt(Token *start, Token *end)
            : CStmt(CK_DO_STMT, start, end)
        {
        }
    };

    struct CWhileStmt : public CStmt
    {
        CNodeList children;
        CWhileStmt(Token *start, Token *end)
            : CStmt(CK_WHILE_STMT, start, end)
        {
        }
    };

    struct CTypedef : public CDecl
    {
        CNodeList children;
        CTypedef(std::string name, Token *start, Token *end)
            : CDecl(CK_TYPEDEF_DECL, std::move(name), start, end)
        {
        }
    };

    struct CVarDecl : public CDecl
    {
        CNodeList children;
        CVarDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_VAR_DECL, std::move(name), start, end)
        {
        }
    };

    struct CParamDecl : public CDecl
    {
        CNodeList children;
        CParamDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_PARAM_DECL, std::move(name), start, end)
        {
        }
    };

    struct CFuncDecl : public CDecl
    {
        CNodeList children;
        CFuncDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_FUNC_DECL, std::move(name), start, end)
        {
        }
    };

    struct CEnumeratorDecl : public CDecl
    {
        CNodeList children;
        CEnumeratorDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_ENUMERATOR_DECL, std::move(name), start, end)
        {
        }
    };

    struct CEnumDecl : public CDecl
    {
        CNodeList enumerators;
        CEnumDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_ENUM_DECL, std::move(name), start, end)
        {
        }
    };

    struct CStructDecl : public CDecl
    {
        CNodeList members;
        CStructDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_STRUCT_DECL, std::move(name), start, end)
        {
        }
    };

    struct CDefineMacro : public CMacro
    {
        std::string name;
        std::string code;
        std::vector< std::string > params;
        CDefineMacro(std::string name, std::string code, Token *start = nullptr,
            Token *end = nullptr)
            : CMacro(CK_DEFINE_MACRO, start, end)
            , name(std::move(name))
            , code(std::move(code))
        {
        }
        CDefineMacro(std::string name, std::string code,
            std::vector< std::string > params, Token *start = nullptr,
            Token *end = nullptr)
            : CMacro(CK_DEFINE_MACRO, start, end)
            , name(std::move(name))
            , code(std::move(code))
            , params(std::move(params))
        {
        }
    };

    struct CUndefMacro : public CMacro
    {
        std::string name;
        CUndefMacro(
            std::string name, Token *start = nullptr, Token *end = nullptr)
            : CMacro(CK_UNDEF_MACRO, start, end)
            , name(std::move(name))
        {
        }
    };

    struct CIfMacro : public CMacro
    {
        std::string condText;
        CIfMacro(
            std::string condText, Token *start = nullptr, Token *end = nullptr)
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
            Token *start = nullptr, Token *end = nullptr)
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
