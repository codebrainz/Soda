#pragma once

#include "SymbolTable.h"
#include "Tokenizer.h"
#include "Visitor.h"
#include <memory>
#include <string>
#include <vector>

namespace SODA
{

	enum AST_NODE_KIND
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
		NK_EMPTY_STMT,
		NK_COMMENT_STMT,
		NK_EXPR_STMT,
		NK_BLOCK_STMT,
		NK_EMPTY_DECL,
		NK_COMMENT_DECL,
		NK_USING_DECL,
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

	struct AST_NODE
	{
		AST_NODE_KIND Kind;
		TOKEN *Start;
		TOKEN *End;
		AST_NODE(AST_NODE_KIND kind, TOKEN *start, TOKEN *end)
			: Kind(kind), Start(start), End(end) {}
		~AST_NODE() {}
		virtual void accept(AST_VISITOR&) = 0;
		virtual const std::string &kindName() const = 0;
	};

#define AST_VISITABLE(Name) \
  virtual void accept(AST_VISITOR &v) override final { v.visit(*this); } \
  virtual const std::string &kindName() const override final { \
    static const std::string nm(#Name); \
	return nm; \
  }

	struct AST_EXPR : public AST_NODE
	{
		AST_EXPR(AST_NODE_KIND kind, TOKEN *start, TOKEN *end)
			: AST_NODE(kind, start, end) {}
	};

	struct AST_STMT : public AST_NODE
	{
		AST_STMT(AST_NODE_KIND kind, TOKEN *start, TOKEN *end)
			: AST_NODE(kind, start, end) {}
	};

	enum DECL_FLAGS
	{
		DF_NONE = 0,
		DF_STATIC = 1,
	};

	struct AST_DECL : public AST_STMT
	{
		DECL_FLAGS Flags;
		std::string Name;
		std::string MangledName;
		AST_DECL(DECL_FLAGS flags, std::string name, AST_NODE_KIND kind, TOKEN *start, TOKEN *end)
			: AST_STMT(kind, start, end), Flags(flags), Name(std::move(name)) {}
	};

	typedef std::unique_ptr<AST_NODE> AST_NODE_PTR;
	typedef std::unique_ptr<AST_EXPR> AST_EXPR_PTR;
	typedef std::unique_ptr<AST_STMT> AST_STMT_PTR;
	typedef std::unique_ptr<AST_DECL> AST_DECL_PTR;

	typedef std::vector<AST_NODE_PTR> AST_NODE_LIST;
	typedef std::vector<AST_EXPR_PTR> AST_EXPR_LIST;
	typedef std::vector<AST_STMT_PTR> AST_STMT_LIST;
	typedef std::vector<AST_DECL_PTR> AST_DECL_LIST;

	struct AST_AMBIGUITY final : public AST_NODE
	{
		AST_NODE_LIST Alternatives;
		AST_AMBIGUITY(TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_NODE(NK_AMBIGUITY, start, end) {}
		AST_AMBIGUITY(AST_NODE_LIST Alternatives, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_NODE(NK_AMBIGUITY, start, end), Alternatives(std::move(Alternatives)) {}
		AST_VISITABLE(Ambiguity)
	};

	struct AST_NIL final : public AST_EXPR
	{
		AST_NIL(TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_NIL, start, end) {}
		AST_VISITABLE(Nil)
	};

	struct AST_BOOL final : public AST_EXPR
	{
		bool Value;
		AST_BOOL(bool value, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_BOOL, start, end) {}
		AST_VISITABLE(Bool)
	};

	struct AST_INT final : public AST_EXPR
	{
		unsigned long long int Value;
		AST_INT(unsigned long long int value, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_INT, start, end), Value(value) {}
		AST_VISITABLE(Int)
	};

	struct AST_FLOAT final : public AST_EXPR
	{
		long double Value;
		AST_FLOAT(long double value, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_FLOAT, start, end), Value(value) {}
		AST_VISITABLE(Float)
	};

	struct AST_CHAR final : public AST_EXPR
	{
		std::string Value;
		AST_CHAR(std::string value, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_CHAR, start, end), Value(std::move(value)) {}
		AST_VISITABLE(Char)
	};

	struct AST_STRING final : public AST_EXPR
	{
		std::string Value;
		AST_STRING(std::string value, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_STRING, start, end), Value(std::move(value)) {}
		AST_VISITABLE(String)
	};

	struct AST_IDENTIFIER final : public AST_EXPR
	{
		std::string Name;
		AST_IDENTIFIER(std::string name, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_IDENTIFIER, start, end), Name(name) {}
		AST_VISITABLE(Identifier)
	};

	enum UNARY_OPERATOR
	{
		UOP_POS,
		UOP_NEG,
		UOP_PREINC,
		UOP_PREDEC,
		UOP_POSTINC,
		UOP_POSTDEC,
		UOP_COMPL,
		UOP_NOT,
	};

	struct AST_UNARY final : public AST_EXPR
	{
		UNARY_OPERATOR Op;
		AST_EXPR_PTR Operand;
		AST_UNARY(UNARY_OPERATOR op, AST_EXPR_PTR operand, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_UNARY, start, end), Op(op), Operand(std::move(operand)) {}
		AST_VISITABLE(Unary)
	};

	enum BINARY_OPERATOR
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

	struct AST_BINARY final : public AST_EXPR
	{
		BINARY_OPERATOR Op;
		AST_EXPR_PTR Left;
		AST_EXPR_PTR Right;
		AST_BINARY(BINARY_OPERATOR op, AST_EXPR_PTR left, AST_EXPR_PTR right, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_BINARY, start, end), Op(op), Left(std::move(left)), Right(std::move(right)) {}
		AST_VISITABLE(Binary)
	};

	enum TYPE_FLAGS
	{
		TF_NONE = 0,
		TF_CONST = 1,
		TF_POINTER = 2,
		TF_ARRAY = 4,
	};

	struct AST_TYPEREF final : public AST_NODE
	{
		std::string Name;
		std::unique_ptr<AST_TYPEREF> RefType;
		TYPE_FLAGS TypeFlags;
		AST_TYPEREF(std::string name, TYPE_FLAGS typeFlags = TF_NONE, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_NODE(NK_TYPEREF, start, end), Name(std::move(name)), TypeFlags(typeFlags) {}
		AST_TYPEREF(std::unique_ptr<AST_TYPEREF> refType, TYPE_FLAGS typeFlags = TF_NONE, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_NODE(NK_TYPEREF, start, end), RefType(std::move(refType)), TypeFlags(typeFlags) {}
		AST_VISITABLE(TypeRef)
	};

	typedef std::unique_ptr<AST_TYPEREF> AST_TYPEREF_PTR;
	typedef std::vector<AST_TYPEREF_PTR> AST_TYPEREF_LIST;

	struct AST_CAST final : public AST_EXPR
	{
		AST_TYPEREF_PTR TypeRef;
		AST_EXPR_PTR Expr;
		AST_CAST(AST_TYPEREF_PTR typeRef, AST_EXPR_PTR expr, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_EXPR(NK_CAST, start, end), TypeRef(std::move(typeRef)), Expr(std::move(expr)) {}
		AST_VISITABLE(Cast)
	};

	struct AST_EMPTY_STMT final : public AST_STMT
	{
		AST_EMPTY_STMT(TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STMT(NK_EMPTY_STMT, start, end) {}
		AST_VISITABLE(EmptyStmt)
	};

	struct AST_COMMENT_STMT final : public AST_STMT
	{
		std::string Text;
		AST_COMMENT_STMT(std::string text, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STMT(NK_COMMENT_STMT, start, end), Text(std::move(text)) {}
		AST_VISITABLE(CommentStmt)
	};

	struct AST_EXPR_STMT final : public AST_STMT
	{
		AST_EXPR_PTR Expr;
		AST_EXPR_STMT(AST_EXPR_PTR expr, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STMT(NK_EXPR_STMT, start, end), Expr(std::move(expr)) {}
		AST_VISITABLE(ExprStmt)
	};

	struct AST_BLOCK_STMT final : public AST_STMT
	{
		SYMBOL_TABLE Scope;
		AST_STMT_LIST Statements;
		AST_BLOCK_STMT(TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STMT(NK_BLOCK_STMT, start, end) {}
		AST_BLOCK_STMT(AST_STMT_LIST stmts, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STMT(NK_BLOCK_STMT, start, end), Statements(std::move(stmts)) {}
		AST_VISITABLE(BlockStmt)
	};

	struct AST_EMPTY_DECL final : public AST_DECL
	{
		AST_EMPTY_DECL(TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, "", NK_EMPTY_DECL, start, end) {}
		AST_VISITABLE(EmptyDecl)
	};

	struct AST_COMMENT_DECL final : public AST_DECL
	{
		std::string Text;
		AST_COMMENT_DECL(std::string text, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, "", NK_COMMENT_DECL, start, end), Text(std::move(text)) {}
		AST_VISITABLE(Comment)
	};

	struct AST_USING_DECL final : public AST_DECL
	{
		AST_USING_DECL(std::string name, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_USING_DECL, start, end) {}
		AST_VISITABLE(UsingDecl)
	};

	struct AST_TYPEDEF final : public AST_DECL
	{
		AST_TYPEREF_PTR TypeRef;
		AST_TYPEDEF(std::string name, AST_TYPEREF_PTR typeRef, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_TYPEDEF_DECL, start, end), TypeRef(std::move(typeRef)) {}
		AST_VISITABLE(TypeDef)
	};

	struct AST_NAMESPACE_DECL final : public AST_DECL
	{
		AST_STMT_LIST Statements;
		AST_NAMESPACE_DECL(std::string name, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_NAMESPACE_DECL, start, end) {}
		AST_NAMESPACE_DECL(std::string name, AST_STMT_LIST statements, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_NAMESPACE_DECL, start, end), Statements(std::move(statements)) {}
		AST_VISITABLE(NamespaceDecl)
	};

	struct AST_VAR_DECL final : public AST_DECL
	{
		AST_TYPEREF_PTR TypeRef;
		AST_EXPR_PTR InitExpr;
		AST_VAR_DECL(std::string name, AST_TYPEREF_PTR typeRef, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_VAR_DECL, start, end), TypeRef(std::move(typeRef)) {}
		AST_VAR_DECL(std::string name, AST_TYPEREF_PTR typeRef, AST_EXPR_PTR initExpr, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_VAR_DECL, start, end), TypeRef(std::move(typeRef)), InitExpr(std::move(initExpr)) {}
		AST_VISITABLE(VarDecl)
	};

	struct AST_PARAM_DECL final : public AST_DECL
	{
		AST_TYPEREF_PTR TypeRef;
		AST_EXPR_PTR DefaultExpr;
		AST_PARAM_DECL(std::string name, AST_TYPEREF_PTR typeRef, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_PARAM_DECL, start, end), TypeRef(std::move(typeRef)) {}
		AST_PARAM_DECL(std::string name, AST_TYPEREF_PTR typeRef, AST_EXPR_PTR defaultExpr, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_PARAM_DECL, start, end), TypeRef(std::move(typeRef)), DefaultExpr(std::move(defaultExpr)) {}
		AST_VISITABLE(ParamDecl)
	};

	struct AST_FUNC_DECL final : public AST_DECL
	{
		SYMBOL_TABLE Scope;
		AST_TYPEREF_PTR TypeRef;
		AST_DECL_LIST Parameters;
		AST_STMT_LIST Statements;
		AST_FUNC_DECL(std::string name, AST_TYPEREF_PTR typeRef, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_FUNC_DECL(std::move(name), std::move(typeRef), AST_DECL_LIST(), AST_STMT_LIST(), start, end) {}
		AST_FUNC_DECL(std::string name, AST_TYPEREF_PTR typeRef, AST_DECL_LIST parameters, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_FUNC_DECL(std::move(name), std::move(typeRef), std::move(parameters), AST_STMT_LIST(), start, end) {}
		AST_FUNC_DECL(std::string name, AST_TYPEREF_PTR typeRef, AST_DECL_LIST parameters, AST_STMT_LIST stmts, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_FUNC_DECL, start, end), TypeRef(std::move(typeRef)), Parameters(std::move(parameters)), Statements(std::move(stmts)) {}
		AST_VISITABLE(FuncDecl)
	};

	struct AST_DELEGATE_DECL final : public AST_DECL
	{
		SYMBOL_TABLE Scope;
		AST_TYPEREF_PTR TypeRef;
		AST_DECL_LIST Parameters;
		AST_DELEGATE_DECL(std::string name, AST_TYPEREF_PTR typeRef, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_DELEGATE_DECL, start, end), TypeRef(std::move(typeRef)) {}
		AST_DELEGATE_DECL(std::string name, AST_TYPEREF_PTR typeRef, AST_DECL_LIST parameters, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_DELEGATE_DECL, start, end), TypeRef(std::move(typeRef)), Parameters(std::move(parameters)) {}
		AST_VISITABLE(DelegateDecl)
	};

	struct AST_STRUCT_DECL final : public AST_DECL
	{
		SYMBOL_TABLE Scope;
		AST_TYPEREF_LIST BaseTypes;
		AST_DECL_LIST Members;
		AST_STRUCT_DECL(std::string name, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STRUCT_DECL(std::move(name), AST_TYPEREF_LIST(), AST_DECL_LIST(), start, end) {}
		AST_STRUCT_DECL(std::string name, AST_TYPEREF_LIST bases, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STRUCT_DECL(std::move(name), std::move(bases), AST_DECL_LIST(), start, end) {}
		AST_STRUCT_DECL(std::string name, AST_DECL_LIST members, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_STRUCT_DECL(std::move(name), AST_TYPEREF_LIST(), std::move(members), start, end) {}
		AST_STRUCT_DECL(std::string name, AST_TYPEREF_LIST bases, AST_DECL_LIST members, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_STRUCT_DECL, start, end), BaseTypes(std::move(bases)), Members(std::move(members)) {}
		AST_VISITABLE(StructDecl)
	};

	struct AST_ENUMERATOR_DECL final : public AST_DECL
	{
		AST_EXPR_PTR Value;
		AST_ENUMERATOR_DECL(std::string name, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_ENUMERATOR_DECL, start, end) {}
		AST_ENUMERATOR_DECL(std::string name, AST_EXPR_PTR value, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_ENUMERATOR_DECL, start, end), Value(std::move(value)) {}
		AST_VISITABLE(EnumeratorDecl)
	};

	typedef std::unique_ptr<AST_ENUMERATOR_DECL> AST_ENUMERATOR_DECL_PTR;
	typedef std::vector<AST_ENUMERATOR_DECL_PTR> AST_ENUMERATOR_LIST;

	struct AST_ENUM_DECL final : public AST_DECL
	{
		SYMBOL_TABLE Scope;
		AST_ENUMERATOR_LIST Enumerators;
		AST_DECL_LIST Members;
		AST_ENUM_DECL(std::string name, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_ENUM_DECL(std::move(name), AST_ENUMERATOR_LIST(), AST_DECL_LIST(), start, end) {}
		AST_ENUM_DECL(std::string name, AST_ENUMERATOR_LIST enumerators, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_ENUM_DECL(std::move(name), std::move(enumerators), AST_DECL_LIST(), start, end) {}
		AST_ENUM_DECL(std::string name, AST_DECL_LIST members, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_ENUM_DECL(std::move(name), AST_ENUMERATOR_LIST(), std::move(members), start, end) {}
		AST_ENUM_DECL(std::string name, AST_ENUMERATOR_LIST enumerators, AST_DECL_LIST members, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_DECL(DF_NONE, std::move(name), NK_ENUM_DECL, start, end), Enumerators(std::move(enumerators)), Members(std::move(members)) {}
		AST_VISITABLE(EnumDecl)
	};

	struct AST_MODULE final : public AST_NODE
	{
		AST_DECL_LIST Members;
		AST_MODULE(TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_NODE(NK_MODULE, start, end) {}
		AST_MODULE(AST_DECL_LIST members, TOKEN *start = nullptr, TOKEN *end = nullptr)
			: AST_NODE(NK_MODULE, start, end), Members(std::move(members)) {}
		AST_VISITABLE(Module)
	};

} // namespace SODA