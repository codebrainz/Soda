#pragma once

namespace Soda
{

	struct AstAmbiguity;
	struct AstNil;
	struct AstBool;
	struct AstInt;
	struct AstFloat;
	struct AstChar;
	struct AstString;
	struct AstIdentifier;
	struct AstUnary;
	struct AstBinary;
	struct AstTypeRef;
	struct AstCast;
	struct AstEmptyStmt;
	struct AstCommentStmt;
	struct AstExprStmt;
	struct AstBlockStmt;
	struct AstEmptyDecl;
	struct AstCommentDecl;
	struct AstUsingDecl;
	struct AstTypedef;
	struct AstNamespaceDecl;
	struct AstVarDecl;
	struct AstParamDecl;
	struct AstFuncDecl;
	struct AstDelegateDecl;
	struct AstStructDecl;
	struct AstEnumeratorDecl;
	struct AstEnumDecl;
	struct AstModule;

	class AstVisitor
	{
	public:
		virtual void visit(AstAmbiguity&) {}
		virtual void visit(AstNil&) {}
		virtual void visit(AstBool&) {}
		virtual void visit(AstInt&) {}
		virtual void visit(AstFloat&) {}
		virtual void visit(AstChar&) {}
		virtual void visit(AstString&) {}
		virtual void visit(AstIdentifier&) {}
		virtual void visit(AstUnary&) {}
		virtual void visit(AstBinary&) {}
		virtual void visit(AstTypeRef&) {}
		virtual void visit(AstCast&) {}
		virtual void visit(AstEmptyStmt&) {}
		virtual void visit(AstCommentStmt&) {}
		virtual void visit(AstExprStmt&) {}
		virtual void visit(AstBlockStmt&) {}
		virtual void visit(AstEmptyDecl&) {}
		virtual void visit(AstCommentDecl&) {}
		virtual void visit(AstUsingDecl&) {}
		virtual void visit(AstTypedef&) {}
		virtual void visit(AstNamespaceDecl&) {}
		virtual void visit(AstVarDecl&) {}
		virtual void visit(AstParamDecl&) {}
		virtual void visit(AstFuncDecl&) {}
		virtual void visit(AstDelegateDecl&) {}
		virtual void visit(AstStructDecl&) {}
		virtual void visit(AstEnumeratorDecl&) {}
		virtual void visit(AstEnumDecl&) {}
		virtual void visit(AstModule&) {}
	};

} // namespace Soda