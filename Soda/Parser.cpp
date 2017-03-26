#include "Parser.h"
#include "Ast.h"
#include "Compiler.h"
#include "SymbolTable.h"
#include <cassert>
#include <stack>
#include <vector>

namespace SODA
{

	struct PARSER
	{
		COMPILER &Compiler;
		TOKEN_LIST &Tokens;
		size_t Offset;
		std::stack<size_t> OffsetStack;
		std::stack<TOKEN*> TokenStack;

		PARSER(COMPILER &compiler, TOKEN_LIST &tokenList)
			: Compiler(compiler), Tokens(tokenList), Offset(0)
		{
			assert(!Tokens.empty());
		}

		TOKEN *CurrentToken()
		{
			assert(Offset < Tokens.size());
			return &Tokens[Offset];
		}

		TOKEN &PeekToken(size_t off = 1)
		{
			if ((Offset + off) >= Tokens.size())
				return Tokens[Tokens.size() - 1];
			return Tokens[Offset + off];
		}

		void SaveOffset()
		{
			OffsetStack.push(Offset);
		}

		void RestoreOffset()
		{
			Offset = OffsetStack.top();
			DropOffset();
		}

		void DropOffset()
		{
			OffsetStack.pop();
		}

		bool Accept(TOKEN_KIND kind)
		{
			if (Tokens[Offset].Kind == kind)
			{
				Offset++;
				if (Offset >= Tokens.size())
					Offset = Tokens.size() - 1;
				return true;
			}
			return false;
		}

		bool Accept(int kind)
		{
			return Accept(static_cast<TOKEN_KIND>(kind));
		}

		bool Expect(TOKEN_KIND kind)
		{
			if (!Accept(kind))
			{
				Compiler.Error(Tokens[Offset], "unexpected token '%', expecting '%'", 
					Tokens[Offset].GetKindName(), TokenKindName(kind));
				return false;
			}
			return true;
		}

		bool Expect(int kind)
		{
			return Expect(static_cast<TOKEN_KIND>(kind));
		}

		template< class NodePtrT >
		bool IsIgnored(NodePtrT &node)
		{
			switch (node->Kind)
			{
			case NK_EMPTY_DECL:
			case NK_EMPTY_STMT:
			case NK_COMMENT_DECL:
			case NK_COMMENT_STMT:
				return true;
			default:
				return false;
			}
		}

		std::string TokenText()
		{
			std::string text;
			CurrentToken()->GetText(text);
			return text;
		}

		AST_EXPR_PTR ParsePrimaryExpr()
		{
			auto startToken = CurrentToken();
			auto text = TokenText();
			if (Accept(TK_NIL))
				return std::make_unique<AST_NIL>(startToken, startToken);
			else if (Accept(TK_TRUE))
				return std::make_unique<AST_BOOL>(true, startToken, startToken);
			else if (Accept(TK_FALSE))
				return std::make_unique<AST_BOOL>(false, startToken, startToken);
			else if (Accept(TK_INT))
				return std::make_unique<AST_INT>(0, startToken, startToken);
			else if (Accept(TK_FLOAT))
				return std::make_unique<AST_FLOAT>(0.0, startToken, startToken);
			else if (Accept(TK_CHAR))
				return std::make_unique<AST_CHAR>(text, startToken, startToken);
			else if (Accept(TK_STRING))
				return std::make_unique<AST_STRING>(text, startToken, startToken);
			else if (Accept(TK_IDENT))
				return std::make_unique<AST_IDENTIFIER>(text, startToken, startToken);
			else if (Accept('('))
			{
				auto expr = ParseExpr();
				auto endToken = CurrentToken();
				if (!Expect(')'))
					return nullptr;
				expr->Start = startToken;
				expr->End = endToken;
				return expr;
			}
			else
			{
				return nullptr;
			}
		}

		AST_EXPR_PTR ParseExpr()
		{
			return ParsePrimaryExpr();
		}

		AST_STMT_PTR ParseStmt()
		{
			auto startToken = CurrentToken();
			if (auto stmt = ParseTopLevel())
				return stmt;
			else if (auto expr = ParseExpr()) 
			{
				auto endToken = CurrentToken();
				if (!Expect(';'))
					return nullptr;
				return std::make_unique<AST_EXPR_STMT>(std::move(expr), startToken, endToken);
			}
			return nullptr;
		}
		
		AST_TYPEREF_PTR ParseTypeRef();
		AST_DECL_PTR ParseTypeDef();
		AST_DECL_PTR ParseVarOrFuncDef();
		AST_DECL_PTR ParseParameter();
		AST_DECL_PTR ParseTopLevel();
	};

	struct BACKTRACKER
	{
		PARSER &Parser;
		bool Cancelled;
		BACKTRACKER(PARSER &p) : Parser(p), Cancelled(false)
		{ 
			Parser.SaveOffset(); 
		}
		~BACKTRACKER() 
		{ 
			if (!Cancelled) 
				Parser.RestoreOffset(); 
		}
		void Cancel()
		{ 
			if (!Cancelled)
			{
				Parser.DropOffset();
				Cancelled = true;
			}
		}
	};

	AST_TYPEREF_PTR PARSER::ParseTypeRef()
	{
		BACKTRACKER bt(*this);
		auto startToken = CurrentToken();
		auto typeFlags = TF_NONE;
		if (Accept(TK_CONST))
			typeFlags = TYPE_FLAGS(typeFlags | TF_CONST);
		auto name = TokenText();
		auto endToken = CurrentToken();
		if (!Accept(TK_IDENT))
			return nullptr;
		auto typeRef = std::make_unique<AST_TYPEREF>(name, typeFlags, startToken, endToken);
		while (true)
		{
			typeFlags = TF_NONE;
			endToken = CurrentToken();
			if (Accept('*'))
			{
				typeFlags = TYPE_FLAGS(typeFlags | TF_POINTER);
				if (CurrentToken()->Kind == TK_CONST)
				{
					endToken = CurrentToken();
					Expect(TK_CONST);
					typeFlags = TYPE_FLAGS(typeFlags | TF_CONST);
				}
				typeRef = std::make_unique<AST_TYPEREF>(std::move(typeRef), typeFlags, startToken, endToken);
			}
			else if (Accept('['))
			{
				if (!Expect(']'))
					return nullptr;
				typeFlags = TYPE_FLAGS(typeFlags | TF_ARRAY);
				if (CurrentToken()->Kind == TK_CONST)
				{
					endToken = CurrentToken();
					Expect(TK_CONST);
					typeFlags = TYPE_FLAGS(typeFlags | TF_CONST);
				}
				typeRef = std::make_unique<AST_TYPEREF>(std::move(typeRef), typeFlags, startToken, endToken);
			}
			else
				break;
		}
		bt.Cancel();
		return typeRef;
	}

	AST_DECL_PTR PARSER::ParseTypeDef()
	{
		auto startToken = CurrentToken();
		Expect(TK_TYPEDEF);
		auto typeRef = ParseTypeRef();
		assert(typeRef);
		auto name = TokenText();
		if (!Expect(TK_IDENT))
			return nullptr;
		auto endToken = CurrentToken();
		if (!Expect(';'))
			return nullptr;
		return std::make_unique<AST_TYPEDEF>(std::move(name), std::move(typeRef), startToken, endToken);
	}

	AST_DECL_PTR PARSER::ParseVarOrFuncDef()
	{
		BACKTRACKER bt(*this);
		auto startToken = CurrentToken();
		auto declFlags = DF_NONE;
		if (Accept(TK_STATIC))
			declFlags = DECL_FLAGS(declFlags | DF_STATIC);
		auto typeRef = ParseTypeRef();
		if (!typeRef)
			return nullptr;
		auto name = TokenText();
		if (!Expect(TK_IDENT))
			return nullptr;
		AST_DECL_LIST params;
		// Function
		if (Accept('('))
		{
			if (!Accept(')'))
			{
				while (true)
				{
					if (auto param = ParseParameter())
					{
						if (!IsIgnored(param))
							params.push_back(std::move(param));
						if (!Accept(','))
							break;
					}
					else
						break;
				}
				if (!Expect(')'))
					return nullptr;
			}
			if (!Expect('{'))
				return nullptr;
			AST_STMT_LIST stmts;
			auto endToken = CurrentToken();
			if (!Accept('}'))
			{
				while (true)
				{
					if (auto stmt = ParseStmt())
					{
						if (!IsIgnored(stmt))
							stmts.push_back(std::move(stmt));
					}
					else
						break;
				}
				endToken = CurrentToken();
				if (!Expect('}'))
					return nullptr;
			}
			auto fun = std::make_unique<AST_FUNC_DECL>(std::move(name), std::move(typeRef), std::move(params), std::move(stmts), startToken, endToken);
			fun->Flags = DECL_FLAGS(fun->Flags | declFlags);
			bt.Cancel();
			return fun;
		}
		// Variable
		else
		{
			AST_EXPR_PTR initExpr;
			if (Accept('='))
				initExpr = ParseExpr();
			auto endToken = CurrentToken();
			if (!Expect(';'))
				return nullptr;
			auto var = std::make_unique<AST_VAR_DECL>(std::move(name), std::move(typeRef), std::move(initExpr), startToken, endToken);
			var->Flags = DECL_FLAGS(var->Flags | declFlags);
			bt.Cancel();
			return var;
		}
	}

	AST_DECL_PTR PARSER::ParseParameter()
	{
		auto startToken = CurrentToken();
		auto typeRef = ParseTypeRef();
		assert(typeRef);
		auto name = TokenText();
		auto endToken = CurrentToken();
		if (!Expect(TK_IDENT))
			return nullptr;
		AST_EXPR_PTR defExpr;
		if (Accept('=')) {
			defExpr = ParseExpr();
			assert(defExpr);
			endToken = defExpr->End;
		}
		return std::make_unique<AST_PARAM_DECL>(std::move(name), std::move(typeRef), std::move(defExpr), startToken, endToken);
	}

	AST_DECL_PTR PARSER::ParseTopLevel()
	{
		auto startToken = CurrentToken();
		if (CurrentToken()->Kind == ';')
		{
			auto endToken = CurrentToken();
			Expect(';');
			return std::make_unique<AST_EMPTY_DECL>(startToken, endToken);
		}
		else if (CurrentToken()->Kind == TK_COMMENT)
		{
			std::string text;
			CurrentToken()->GetText(text);
			auto endToken = CurrentToken();
			Expect(TK_COMMENT);
			return std::make_unique<AST_COMMENT_DECL>(text, startToken, endToken);
		}
		else if (CurrentToken()->Kind == TK_TYPEDEF)
		{
			return ParseTypeDef();
		}
		else
		{
			return ParseVarOrFuncDef();
		}
	}

	std::unique_ptr<AST_MODULE> ParseTokens(COMPILER &compiler, TOKEN_LIST &tokenList)
	{
		PARSER parser(compiler, tokenList);
		auto mod = std::make_unique<AST_MODULE>();
		while (true)
		{
			if (auto decl = parser.ParseTopLevel())
			{
				if (!parser.IsIgnored(decl))
					mod->Members.push_back(std::move(decl));
			}
			else
			{
				break;
			}
		}
		return mod;
	}

} // namespace SODA