#pragma once

namespace SODA
{

	struct AST_AMBIGUITY;
	struct AST_NIL;
	struct AST_BOOL;
	struct AST_INT;
	struct AST_FLOAT;
	struct AST_CHAR;
	struct AST_STRING;
	struct AST_IDENTIFIER;
	struct AST_UNARY;
	struct AST_BINARY;
	struct AST_TYPEREF;
	struct AST_CAST;
	struct AST_EMPTY_STMT;
	struct AST_COMMENT_STMT;
	struct AST_EXPR_STMT;
	struct AST_BLOCK_STMT;
	struct AST_EMPTY_DECL;
	struct AST_COMMENT_DECL;
	struct AST_USING_DECL;
	struct AST_TYPEDEF;
	struct AST_NAMESPACE_DECL;
	struct AST_VAR_DECL;
	struct AST_PARAM_DECL;
	struct AST_FUNC_DECL;
	struct AST_DELEGATE_DECL;
	struct AST_STRUCT_DECL;
	struct AST_ENUMERATOR_DECL;
	struct AST_ENUM_DECL;
	struct AST_MODULE;

	class AST_VISITOR
	{
	public:
		virtual void visit(AST_AMBIGUITY&) {}
		virtual void visit(AST_NIL&) {}
		virtual void visit(AST_BOOL&) {}
		virtual void visit(AST_INT&) {}
		virtual void visit(AST_FLOAT&) {}
		virtual void visit(AST_CHAR&) {}
		virtual void visit(AST_STRING&) {}
		virtual void visit(AST_IDENTIFIER&) {}
		virtual void visit(AST_UNARY&) {}
		virtual void visit(AST_BINARY&) {}
		virtual void visit(AST_TYPEREF&) {}
		virtual void visit(AST_CAST&) {}
		virtual void visit(AST_EMPTY_STMT&) {}
		virtual void visit(AST_COMMENT_STMT&) {}
		virtual void visit(AST_EXPR_STMT&) {}
		virtual void visit(AST_BLOCK_STMT&) {}
		virtual void visit(AST_EMPTY_DECL&) {}
		virtual void visit(AST_COMMENT_DECL&) {}
		virtual void visit(AST_USING_DECL&) {}
		virtual void visit(AST_TYPEDEF&) {}
		virtual void visit(AST_NAMESPACE_DECL&) {}
		virtual void visit(AST_VAR_DECL&) {}
		virtual void visit(AST_PARAM_DECL&) {}
		virtual void visit(AST_FUNC_DECL&) {}
		virtual void visit(AST_DELEGATE_DECL&) {}
		virtual void visit(AST_STRUCT_DECL&) {}
		virtual void visit(AST_ENUMERATOR_DECL&) {}
		virtual void visit(AST_ENUM_DECL&) {}
		virtual void visit(AST_MODULE&) {}
	};

} // namespace SODA