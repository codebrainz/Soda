#pragma once

#include "CCodeVisitor.h"
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
        CNodeListType< CNodePtrType< CNode > > children;

        CNode(CNodeKind kind, Token *start, Token *end)
            : kind(kind)
            , start(start)
            , end(end)
        {
        }

        virtual ~CNode()
        {
        }

        virtual void accept(CCodeVisitor &) = 0;

        virtual void acceptChildren(CCodeVisitor &v)
        {
            for (auto &child : children)
                child->accept(v);
        }

        virtual const std::string &kindName() const = 0;
    };

#define CCODE_VISITABLE(nm)                                    \
    virtual void accept(CCodeVisitor &v) override final        \
    {                                                          \
        v.visit(*this);                                        \
    }                                                          \
    virtual const std::string &kindName() const override final \
    {                                                          \
        static const std::string n(#nm);                       \
        return n;                                              \
    }

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

    struct CNullLit final : public CExpr
    {
        CNullLit(Token *start, Token *end)
            : CExpr(CK_NULL_LIT, start, end)
        {
        }
        CCODE_VISITABLE(CNullLit)
    };

    struct CBoolLit final : public CExpr
    {
        bool value;
        CBoolLit(bool value, Token *start, Token *end)
            : CExpr(CK_BOOL_LIT, start, end)
            , value(value)
        {
        }
        CCODE_VISITABLE(CBoolLit)
    };

    struct CIntLit final : public CExpr
    {
        unsigned long long value;
        CIntLit(unsigned long long value, Token *start, Token *end)
            : CExpr(CK_INT_LIT, start, end)
            , value(value)
        {
        }
        CCODE_VISITABLE(CIntLit)
    };

    struct CFloatLit final : public CExpr
    {
        long double value;
        CFloatLit(long double value, Token *start, Token *end)
            : CExpr(CK_FLOAT_LIT, start, end)
            , value(value)
        {
        }
        CCODE_VISITABLE(CFloatLit)
    };

    struct CCharLit final : public CExpr
    {
        std::string value;
        CCharLit(std::string value, Token *start, Token *end)
            : CExpr(CK_CHAR_LIT, start, end)
            , value(std::move(value))
        {
        }
        CCODE_VISITABLE(CCharLit)
    };

    struct CStringLit final : public CExpr
    {
        std::string value;
        CStringLit(std::string value, Token *start, Token *end)
            : CExpr(CK_STRING_LIT, start, end)
            , value(std::move(value))
        {
        }
        CCODE_VISITABLE(CStringLit)
    };

    struct CIdentifier final : public CExpr
    {
        std::string name;
        CIdentifier(std::string name, Token *start, Token *end)
            : CExpr(CK_IDENTIFIER, start, end)
            , name(std::move(name))
        {
        }
        CCODE_VISITABLE(CIdentifier)
    };

    struct CUnaryExpr final : public CExpr
    {
        UnaryOperator op;
        CUnaryExpr(UnaryOperator op, Token *start, Token *end)
            : CExpr(CK_UNARY_EXPR, start, end)
            , op(op)
        {
        }
        CCODE_VISITABLE(CUnaryExpr)
    };

    struct CBinaryExpr final : public CExpr
    {
        BinaryOperator op;
        CBinaryExpr(BinaryOperator op, Token *start, Token *end)
            : CExpr(CK_BINARY_EXPR, start, end)
            , op(op)
        {
        }
        CCODE_VISITABLE(CBinaryExpr)
    };

    struct CTypeRef final : public CNode
    {
        std::string name;
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
        CCODE_VISITABLE(CTypeRef)
    };

    typedef CNodePtrType< CTypeRef > CTypeRefPtr;

    struct CCastExpr final : public CExpr
    {
        CCastExpr(Token *start, Token *end)
            : CExpr(CK_CAST_EXPR, start, end)
        {
        }
        CCODE_VISITABLE(CCastExpr)
    };

    struct CIfExpr final : public CExpr
    {
        CIfExpr(Token *start, Token *end)
            : CExpr(CK_IF_EXPR, start, end)
        {
        }
        CCODE_VISITABLE(CIfExpr)
    };

    struct CCallExpr final : public CExpr
    {
        CCallExpr(Token *start, Token *end)
            : CExpr(CK_CALL_EXPR, start, end)
        {
        }
        CCODE_VISITABLE(CCallExpr)
    };

    struct CIndexExpr final : public CExpr
    {
        CIndexExpr(Token *start, Token *end)
            : CExpr(CK_INDEX_EXPR, start, end)
        {
        }
        CCODE_VISITABLE(CIndexExpr)
    };

    struct CMemberExpr final : public CExpr
    {
        CMemberExpr(Token *start, Token *end)
            : CExpr(CK_MEMBER_EXPR, start, end)
        {
        }
        CCODE_VISITABLE(CMemberExpr)
    };

    struct CExprStmt final : public CStmt
    {
        CExprStmt(Token *start, Token *end)
            : CStmt(CK_EXPR_STMT, start, end)
        {
        }
        CCODE_VISITABLE(CExprStmt)
    };

    struct CBlockStmt final : public CStmt
    {
        CBlockStmt(Token *start, Token *end)
            : CStmt(CK_BLOCK_STMT, start, end)
        {
        }
        CCODE_VISITABLE(CBlockStmt)
    };

    struct CReturnStmt final : public CStmt
    {
        CReturnStmt(Token *start, Token *end)
            : CStmt(CK_RETURN_STMT, start, end)
        {
        }
        CCODE_VISITABLE(CBlockStmt)
    };

    struct CGotoStmt final : public CStmt
    {
        std::string label;
        CGotoStmt(std::string label, Token *start, Token *end)
            : CStmt(CK_GOTO_STMT, start, end)
            , label(std::move(label))
        {
        }
        CCODE_VISITABLE(CGotoStmt)
    };

    struct CIfStmt final : public CStmt
    {
        CIfStmt(Token *start, Token *end)
            : CStmt(CK_IF_STMT, start, end)
        {
        }
        CCODE_VISITABLE(CIfStmt)
    };

    struct CDoStmt final : public CStmt
    {
        CDoStmt(Token *start, Token *end)
            : CStmt(CK_DO_STMT, start, end)
        {
        }
        CCODE_VISITABLE(CDoStmt)
    };

    struct CWhileStmt final : public CStmt
    {
        CWhileStmt(Token *start, Token *end)
            : CStmt(CK_WHILE_STMT, start, end)
        {
        }
        CCODE_VISITABLE(CWhileStmt)
    };

    struct CTypedef final : public CDecl
    {
        CTypedef(std::string name, Token *start, Token *end)
            : CDecl(CK_TYPEDEF_DECL, std::move(name), start, end)
        {
        }
        CCODE_VISITABLE(CTypedef)
    };

    struct CVarDecl final : public CDecl
    {
        CVarDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_VAR_DECL, std::move(name), start, end)
        {
        }
        CCODE_VISITABLE(CVarDecl)
    };

    struct CParamDecl final : public CDecl
    {
        CParamDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_PARAM_DECL, std::move(name), start, end)
        {
        }
        CCODE_VISITABLE(CParamDecl)
    };

    struct CFuncDecl final : public CDecl
    {
        CFuncDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_FUNC_DECL, std::move(name), start, end)
        {
        }
        CCODE_VISITABLE(CFuncDecl)
    };

    struct CEnumeratorDecl final : public CDecl
    {
        CEnumeratorDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_ENUMERATOR_DECL, std::move(name), start, end)
        {
        }
        CCODE_VISITABLE(CEnumeratorDecl)
    };

    struct CEnumDecl final : public CDecl
    {
        CEnumDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_ENUM_DECL, std::move(name), start, end)
        {
        }
        CCODE_VISITABLE(CEnumDecl)
    };

    struct CStructDecl final : public CDecl
    {
        CStructDecl(std::string name, Token *start, Token *end)
            : CDecl(CK_STRUCT_DECL, std::move(name), start, end)
        {
        }
        CCODE_VISITABLE(CStructDecl)
    };

    struct CDefineMacro final : public CMacro
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
        CCODE_VISITABLE(CDefineMacro)
    };

    struct CUndefMacro final : public CMacro
    {
        std::string name;
        CUndefMacro(
            std::string name, Token *start = nullptr, Token *end = nullptr)
            : CMacro(CK_UNDEF_MACRO, start, end)
            , name(std::move(name))
        {
        }
        CCODE_VISITABLE(CUndefMacro)
    };

    struct CIfMacro final : public CMacro
    {
        std::string condText;
        CIfMacro(
            std::string condText, Token *start = nullptr, Token *end = nullptr)
            : CMacro(CK_IF_MACRO, start, end)
            , condText(std::move(condText))
        {
        }
        CCODE_VISITABLE(CIfMacro)
    };

    struct CElifMacro final : public CMacro
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
        CCODE_VISITABLE(CElifMacro)
    };

    struct CElseMacro final : public CMacro
    {
        CElseMacro(Token *start = nullptr, Token *end = nullptr)
            : CMacro(CK_ELSE_MACRO, start, end)
        {
        }
        CCODE_VISITABLE(CElseMacro)
    };

    struct CEndifMacro final : public CMacro
    {
        CEndifMacro(Token *start = nullptr, Token *end = nullptr)
            : CMacro(CK_ENDIF_MACRO, start, end)
        {
        }
        CCODE_VISITABLE(CEndifMacro)
    };

    struct CFile final : CNode
    {
        CNodeList children;
        CFile(Token *start = nullptr, Token *end = nullptr)
            : CNode(CK_FILE, start, end)
        {
        }
        CCODE_VISITABLE(CFile)
    };

    typedef CNodePtrType< CFile > CFilePtr;

} // namespace Soda
