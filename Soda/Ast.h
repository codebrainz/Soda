#pragma once

#include "Operators.h"
#include "SourceFile.h"
#include "SymbolTable.h"
#include "Tokenizer.h"
#include "Visitor.h"
#include <memory>
#include <string>
#include <vector>

namespace Soda
{

    template < class T >
    using NodePtrType = std::unique_ptr< T >;

    enum AstNodeKind
    {
        NK_NIL,
        NK_BOOL,
        NK_INT,
        NK_FLOAT,
        NK_CHAR,
        NK_STRING,
        NK_IDENTIFIER,
        NK_UNARY,
        NK_BINARY,
        NK_TYPEREF,
        NK_CAST,
        NK_IF_EXPR,
        NK_CALL_EXPR,
        NK_INDEX_EXPR,
        NK_MEMBER_EXPR,
        NK_EMPTY_STMT,
        NK_EXPR_STMT,
        NK_BLOCK_STMT,
        NK_RETURN_STMT,
        NK_BREAK_STMT,
        NK_CONTINUE_STMT,
        NK_GOTO_STMT,
        NK_IF_STMT,
        NK_CASE_STMT,
        NK_SWITCH_STMT,
        NK_FOR_STMT,
        NK_DO_STMT,
        NK_WHILE_STMT,
        NK_CATCH_STMT,
        NK_FINALLY_STMT,
        NK_TRY_STMT,
        NK_ATTR_BOOL,
        NK_ATTR_INT,
        NK_ATTR_FLOAT,
        NK_LABEL_DECL,
        NK_USING_DECL,
        NK_TYPENAME,
        NK_TYPEDEF_DECL,
        NK_NAMESPACE_DECL,
        NK_VAR_DECL,
        NK_PARAM_DECL,
        NK_FUNC_DECL,
        NK_DELEGATE_DECL,
        NK_CONSTRUCTOR_DECL,
        NK_DESTRUCTOR_DECL,
        NK_STRUCT_DECL,
        NK_ENUMERATOR_DECL,
        NK_ENUM_DECL,
        NK_MODULE,
    };

    struct AstNode
    {
        AstNodeKind kind;
        Token *start;
        Token *end;
        AstNode *parentNode;
        SymbolTable *ownerScope;
        AstNode(AstNodeKind kind, Token *start, Token *end)
            : kind(kind)
            , start(start)
            , end(end)
            , parentNode(nullptr)
            , ownerScope(nullptr)
        {
        }
        ~AstNode()
        {
        }
        virtual void accept(AstVisitor &) = 0;
        virtual void acceptChildren(AstVisitor &)
        {
        }
        virtual const std::string &kindName() const = 0;
    };

#define AST_VISITABLE(name)                                    \
    virtual void accept(AstVisitor &v) override final          \
    {                                                          \
        v.visit(*this);                                        \
    }                                                          \
    virtual const std::string &kindName() const override final \
    {                                                          \
        static const std::string nm(#name);                    \
        return nm;                                             \
    }

    struct AstExpr : public AstNode
    {
        AstExpr(AstNodeKind kind, Token *start, Token *end)
            : AstNode(kind, start, end)
        {
        }
    };

    struct AstStmt : public AstNode
    {
        AstStmt(AstNodeKind kind, Token *start, Token *end)
            : AstNode(kind, start, end)
        {
        }
    };

    struct AstAttribute : public AstNode
    {
        AstAttribute(AstNodeKind kind, Token *start, Token *end)
            : AstNode(kind, start, end)
        {
        }
    };

    typedef NodePtrType< AstAttribute > AstAttributePtr;
    typedef std::vector< AstAttributePtr > AstAttributeList;

    enum DeclFlags
    {
        DF_NONE = 0,
        DF_STATIC = 1,
    };

    struct AstDecl : public AstStmt
    {
        DeclFlags flags;
        std::string name;
        AstAttributeList attributes;
        AstDecl(DeclFlags flags, std::string name, AstNodeKind kind,
            Token *start, Token *end)
            : AstStmt(kind, start, end)
            , flags(flags)
            , name(std::move(name))
        {
        }
        virtual bool isTypeDecl() const
        {
            return false;
        }
    };

    struct AstTypeDecl : public AstDecl
    {
        AstTypeDecl(DeclFlags flags, std::string name, AstNodeKind kind,
            Token *start, Token *end)
            : AstDecl(flags, std::move(name), kind, start, end)
        {
        }
        virtual bool isTypeDecl() const override final
        {
            return true;
        }
    };

    typedef NodePtrType< AstNode > AstNodePtr;
    typedef NodePtrType< AstExpr > AstExprPtr;
    typedef NodePtrType< AstStmt > AstStmtPtr;
    typedef NodePtrType< AstDecl > AstDeclPtr;

    typedef std::vector< AstNodePtr > AstNodeList;
    typedef std::vector< AstExprPtr > AstExprList;
    typedef std::vector< AstStmtPtr > AstStmtList;
    typedef std::vector< AstDeclPtr > AstDeclList;

    struct AstNil final : public AstExpr
    {
        AstNil(Token *start, Token *end)
            : AstExpr(NK_NIL, start, end)
        {
        }
        AST_VISITABLE(Nil)
    };

    struct AstBool final : public AstExpr
    {
        bool value;
        AstBool(bool value, Token *start, Token *end)
            : AstExpr(NK_BOOL, start, end)
            , value(value)
        {
        }
        AST_VISITABLE(Bool)
    };

    struct AstInt final : public AstExpr
    {
        unsigned long long int value;
        AstInt(unsigned long long int value, Token *start, Token *end)
            : AstExpr(NK_INT, start, end)
            , value(value)
        {
        }
        AST_VISITABLE(Int)
    };

    struct AstFloat final : public AstExpr
    {
        long double value;
        AstFloat(long double value, Token *start, Token *end)
            : AstExpr(NK_FLOAT, start, end)
            , value(value)
        {
        }
        AST_VISITABLE(Float)
    };

    struct AstChar final : public AstExpr
    {
        std::string value;
        AstChar(std::string value, Token *start, Token *end)
            : AstExpr(NK_CHAR, start, end)
            , value(std::move(value))
        {
        }
        AST_VISITABLE(Char)
    };

    struct AstString final : public AstExpr
    {
        std::string value;
        AstString(std::string value, Token *start, Token *end)
            : AstExpr(NK_STRING, start, end)
            , value(std::move(value))
        {
        }
        AST_VISITABLE(String)
    };

    struct AstIdentifier final : public AstExpr
    {
        std::string name;
        Symbol *refSymbol;
        AstIdentifier(std::string name, Token *start, Token *end)
            : AstExpr(NK_IDENTIFIER, start, end)
            , name(name)
            , refSymbol(nullptr)
        {
        }
        AST_VISITABLE(Identifier)
    };

    struct AstUnary final : public AstExpr
    {
        UnaryOperator op;
        AstExprPtr operand;
        AstUnary(UnaryOperator op, AstExprPtr operand, Token *start, Token *end)
            : AstExpr(NK_UNARY, start, end)
            , op(op)
            , operand(std::move(operand))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            operand->accept(v);
        }
        AST_VISITABLE(Unary)
    };

    struct AstBinary final : public AstExpr
    {
        BinaryOperator op;
        AstExprPtr left;
        AstExprPtr right;
        AstBinary(BinaryOperator op, AstExprPtr left, AstExprPtr right,
            Token *start, Token *end)
            : AstExpr(NK_BINARY, start, end)
            , op(op)
            , left(std::move(left))
            , right(std::move(right))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            left->accept(v);
            right->accept(v);
        }
        AST_VISITABLE(Binary)
    };

    enum TypeFlags
    {
        TF_NONE = 0,
        TF_CONST = 1,
        TF_POINTER = 2,
        TF_ARRAY = 4,
    };

    struct AstTypeRef final : public AstNode
    {
        std::string name;
        NodePtrType< AstTypeRef > refType;
        TypeFlags typeFlags;
        AstTypeRef(
            std::string name, TypeFlags typeFlags, Token *start, Token *end)
            : AstNode(NK_TYPEREF, start, end)
            , name(std::move(name))
            , typeFlags(typeFlags)
        {
        }
        AstTypeRef(NodePtrType< AstTypeRef > refType, TypeFlags typeFlags,
            Token *start, Token *end)
            : AstNode(NK_TYPEREF, start, end)
            , refType(std::move(refType))
            , typeFlags(typeFlags)
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (refType)
                refType->accept(v);
        }
        AST_VISITABLE(TypeRef)
    };

    typedef NodePtrType< AstTypeRef > AstTypeRefPtr;
    typedef std::vector< AstTypeRefPtr > AstTypeRefList;

    struct AstCast final : public AstExpr
    {
        AstTypeRefPtr typeRef;
        AstExprPtr expr;
        AstCast(
            AstTypeRefPtr typeRef, AstExprPtr expr, Token *start, Token *end)
            : AstExpr(NK_CAST, start, end)
            , typeRef(std::move(typeRef))
            , expr(std::move(expr))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            typeRef->accept(v);
            expr->accept(v);
        }
        AST_VISITABLE(Cast)
    };

    struct AstIfExpr final : public AstExpr
    {
        AstExprPtr condExpr;
        AstExprPtr thenExpr;
        AstExprPtr elseExpr;
        AstIfExpr(AstExprPtr condExpr, AstExprPtr thenExpr, AstExprPtr elseExpr,
            Token *start, Token *end)
            : AstExpr(NK_IF_EXPR, start, end)
            , condExpr(std::move(condExpr))
            , thenExpr(std::move(thenExpr))
            , elseExpr(std::move(elseExpr))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            condExpr->accept(v);
            thenExpr->accept(v);
            elseExpr->accept(v);
        }
        AST_VISITABLE(IfExpr)
    };

    struct AstCallExpr final : public AstExpr
    {
        AstExprPtr callee;
        AstExprList arguments;
        AstCallExpr(
            AstExprPtr callee, AstExprList args, Token *start, Token *end)
            : AstExpr(NK_CALL_EXPR, start, end)
            , callee(std::move(callee))
            , arguments(std::move(args))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            callee->accept(v);
            for (auto &arg : arguments)
                arg->accept(v);
        }
        AST_VISITABLE(CallExpr)
    };

    struct AstIndexExpr final : public AstExpr
    {
        AstExprPtr object;
        AstExprPtr index;
        AstIndexExpr(
            AstExprPtr object, AstExprPtr index, Token *start, Token *end)
            : AstExpr(NK_INDEX_EXPR, start, end)
            , object(std::move(object))
            , index(std::move(index))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            object->accept(v);
            index->accept(v);
        }
        AST_VISITABLE(IndexExpr)
    };

    struct AstMemberExpr final : public AstExpr
    {
        AstExprPtr object;
        AstExprPtr member;
        AstMemberExpr(
            AstExprPtr object, AstExprPtr member, Token *start, Token *end)
            : AstExpr(NK_MEMBER_EXPR, start, end)
            , object(std::move(object))
            , member(std::move(member))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            object->accept(v);
            member->accept(v);
        }
        AST_VISITABLE(MemberExpr)
    };

    struct AstEmptyStmt final : public AstDecl // allows using as a decl or stmt
    {
        AstEmptyStmt(Token *start, Token *end)
            : AstDecl(DF_NONE, "", NK_EMPTY_STMT, start, end)
        {
        }
        AST_VISITABLE(EmptyStmt)
    };

    struct AstExprStmt final : public AstStmt
    {
        AstExprPtr expr;
        AstExprStmt(AstExprPtr expr, Token *start, Token *end)
            : AstStmt(NK_EXPR_STMT, start, end)
            , expr(std::move(expr))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            expr->accept(v);
        }
        AST_VISITABLE(ExprStmt)
    };

    struct AstBlockStmt final : public AstStmt
    {
        SymbolTable scope;
        AstStmtList stmts;
        AstBlockStmt(AstStmtList stmts, Token *start, Token *end)
            : AstStmt(NK_BLOCK_STMT, start, end)
            , stmts(std::move(stmts))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            for (auto &stmt : stmts)
                stmt->accept(v);
        }
        AST_VISITABLE(BlockStmt)
    };

    struct AstReturnStmt final : public AstStmt
    {
        AstExprPtr expr;
        AstReturnStmt(AstExprPtr expr, Token *start, Token *end)
            : AstStmt(NK_RETURN_STMT, start, end)
            , expr(std::move(expr))
        {
        }
        bool returnsValue() const
        {
            return expr ? true : false;
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (expr)
                expr->accept(v);
        }
        AST_VISITABLE(ReturnStmt)
    };

    struct AstBreakStmt final : public AstStmt
    {
        AstBreakStmt(Token *start, Token *end)
            : AstStmt(NK_BREAK_STMT, start, end)
        {
        }
        AST_VISITABLE(BreakStmt)
    };

    struct AstContinueStmt final : public AstStmt
    {
        AstContinueStmt(Token *start, Token *end)
            : AstStmt(NK_CONTINUE_STMT, start, end)
        {
        }
        AST_VISITABLE(ContinueStmt)
    };

    struct AstGotoStmt final : public AstStmt
    {
        std::string label;
        Symbol *refSymbol;
        AstGotoStmt(std::string label, Token *start, Token *end)
            : AstStmt(NK_GOTO_STMT, start, end)
            , label(std::move(label))
            , refSymbol(nullptr)
        {
        }
        AST_VISITABLE(GotoStmt)
    };

    struct AstIfStmt final : public AstStmt
    {
        SymbolTable scope;
        AstNodePtr condNode;
        AstStmtPtr thenStmt;
        AstStmtPtr elseStmt;
        AstIfStmt(AstNodePtr condNode, AstStmtPtr thenStmt, AstStmtPtr elseStmt,
            Token *start, Token *end)
            : AstStmt(NK_IF_STMT, start, end)
            , condNode(std::move(condNode))
            , thenStmt(std::move(thenStmt))
            , elseStmt(std::move(elseStmt))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            condNode->accept(v);
            thenStmt->accept(v);
            if (elseStmt)
                elseStmt->accept(v);
        }
        AST_VISITABLE(IfStmt)
    };

    struct AstCaseStmt final : public AstStmt
    {
        SymbolTable scope;
        AstExprPtr expr;
        AstStmtList stmts;
        AstCaseStmt(
            AstExprPtr expr, AstStmtList stmts, Token *start, Token *end)
            : AstStmt(NK_CASE_STMT, start, end)
            , expr(std::move(expr))
            , stmts(std::move(stmts))
        {
        }
        bool isDefaultCase() const
        {
            return expr ? true : false;
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (expr)
                expr->accept(v);
            for (auto &stmt : stmts)
                stmt->accept(v);
        }
        AST_VISITABLE(AstCaseStmt)
    };

    struct AstSwitchStmt final : public AstStmt
    {
        SymbolTable scope;
        AstNodePtr testNode;
        AstStmtList cases;
        AstSwitchStmt(
            AstNodePtr testNode, AstStmtList cases, Token *start, Token *end)
            : AstStmt(NK_SWITCH_STMT, start, end)
            , testNode(std::move(testNode))
            , cases(std::move(cases))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            testNode->accept(v);
            for (auto &case_ : cases)
                case_->accept(v);
        }
        AST_VISITABLE(AstSwitchStmt)
    };

    struct AstForStmt final : public AstStmt
    {
        SymbolTable scope;
        AstNodePtr initNode;
        AstExprPtr testExpr;
        AstExprPtr incrExpr;
        AstStmtPtr stmt;
        AstForStmt(AstNodePtr initNode, AstExprPtr testExpr,
            AstExprPtr incrExpr, AstStmtPtr stmt, Token *start, Token *end)
            : AstStmt(NK_FOR_STMT, start, end)
            , initNode(std::move(initNode))
            , testExpr(std::move(testExpr))
            , incrExpr(std::move(incrExpr))
            , stmt(std::move(stmt))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (initNode)
                initNode->accept(v);
            if (testExpr)
                testExpr->accept(v);
            if (incrExpr)
                incrExpr->accept(v);
            stmt->accept(v);
        }
        AST_VISITABLE(ForStmt)
    };

    struct AstDoStmt final : public AstStmt
    {
        AstStmtPtr stmt;
        AstExprPtr expr;
        AstDoStmt(AstStmtPtr stmt, AstExprPtr expr, Token *start, Token *end)
            : AstStmt(NK_DO_STMT, start, end)
            , stmt(std::move(stmt))
            , expr(std::move(expr))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            stmt->accept(v);
            expr->accept(v);
        }
        AST_VISITABLE(DoStmt)
    };

    struct AstWhileStmt final : public AstStmt
    {
        AstExprPtr expr;
        AstStmtPtr stmt;
        AstWhileStmt(AstExprPtr expr, AstStmtPtr stmt, Token *start, Token *end)
            : AstStmt(NK_WHILE_STMT, start, end)
            , expr(std::move(expr))
            , stmt(std::move(stmt))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            expr->accept(v);
            stmt->accept(v);
        }
        AST_VISITABLE(WhileStmt)
    };

    struct AstCatchStmt final : public AstStmt
    {
        SymbolTable scope;
        AstDeclPtr exc;
        AstStmtPtr stmt;
        AstCatchStmt(AstDeclPtr exc, AstStmtPtr stmt, Token *start, Token *end)
            : AstStmt(NK_CATCH_STMT, start, end)
            , exc(std::move(exc))
            , stmt(std::move(stmt))
        {
        }
        bool isCatchAll() const
        {
            return exc ? false : true;
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (exc)
                exc->accept(v);
            stmt->accept(v);
        }
        AST_VISITABLE(CatchStmt)
    };

    typedef NodePtrType< AstCatchStmt > AstCatchPtr;
    typedef std::vector< AstCatchPtr > AstCatchList;

    struct AstFinallyStmt final : public AstStmt
    {
        AstStmtPtr stmt;
        AstFinallyStmt(AstStmtPtr stmt, Token *start, Token *end)
            : AstStmt(NK_FINALLY_STMT, start, end)
            , stmt(std::move(stmt))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            stmt->accept(v);
        }
        AST_VISITABLE(FinallyStmt)
    };

    typedef NodePtrType< AstFinallyStmt > AstFinallyPtr;

    struct AstTryStmt final : public AstStmt
    {
        AstStmtPtr stmt;
        AstCatchList catchStmts;
        AstFinallyPtr finallyStmt;
        AstTryStmt(AstStmtPtr stmt, AstCatchList catchStmts, AstFinallyPtr fin,
            Token *start, Token *end)
            : AstStmt(NK_TRY_STMT, start, end)
            , stmt(std::move(stmt))
            , catchStmts(std::move(catchStmts))
            , finallyStmt(std::move(fin))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            stmt->accept(v);
            for (auto &c : catchStmts)
                c->accept(v);
            if (finallyStmt)
                finallyStmt->accept(v);
        }
        AST_VISITABLE(TryStmt)
    };

    struct AstBoolAttribute final : public AstAttribute
    {
        AstBoolAttribute(Token *start, Token *end)
            : AstAttribute(NK_ATTR_BOOL, start, end)
        {
        }
        AST_VISITABLE(BoolAttribute)
    };

    struct AstIntAttribute final : public AstAttribute
    {
        int rank;
        int width;
        bool isSigned;
        uint64_t min;
        uint64_t max;
        AstIntAttribute(Token *start, Token *end)
            : AstAttribute(NK_ATTR_INT, start, end)
            , rank(0)
            , width(0)
            , isSigned(false)
            , min(0)
            , max(0)
        {
        }
        AST_VISITABLE(IntAttribute)
    };

    struct AstFloatAttribute final : public AstAttribute
    {
        int rank;
        int width;
        AstFloatAttribute(Token *start, Token *end)
            : AstAttribute(NK_ATTR_FLOAT, start, end)
            , rank(0)
            , width(0)
        {
        }
        AST_VISITABLE(FloatAttribute)
    };

    struct AstLabelDecl final : public AstDecl
    {
        AstStmtPtr stmt;
        AstLabelDecl(
            std::string name, AstStmtPtr stmt, Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_LABEL_DECL, start, end)
            , stmt(std::move(stmt))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            stmt->accept(v);
        }
        AST_VISITABLE(LabelDecl)
    };

    struct AstUsingDecl final : public AstDecl
    {
        AstUsingDecl(std::string name, Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_USING_DECL, start, end)
        {
        }
        AST_VISITABLE(UsingDecl)
    };

    struct AstTypedef final : public AstTypeDecl
    {
        AstTypeRefPtr typeRef;
        AstTypedef(
            std::string name, AstTypeRefPtr typeRef, Token *start, Token *end)
            : AstTypeDecl(DF_NONE, std::move(name), NK_TYPEDEF_DECL, start, end)
            , typeRef(std::move(typeRef))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (typeRef)
                typeRef->accept(v);
        }
        AST_VISITABLE(TypeDef)
    };

    struct AstNamespaceDecl final : public AstDecl
    {
        SymbolTable scope;
        AstStmtList stmts;
        AstNamespaceDecl(
            std::string name, AstStmtList statements, Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_NAMESPACE_DECL, start, end)
            , stmts(std::move(statements))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            for (auto &stmt : stmts)
                stmt->accept(v);
        }
        AST_VISITABLE(NamespaceDecl)
    };

    struct AstVarDecl final : public AstDecl
    {
        AstTypeRefPtr typeRef;
        AstExprPtr initExpr;
        AstVarDecl(std::string name, AstTypeRefPtr typeRef, AstExprPtr initExpr,
            Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_VAR_DECL, start, end)
            , typeRef(std::move(typeRef))
            , initExpr(std::move(initExpr))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (typeRef)
                typeRef->accept(v);
            if (initExpr)
                initExpr->accept(v);
        }
        AST_VISITABLE(VarDecl)
    };

    struct AstParamDecl final : public AstDecl
    {
        AstTypeRefPtr typeRef;
        AstExprPtr defaultExpr;
        AstParamDecl(std::string name, AstTypeRefPtr typeRef,
            AstExprPtr defaultExpr, Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_PARAM_DECL, start, end)
            , typeRef(std::move(typeRef))
            , defaultExpr(std::move(defaultExpr))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (typeRef)
                typeRef->accept(v);
            if (defaultExpr)
                defaultExpr->accept(v);
        }
        AST_VISITABLE(ParamDecl)
    };

    struct AstFuncDecl final : public AstTypeDecl
    {
        SymbolTable scope;
        AstTypeRefPtr typeRef;
        AstDeclList params;
        AstStmtList stmts;
        AstFuncDecl(std::string name, AstTypeRefPtr typeRef,
            AstDeclList parameters, AstStmtList stmts, Token *start, Token *end)
            : AstTypeDecl(DF_NONE, std::move(name), NK_FUNC_DECL, start, end)
            , typeRef(std::move(typeRef))
            , params(std::move(parameters))
            , stmts(std::move(stmts))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (typeRef)
                typeRef->accept(v);
            for (auto &param : params)
                param->accept(v);
            for (auto &stmt : stmts)
                stmt->accept(v);
        }
        AST_VISITABLE(FuncDecl)
    };

    struct AstDelegateDecl final : public AstTypeDecl
    {
        SymbolTable scope;
        AstTypeRefPtr typeRef;
        AstDeclList params;
        AstDelegateDecl(std::string name, AstTypeRefPtr typeRef,
            AstDeclList parameters, Token *start, Token *end)
            : AstTypeDecl(
                  DF_NONE, std::move(name), NK_DELEGATE_DECL, start, end)
            , typeRef(std::move(typeRef))
            , params(std::move(parameters))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (typeRef)
                typeRef->accept(v);
            for (auto &param : params)
                param->accept(v);
        }
        AST_VISITABLE(DelegateDecl)
    };

    struct AstConstructorDecl final : public AstDecl
    {
        SymbolTable scope;
        AstDeclList params;
        AstStmtList stmts;
        AstConstructorDecl(std::string name, AstDeclList params,
            AstStmtList stmts, Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_CONSTRUCTOR_DECL, start, end)
            , params(std::move(params))
            , stmts(std::move(stmts))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            for (auto &param : params)
                param->accept(v);
            for (auto &stmt : stmts)
                stmt->accept(v);
        }
        AST_VISITABLE(ConstructorDecl)
    };

    struct AstDestructorDecl final : public AstDecl
    {
        SymbolTable scope;
        AstStmtList stmts;
        AstDestructorDecl(
            std::string name, AstStmtList stmts, Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_DESTRUCTOR_DECL, start, end)
            , stmts(std::move(stmts))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            for (auto &stmt : stmts)
                stmt->accept(v);
        }
        AST_VISITABLE(DestructorDecl)
    };

    struct AstStructDecl final : public AstTypeDecl
    {
        SymbolTable scope;
        AstTypeRefList baseTypes;
        AstDeclList members;
        AstStructDecl(std::string name, AstTypeRefList bases,
            AstDeclList members, Token *start, Token *end)
            : AstTypeDecl(DF_NONE, std::move(name), NK_STRUCT_DECL, start, end)
            , baseTypes(std::move(bases))
            , members(std::move(members))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            for (auto &base : baseTypes)
                base->accept(v);
            for (auto &member : members)
                member->accept(v);
        }
        AST_VISITABLE(StructDecl)
    };

    struct AstEnumeratorDecl final : public AstDecl
    {
        AstExprPtr value;
        AstEnumeratorDecl(
            std::string name, AstExprPtr value, Token *start, Token *end)
            : AstDecl(DF_NONE, std::move(name), NK_ENUMERATOR_DECL, start, end)
            , value(std::move(value))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            if (value)
                value->accept(v);
        }
        AST_VISITABLE(EnumeratorDecl)
    };

    typedef NodePtrType< AstEnumeratorDecl > AstEnumeratorDeclPtr;
    typedef std::vector< AstEnumeratorDeclPtr > AstEnumeratorList;

    struct AstEnumDecl final : public AstTypeDecl
    {
        SymbolTable scope;
        AstEnumeratorList enumerators;
        AstDeclList members;
        AstEnumDecl(std::string name, AstEnumeratorList enumerators,
            AstDeclList members, Token *start, Token *end)
            : AstTypeDecl(DF_NONE, std::move(name), NK_ENUM_DECL, start, end)
            , enumerators(std::move(enumerators))
            , members(std::move(members))
        {
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            for (auto &etor : enumerators)
                etor->accept(v);
            for (auto &member : members)
                member->accept(v);
        }
        AST_VISITABLE(EnumDecl)
    };

    struct AstModule final : public AstNode
    {
        std::string fileName;
        AstDeclList members;
        AstModule(Token *start, Token *end)
            : AstNode(NK_MODULE, start, end)
        {
            updateFileName();
        }
        std::string replaceExtension(const std::string &ext) const
        {
            auto lastDot = fileName.rfind('.');
            if (lastDot != fileName.npos)
                return fileName.substr(0, lastDot) + ext;
            return fileName;
        }
        std::string identifierName() const
        {
            std::string n("_soda_");
            std::string noExt = replaceExtension("");
            auto startPos = noExt.rfind('/');
            if (startPos == noExt.npos)
                startPos = noExt.rfind('\\');
            if (startPos != noExt.npos)
                noExt = noExt.substr(startPos + 1);
            for (auto &ch : noExt) {
                if (isAlnum(ch))
                    n += ch;
                else
                    n += '_';
            }
            return n;
        }
        virtual void acceptChildren(AstVisitor &v) override final
        {
            for (auto &member : members)
                member->accept(v);
        }
        AST_VISITABLE(Module)

    private:
        void updateFileName()
        {
            if (start)
                fileName = start->file.getFileName();
            else {
                static unsigned int n = 0;
                fileName = "untitled" + std::to_string(n++) + ".soda";
            }
        }
        static bool isAlnum(int ch)
        {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
                || (ch >= '0' && ch <= '9');
        }
    };

    typedef NodePtrType< AstModule > AstModulePtr;
    typedef std::vector< AstModulePtr > AstModuleList;

} // namespace Soda
