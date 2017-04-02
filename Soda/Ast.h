#pragma once

#include "SymbolTable.h"
#include "SourceFile.h"
#include "Tokenizer.h"
#include "Visitor.h"
#include <memory>
#include <string>
#include <vector>

namespace Soda
{

	enum AstNodeKind
	{
		NK_AMBIGUITY,
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
		NK_COMMENT_STMT,
		NK_EXPR_STMT,
		NK_BLOCK_STMT,
		NK_RETURN_STMT,
		NK_BREAK_STMT,
		NK_CONTINUE_STMT,
		NK_GOTO_STMT,
		NK_IF_STMT,
		NK_WHILE_STMT,
		NK_EMPTY_DECL,
		NK_COMMENT_DECL,
		NK_USING_DECL,
		NK_TYPENAME,
		NK_TYPEDEF_DECL,
		NK_NAMESPACE_DECL,
		NK_VAR_DECL,
		NK_PARAM_DECL,
		NK_FUNC_DECL, 
		NK_DELEGATE_DECL,
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
			: kind(kind), start(start), end(end), parentNode(nullptr), ownerScope(nullptr) {}
		~AstNode() {}
		virtual void accept(AstVisitor&) = 0;
		virtual void acceptChildren(AstVisitor&) {}
		virtual const std::string &kindName() const = 0;
	};

#define AST_VISITABLE(name) \
  virtual void accept(AstVisitor &v) override final { v.visit(*this); } \
  virtual const std::string &kindName() const override final { \
    static const std::string nm(#name); \
	return nm; \
  }

	struct AstExpr : public AstNode
	{
		AstExpr(AstNodeKind kind, Token *start, Token *end)
			: AstNode(kind, start, end) {}
	};

	struct AstStmt : public AstNode
	{
		AstStmt(AstNodeKind kind, Token *start, Token *end)
			: AstNode(kind, start, end) {}
	};

	enum DeclFlags
	{
		DF_NONE = 0,
		DF_STATIC = 1,
	};

	struct AstDecl : public AstStmt
	{
		DeclFlags flags;
		std::string name;
		std::string mangledName;
		AstDecl(DeclFlags flags, std::string name, AstNodeKind kind, Token *start, Token *end)
			: AstStmt(kind, start, end), flags(flags), name(std::move(name)) {}
	};

	typedef std::unique_ptr<AstNode> AstNodePtr;
	typedef std::unique_ptr<AstExpr> AstExprPtr;
	typedef std::unique_ptr<AstStmt> AstStmtPtr;
	typedef std::unique_ptr<AstDecl> AstDeclPtr;

	typedef std::vector<AstNodePtr> AstNodeList;
	typedef std::vector<AstExprPtr> AstExprList;
	typedef std::vector<AstStmtPtr> AstStmtList;
	typedef std::vector<AstDeclPtr> AstDeclList;

	struct AstAmbiguityStmt final : public AstStmt
	{
		AstNodeList alternatives;
		AstAmbiguityStmt(Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_AMBIGUITY, start, end) {}
		AstAmbiguityStmt(AstNodeList alternatives, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_AMBIGUITY, start, end), alternatives(std::move(alternatives)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			for (auto &alt : alternatives)
				alt->accept(v);
		}
		AST_VISITABLE(AmbiguityStmt)
	};

	struct AstNil final : public AstExpr
	{
		AstNil(Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_NIL, start, end) {}
		AST_VISITABLE(Nil)
	};

	struct AstBool final : public AstExpr
	{
		bool value;
		AstBool(bool value, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_BOOL, start, end), value(value) {}
		AST_VISITABLE(Bool)
	};

	struct AstInt final : public AstExpr
	{
		unsigned long long int value;
		AstInt(unsigned long long int value, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_INT, start, end), value(value) {}
		AST_VISITABLE(Int)
	};

	struct AstFloat final : public AstExpr
	{
		long double value;
		AstFloat(long double value, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_FLOAT, start, end), value(value) {}
		AST_VISITABLE(Float)
	};

	struct AstChar final : public AstExpr
	{
		std::string value;
		AstChar(std::string value, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_CHAR, start, end), value(std::move(value)) {}
		AST_VISITABLE(Char)
	};

	struct AstString final : public AstExpr
	{
		std::string value;
		AstString(std::string value, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_STRING, start, end), value(std::move(value)) {}
		AST_VISITABLE(String)
	};

	struct AstIdentifier final : public AstExpr
	{
		std::string name;
		Symbol *refSymbol;
		AstIdentifier(std::string name, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_IDENTIFIER, start, end), name(name), refSymbol(nullptr) {}
		std::string mangledName() const
		{
			if (refSymbol)
				return refSymbol->decl->mangledName;
			return name;
		}
		AST_VISITABLE(Identifier)
	};

	enum UnaryOperator
	{
		UOP_POS,
		UOP_NEG,
		UOP_PREINC,
		UOP_PREDEC,
		UOP_POSTINC,
		UOP_POSTDEC,
		UOP_COMPL,
		UOP_NOT,
		UOP_DEREF,
	};

	struct AstUnary final : public AstExpr
	{
		UnaryOperator op;
		AstExprPtr operand;
		AstUnary(UnaryOperator op, AstExprPtr operand, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_UNARY, start, end), op(op), operand(std::move(operand)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			operand->accept(v);
		}
		AST_VISITABLE(Unary)
	};

	enum BinaryOperator
	{
		BOP_ADD,
		BOP_SUB,
		BOP_MUL,
		BOP_DIV,
		BOP_MOD,

		BOP_LOG_AND,
		BOP_LOG_OR,

		BOP_BIT_AND,
		BOP_BIT_OR,
		BOP_BIT_XOR,

		BOP_LSHIFT,
		BOP_RSHIFT,

		BOP_EQ,
		BOP_NE,
		BOP_LT,
		BOP_GT,
		BOP_LE,
		BOP_GE,

		BOP_ASSIGN,
		BOP_IADD,
		BOP_ISUB,
		BOP_IMUL,
		BOP_IDIV,
		BOP_IMOD,
		BOP_IAND,
		BOP_IOR,
		BOP_IXOR,
		BOP_ILSHIFT,
		BOP_IRSHIFT,
	};

	struct AstBinary final : public AstExpr
	{
		BinaryOperator op;
		AstExprPtr left;
		AstExprPtr right;
		AstBinary(BinaryOperator op, AstExprPtr left, AstExprPtr right, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_BINARY, start, end), op(op), left(std::move(left)), right(std::move(right)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
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
		std::unique_ptr<AstTypeRef> refType;
		TypeFlags typeFlags;
		AstTypeRef(std::string name, TypeFlags typeFlags = TF_NONE, Token *start = nullptr, Token *end = nullptr)
			: AstNode(NK_TYPEREF, start, end), name(std::move(name)), typeFlags(typeFlags) {}
		AstTypeRef(std::unique_ptr<AstTypeRef> refType, TypeFlags typeFlags = TF_NONE, Token *start = nullptr, Token *end = nullptr)
			: AstNode(NK_TYPEREF, start, end), refType(std::move(refType)), typeFlags(typeFlags) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			if (refType)
				refType->accept(v);
		}
		AST_VISITABLE(TypeRef)
	};

	typedef std::unique_ptr<AstTypeRef> AstTypeRefPtr;
	typedef std::vector<AstTypeRefPtr> AstTypeRefList;

	struct AstCast final : public AstExpr
	{
		AstTypeRefPtr typeRef;
		AstExprPtr expr;
		AstCast(AstTypeRefPtr typeRef, AstExprPtr expr, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_CAST, start, end), typeRef(std::move(typeRef)), expr(std::move(expr)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
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
		AstIfExpr(AstExprPtr condExpr, AstExprPtr thenExpr, AstExprPtr elseExpr, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_IF_EXPR, start, end), condExpr(std::move(condExpr)), thenExpr(std::move(thenExpr)), elseExpr(std::move(elseExpr)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
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
		AstCallExpr(AstExprPtr callee, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_CALL_EXPR, start, end), callee(std::move(callee)) {}
		AstCallExpr(AstExprPtr callee, AstExprList args, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_CALL_EXPR, start, end), callee(std::move(callee)), arguments(std::move(args)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
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
		AstIndexExpr(AstExprPtr object, AstExprPtr index, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_INDEX_EXPR, start, end), object(std::move(object)), index(std::move(index)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			object->accept(v);
			index->accept(v);
		}
		AST_VISITABLE(IndexExpr)
	};

	struct AstMemberExpr final : public AstExpr
	{
		AstExprPtr object;
		std::string member;
		AstMemberExpr(AstExprPtr object, std::string member, Token *start = nullptr, Token *end = nullptr)
			: AstExpr(NK_MEMBER_EXPR, start, end), object(std::move(object)), member(std::move(member)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			object->accept(v);
		}
		AST_VISITABLE(MemberExpr)
	};

	struct AstEmptyStmt final : public AstStmt
	{
		AstEmptyStmt(Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_EMPTY_STMT, start, end) {}
		AST_VISITABLE(EmptyStmt)
	};

	struct AstCommentStmt final : public AstStmt
	{
		std::string text;
		AstCommentStmt(std::string text, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_COMMENT_STMT, start, end), text(std::move(text)) {}
		AST_VISITABLE(CommentStmt)
	};

	struct AstExprStmt final : public AstStmt
	{
		AstExprPtr expr;
		AstExprStmt(AstExprPtr expr, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_EXPR_STMT, start, end), expr(std::move(expr)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			expr->accept(v);
		}
		AST_VISITABLE(ExprStmt)
	};

	struct AstBlockStmt final : public AstStmt
	{
		SymbolTable scope;
		AstStmtList stmts;
		AstBlockStmt(Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_BLOCK_STMT, start, end) {}
		AstBlockStmt(AstStmtList stmts, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_BLOCK_STMT, start, end), stmts(std::move(stmts)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			for (auto &stmt : stmts)
				stmt->accept(v);
		}
		AST_VISITABLE(BlockStmt)
	};

	struct AstReturnStmt final : public AstStmt
	{
		AstExprPtr expr;
		AstReturnStmt(Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_RETURN_STMT, start, end) {}
		AstReturnStmt(AstExprPtr expr, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_RETURN_STMT, start, end), expr(std::move(expr)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			if (expr)
				expr->accept(v);
		}
		AST_VISITABLE(ReturnStmt)
	};

	struct AstBreakStmt final : public AstStmt
	{
		AstBreakStmt(Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_BREAK_STMT, start, end) {}
		AST_VISITABLE(BreakStmt)
	};

	struct AstContinueStmt final : public AstStmt
	{
		AstContinueStmt(Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_CONTINUE_STMT, start, end) {}
		AST_VISITABLE(ContinueStmt)
	};

	struct AstGotoStmt final : public AstStmt
	{
		std::string label;
		AstGotoStmt(std::string label, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_GOTO_STMT, start, end), label(std::move(label)) {}
		AST_VISITABLE(GotoStmt)
	};

	struct AstIfStmt final : public AstStmt
	{
		AstExprPtr condExpr;
		AstStmtPtr thenStmt;
		AstStmtPtr elseStmt;
		AstIfStmt(AstExprPtr condExpr, AstStmtPtr thenStmt, AstStmtPtr elseStmt, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_IF_STMT, start, end),
			  condExpr(std::move(condExpr)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			condExpr->accept(v);
			thenStmt->accept(v);
			if (elseStmt)
				elseStmt->accept(v);
		}
		AST_VISITABLE(IfStmt)
	};

	struct AstWhileStmt final : public AstStmt
	{
		AstExprPtr expr;
		AstStmtPtr stmt;
		AstWhileStmt(AstExprPtr expr, AstStmtPtr stmt, Token *start = nullptr, Token *end = nullptr)
			: AstStmt(NK_WHILE_STMT, start, end), expr(std::move(expr)), stmt(std::move(stmt)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			expr->accept(v);
			stmt->accept(v);
		}
		AST_VISITABLE(WhileStmt)
	};

	struct AstEmptyDecl final : public AstDecl
	{
		AstEmptyDecl(Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, "", NK_EMPTY_DECL, start, end) {}
		AST_VISITABLE(EmptyDecl)
	};

	struct AstCommentDecl final : public AstDecl
	{
		std::string text;
		AstCommentDecl(std::string text, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, "", NK_COMMENT_DECL, start, end), text(std::move(text)) {}
		AST_VISITABLE(Comment)
	};

	struct AstUsingDecl final : public AstDecl
	{
		AstUsingDecl(std::string name, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_USING_DECL, start, end) {}
		AST_VISITABLE(UsingDecl)
	};

	struct AstTypedef final : public AstDecl
	{
		AstTypeRefPtr typeRef;
		AstTypedef(std::string name, AstTypeRefPtr typeRef, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_TYPEDEF_DECL, start, end), typeRef(std::move(typeRef)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			if (typeRef)
				typeRef->accept(v);
		}
		AST_VISITABLE(TypeDef)
	};

	struct AstNamespaceDecl final : public AstDecl
	{
		SymbolTable scope;
		AstStmtList stmts;
		AstNamespaceDecl(std::string name, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_NAMESPACE_DECL, start, end) {}
		AstNamespaceDecl(std::string name, AstStmtList statements, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_NAMESPACE_DECL, start, end), stmts(std::move(statements)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			for (auto &stmt : stmts)
				stmt->accept(v);
		}
		AST_VISITABLE(NamespaceDecl)
	};

	struct AstVarDecl final : public AstDecl
	{
		AstTypeRefPtr typeRef;
		AstExprPtr initExpr;
		AstVarDecl(std::string name, AstTypeRefPtr typeRef, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_VAR_DECL, start, end), typeRef(std::move(typeRef)) {}
		AstVarDecl(std::string name, AstTypeRefPtr typeRef, AstExprPtr initExpr, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_VAR_DECL, start, end), typeRef(std::move(typeRef)), initExpr(std::move(initExpr)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
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
		AstParamDecl(std::string name, AstTypeRefPtr typeRef, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_PARAM_DECL, start, end), typeRef(std::move(typeRef)) {}
		AstParamDecl(std::string name, AstTypeRefPtr typeRef, AstExprPtr defaultExpr, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_PARAM_DECL, start, end), typeRef(std::move(typeRef)), defaultExpr(std::move(defaultExpr)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			if (typeRef)
				typeRef->accept(v);
			if (defaultExpr)
				defaultExpr->accept(v);
		}
		AST_VISITABLE(ParamDecl)
	};

	struct AstFuncDecl final : public AstDecl
	{
		SymbolTable scope;
		AstTypeRefPtr typeRef;
		AstDeclList params;
		AstStmtList stmts;
		AstFuncDecl(std::string name, AstTypeRefPtr typeRef, Token *start = nullptr, Token *end = nullptr)
			: AstFuncDecl(std::move(name), std::move(typeRef), AstDeclList(), AstStmtList(), start, end) {}
		AstFuncDecl(std::string name, AstTypeRefPtr typeRef, AstDeclList parameters, Token *start = nullptr, Token *end = nullptr)
			: AstFuncDecl(std::move(name), std::move(typeRef), std::move(parameters), AstStmtList(), start, end) {}
		AstFuncDecl(std::string name, AstTypeRefPtr typeRef, AstDeclList parameters, AstStmtList stmts, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_FUNC_DECL, start, end), typeRef(std::move(typeRef)), params(std::move(parameters)), stmts(std::move(stmts)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			if (typeRef)
				typeRef->accept(v);
			for (auto &param : params)
				param->accept(v);
			for (auto &stmt : stmts)
				stmt->accept(v);
		}
		AST_VISITABLE(FuncDecl)
	};

	struct AstDelegateDecl final : public AstDecl
	{
		SymbolTable scope;
		AstTypeRefPtr typeRef;
		AstDeclList params;
		AstDelegateDecl(std::string name, AstTypeRefPtr typeRef, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_DELEGATE_DECL, start, end), typeRef(std::move(typeRef)) {}
		AstDelegateDecl(std::string name, AstTypeRefPtr typeRef, AstDeclList parameters, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_DELEGATE_DECL, start, end), typeRef(std::move(typeRef)), params(std::move(parameters)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			if (typeRef)
				typeRef->accept(v);
			for (auto &param : params)
				param->accept(v);
		}
		AST_VISITABLE(DelegateDecl)
	};

	struct AstStructDecl final : public AstDecl
	{
		SymbolTable scope;
		AstTypeRefList baseTypes;
		AstDeclList members;
		AstStructDecl(std::string name, Token *start = nullptr, Token *end = nullptr)
			: AstStructDecl(std::move(name), AstTypeRefList(), AstDeclList(), start, end) {}
		AstStructDecl(std::string name, AstTypeRefList bases, Token *start = nullptr, Token *end = nullptr)
			: AstStructDecl(std::move(name), std::move(bases), AstDeclList(), start, end) {}
		AstStructDecl(std::string name, AstDeclList members, Token *start = nullptr, Token *end = nullptr)
			: AstStructDecl(std::move(name), AstTypeRefList(), std::move(members), start, end) {}
		AstStructDecl(std::string name, AstTypeRefList bases, AstDeclList members, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_STRUCT_DECL, start, end), baseTypes(std::move(bases)), members(std::move(members)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
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
		AstEnumeratorDecl(std::string name, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_ENUMERATOR_DECL, start, end) {}
		AstEnumeratorDecl(std::string name, AstExprPtr value, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_ENUMERATOR_DECL, start, end), value(std::move(value)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
			if (value)
				value->accept(v);
		}
		AST_VISITABLE(EnumeratorDecl)
	};

	typedef std::unique_ptr<AstEnumeratorDecl> AstEnumeratorDeclPtr;
	typedef std::vector<AstEnumeratorDeclPtr> AstEnumeratorList;

	struct AstEnumDecl final : public AstDecl
	{
		SymbolTable scope;
		AstEnumeratorList enumerators;
		AstDeclList members;
		AstEnumDecl(std::string name, Token *start = nullptr, Token *end = nullptr)
			: AstEnumDecl(std::move(name), AstEnumeratorList(), AstDeclList(), start, end) {}
		AstEnumDecl(std::string name, AstEnumeratorList enumerators, Token *start = nullptr, Token *end = nullptr)
			: AstEnumDecl(std::move(name), std::move(enumerators), AstDeclList(), start, end) {}
		AstEnumDecl(std::string name, AstDeclList members, Token *start = nullptr, Token *end = nullptr)
			: AstEnumDecl(std::move(name), AstEnumeratorList(), std::move(members), start, end) {}
		AstEnumDecl(std::string name, AstEnumeratorList enumerators, AstDeclList members, Token *start = nullptr, Token *end = nullptr)
			: AstDecl(DF_NONE, std::move(name), NK_ENUM_DECL, start, end), enumerators(std::move(enumerators)), members(std::move(members)) {}
		virtual void acceptChildren(AstVisitor &v) override final {
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
		AstModule(Token *start = nullptr, Token *end = nullptr)
			: AstNode(NK_MODULE, start, end) 
		{
			updateFileName(); 
		}
		AstModule(AstDeclList members, Token *start = nullptr, Token *end = nullptr)
			: AstNode(NK_MODULE, start, end), members(std::move(members)) 
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
			for (auto &ch : noExt)
			{
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
			else 
			{
				static unsigned int n = 0;
				fileName = "untitled" + std::to_string(n++) + ".soda";
			}
		}
		static bool isAlnum(int ch)
		{
			return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
		}
	};

	typedef std::unique_ptr<AstModule> AstModulePtr;
	typedef std::vector<AstModulePtr> AstModuleList;

} // namespace Soda
