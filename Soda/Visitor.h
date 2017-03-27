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
		virtual void visit(AstAmbiguity&) = 0;
		virtual void visit(AstNil&) = 0;
		virtual void visit(AstBool&) = 0;
		virtual void visit(AstInt&) = 0;
		virtual void visit(AstFloat&) = 0;
		virtual void visit(AstChar&) = 0;
		virtual void visit(AstString&) = 0;
		virtual void visit(AstIdentifier&) = 0;
		virtual void visit(AstUnary&) = 0;
		virtual void visit(AstBinary&) = 0;
		virtual void visit(AstTypeRef&) = 0;
		virtual void visit(AstCast&) = 0;
		virtual void visit(AstEmptyStmt&) = 0;
		virtual void visit(AstCommentStmt&) = 0;
		virtual void visit(AstExprStmt&) = 0;
		virtual void visit(AstBlockStmt&) = 0;
		virtual void visit(AstEmptyDecl&) = 0;
		virtual void visit(AstCommentDecl&) = 0;
		virtual void visit(AstUsingDecl&) = 0;
		virtual void visit(AstTypedef&) = 0;
		virtual void visit(AstNamespaceDecl&) = 0;
		virtual void visit(AstVarDecl&) = 0;
		virtual void visit(AstParamDecl&) = 0;
		virtual void visit(AstFuncDecl&) = 0;
		virtual void visit(AstDelegateDecl&) = 0;
		virtual void visit(AstStructDecl&) = 0;
		virtual void visit(AstEnumeratorDecl&) = 0;
		virtual void visit(AstEnumDecl&) = 0;
		virtual void visit(AstModule&) = 0;
	};

	class AstDefaultVisitor : public AstVisitor
	{
	public:
		virtual void visit(AstAmbiguity &n) override { n.acceptChildren(*this); }
		virtual void visit(AstNil &n) override { n.acceptChildren(*this); }
		virtual void visit(AstBool &n) override { n.acceptChildren(*this); }
		virtual void visit(AstInt &n) override { n.acceptChildren(*this); }
		virtual void visit(AstFloat &n) override { n.acceptChildren(*this); }
		virtual void visit(AstChar &n) override { n.acceptChildren(*this); }
		virtual void visit(AstString &n) override { n.acceptChildren(*this); }
		virtual void visit(AstIdentifier &n) override { n.acceptChildren(*this); }
		virtual void visit(AstUnary &n) override { n.acceptChildren(*this); }
		virtual void visit(AstBinary &n) override { n.acceptChildren(*this); }
		virtual void visit(AstTypeRef &n) override { n.acceptChildren(*this); }
		virtual void visit(AstCast &n) override { n.acceptChildren(*this); }
		virtual void visit(AstEmptyStmt &n) override { n.acceptChildren(*this); }
		virtual void visit(AstCommentStmt &n) override { n.acceptChildren(*this); }
		virtual void visit(AstExprStmt &n) override { n.acceptChildren(*this); }
		virtual void visit(AstBlockStmt &n) override { n.acceptChildren(*this); }
		virtual void visit(AstEmptyDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstCommentDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstUsingDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstTypedef &n) override { n.acceptChildren(*this); }
		virtual void visit(AstNamespaceDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstVarDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstParamDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstFuncDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstDelegateDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstStructDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstEnumeratorDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstEnumDecl &n) override { n.acceptChildren(*this); }
		virtual void visit(AstModule &n) override { n.acceptChildren(*this); }
	};

} // namespace Soda