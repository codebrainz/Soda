#include "Parser.h"
#include "Ast.h"
#include "Compiler.h"
#include "SymbolTable.h"
#include <cassert>
#include <stack>
#include <vector>

namespace Soda
{

	struct Parser
	{
		Compiler &compiler;
		TokenList &tokens;
		size_t offset;
		std::stack<size_t> offsetStack;
		std::stack<Token*> tokenStack;

		Parser(Compiler &compiler, TokenList &tokenList)
			: compiler(compiler), tokens(tokenList), offset(0)
		{
			assert(!tokens.empty());
		}

		Token *currentToken()
		{
			assert(offset < tokens.size());
			return &tokens[offset];
		}

		Token &peekToken(size_t off = 1)
		{
			if ((offset + off) >= tokens.size())
				return tokens[tokens.size() - 1];
			return tokens[offset + off];
		}

		void saveOffset()
		{
			offsetStack.push(offset);
		}

		void restoreOffset()
		{
			offset = offsetStack.top();
			dropOffset();
		}

		void dropOffset()
		{
			offsetStack.pop();
		}

		bool accept(TokenKind kind)
		{
			if (tokens[offset].kind == kind)
			{
				offset++;
				if (offset >= tokens.size())
					offset = tokens.size() - 1;
				return true;
			}
			return false;
		}

		bool accept(int kind)
		{
			return accept(static_cast<TokenKind>(kind));
		}

		bool expect(TokenKind kind)
		{
			if (!accept(kind))
			{
				compiler.error(tokens[offset], "unexpected token '%', expecting '%'", 
					tokens[offset].getKindName(), tokenKindName(kind));
				return false;
			}
			return true;
		}

		bool expect(int kind)
		{
			return expect(static_cast<TokenKind>(kind));
		}

		template< class NodePtrT >
		bool isIgnored(NodePtrT &node)
		{
			switch (node->kind)
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

		std::string tokenText()
		{
			std::string text;
			currentToken()->getText(text);
			return text;
		}

		AstExprPtr parsePrimaryExpr()
		{
			auto startToken = currentToken();
			auto text = tokenText();
			if (accept(TK_NIL))
				return std::make_unique<AstNil>(startToken, startToken);
			else if (accept(TK_TRUE))
				return std::make_unique<AstBool>(true, startToken, startToken);
			else if (accept(TK_FALSE))
				return std::make_unique<AstBool>(false, startToken, startToken);
			else if (accept(TK_INT))
				return std::make_unique<AstInt>(0, startToken, startToken);
			else if (accept(TK_FLOAT))
				return std::make_unique<AstFloat>(0.0, startToken, startToken);
			else if (accept(TK_CHAR))
				return std::make_unique<AstChar>(text, startToken, startToken);
			else if (accept(TK_STRING))
				return std::make_unique<AstString>(text, startToken, startToken);
			else if (accept(TK_IDENT))
				return std::make_unique<AstIdentifier>(text, startToken, startToken);
			else if (accept('('))
			{
				auto expr = parseExpr();
				auto endToken = currentToken();
				if (!expect(')'))
					return nullptr;
				expr->start = startToken;
				expr->end = endToken;
				return expr;
			}
			else
			{
				return nullptr;
			}
		}

		AstExprPtr parsePrefixExpr()
		{
			auto startToken = currentToken();
			if (accept('*'))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_DEREF, std::move(expr), startToken, endToken);
			}
			else
			{
				return parsePrimaryExpr();
			}
		}

		AstExprPtr parseCastExpr();

		AstExprPtr parseMultiplicativeExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseCastExpr();
			while (true)
			{
				if (accept('*'))
				{
					auto rhs = parseCastExpr();
					lhs = std::make_unique<AstBinary>(BOP_MUL, std::move(lhs), std::move(rhs), startToken, rhs->end);
					startToken = currentToken();
				}
				else if (accept('/'))
				{
					auto rhs = parseCastExpr();
					lhs = std::make_unique<AstBinary>(BOP_DIV, std::move(lhs), std::move(rhs), startToken, rhs->end);
					startToken = currentToken();
				}
				else if (accept('%'))
				{
					auto rhs = parseCastExpr();
					lhs = std::make_unique<AstBinary>(BOP_MOD, std::move(lhs), std::move(rhs), startToken, rhs->end);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		AstExprPtr parseAdditiveExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseMultiplicativeExpr();
			while (true)
			{
				if (accept('+'))
				{
					auto rhs = parseMultiplicativeExpr();
					lhs = std::make_unique<AstBinary>(BOP_ADD, std::move(lhs), std::move(rhs), startToken, rhs->end);
					startToken = currentToken();
				}
				else if (accept('-'))
				{
					auto rhs = parseMultiplicativeExpr();
					lhs = std::make_unique<AstBinary>(BOP_SUB, std::move(lhs), std::move(rhs), startToken, rhs->end);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		AstExprPtr parseExpr()
		{
			return parseAdditiveExpr();
		}

		AstStmtPtr parseAmbiguousMulExprVarDecl(bool &isExpr);
		AstStmtPtr parseExprStmt();
		AstStmtPtr parseLocalStmt();		
		AstTypeRefPtr parseTypeRef();
		AstDeclPtr parseTypeDef();
		AstDeclPtr parseVarOrFuncDef();
		AstDeclPtr parseParameter();
		AstDeclPtr parseTopLevel();
	};

	struct BackTracker
	{
		Parser &parser;
		bool cancelled;
		BackTracker(Parser &p) : parser(p), cancelled(false)
		{ 
			parser.saveOffset(); 
		}
		~BackTracker() 
		{ 
			if (!cancelled) 
				parser.restoreOffset(); 
		}
		void cancel()
		{ 
			if (!cancelled)
			{
				parser.dropOffset();
				cancelled = true;
			}
		}
	};	

	AstExprPtr Parser::parseCastExpr()
	{
		auto startToken = currentToken();

		// (int) expr
		{
			BackTracker bt(*this);
			if (accept('('))
			{
				auto typeName = tokenText();
				if (accept(TK_IDENT))
				{
					if (accept(')'))
					{
						if (auto expr = parseExpr())
						{
							auto endToken = currentToken();
							auto type = std::make_unique<AstTypeRef>(std::move(typeName), TF_NONE);
							bt.cancel();
							return std::make_unique<AstCast>(std::move(type), std::move(expr), startToken, endToken);
						}
					}
				}
			}
		}

		// (const int*) expr // etc...
		{
			BackTracker bt(*this);
			if (accept('('))
			{
				if (auto typeRef = parseTypeRef())
				{
					if (accept(')'))
					{
						if (auto expr = parseExpr())
						{
							auto endToken = currentToken();
							bt.cancel();
							return std::make_unique<AstCast>(std::move(typeRef), std::move(expr), startToken, endToken);
						}
					}
				}
			}
		}

		return parsePrefixExpr();
	}

	AstStmtPtr Parser::parseAmbiguousMulExprVarDecl(bool &isAmbiguous)
	{
		BackTracker bt(*this);
		auto startToken = currentToken();
		auto typeName = tokenText();
		if (accept(TK_IDENT))
		{
			if (accept('*'))
			{
				auto name = tokenText();
				if (accept(TK_IDENT))
				{
					// int * x = ...;
					if (accept('='))
					{
						auto initExpr = parseExpr();
						auto endToken = currentToken();
						if (!expect(';'))
							return nullptr;
						auto innerType = std::make_unique<AstTypeRef>(typeName, TF_NONE);
						auto ptrType = std::make_unique<AstTypeRef>(std::move(innerType), TF_POINTER);
						bt.cancel();
						return std::make_unique<AstVarDecl>(std::move(name), std::move(ptrType), std::move(initExpr), startToken, endToken);
					}
					// int * x;
					else if (currentToken()->kind == ';')
					{
						auto endToken = currentToken();
						if (!expect(';'))
							return nullptr;
						auto innerType = std::make_unique<AstTypeRef>(typeName, TF_NONE);
						auto ptrType = std::make_unique<AstTypeRef>(std::move(innerType), TF_POINTER);
						isAmbiguous = true;
						return std::make_unique<AstVarDecl>(std::move(name), std::move(ptrType), startToken, endToken);
					}
				}
			}
		}
		return nullptr;
	}

	AstStmtPtr Parser::parseExprStmt()
	{
		auto startToken = currentToken();
		if (auto expr = parseExpr())
		{
			auto endToken = currentToken();
			if (!expect(';'))
				return nullptr;
			return std::make_unique<AstExprStmt>(std::move(expr), startToken, endToken);
		}
		return nullptr;
	}
	
	AstStmtPtr Parser::parseLocalStmt()
	{
		auto startToken = currentToken();
		
		if (currentToken()->kind == TK_IDENT)
		{
			bool isAmbiguous = false;
			if (auto varDecl = parseAmbiguousMulExprVarDecl(isAmbiguous))
			{
				if (!isAmbiguous)
					return varDecl;
				else
				{
					auto exprStmt = parseExprStmt();
					assert(exprStmt);
					auto ambigStmt = std::make_unique<AstAmbiguityStmt>(startToken, exprStmt->end);
					ambigStmt->alternatives.push_back(std::move(varDecl));
					ambigStmt->alternatives.push_back(std::move(exprStmt));
					return ambigStmt;
				}
			}
		}

		if (auto stmt = parseTopLevel())
			return stmt;
		else if (auto expr = parseExprStmt())
			return stmt;
		return nullptr;
	}

	AstTypeRefPtr Parser::parseTypeRef()
	{
		BackTracker bt(*this);
		auto startToken = currentToken();
		auto typeFlags = TF_NONE;
		if (accept(TK_CONST))
			typeFlags = TypeFlags(typeFlags | TF_CONST);
		auto name = tokenText();
		auto endToken = currentToken();
		if (!accept(TK_IDENT))
			return nullptr;
		auto typeRef = std::make_unique<AstTypeRef>(name, typeFlags, startToken, endToken);
		while (true)
		{
			typeFlags = TF_NONE;
			endToken = currentToken();
			if (accept('*'))
			{
				typeFlags = TypeFlags(typeFlags | TF_POINTER);
				if (currentToken()->kind == TK_CONST)
				{
					endToken = currentToken();
					expect(TK_CONST);
					typeFlags = TypeFlags(typeFlags | TF_CONST);
				}
				typeRef = std::make_unique<AstTypeRef>(std::move(typeRef), typeFlags, startToken, endToken);
			}
			else if (accept('['))
			{
				if (!expect(']'))
					return nullptr;
				typeFlags = TypeFlags(typeFlags | TF_ARRAY);
				if (currentToken()->kind == TK_CONST)
				{
					endToken = currentToken();
					expect(TK_CONST);
					typeFlags = TypeFlags(typeFlags | TF_CONST);
				}
				typeRef = std::make_unique<AstTypeRef>(std::move(typeRef), typeFlags, startToken, endToken);
			}
			else
				break;
		}
		bt.cancel();
		return typeRef;
	}

	AstDeclPtr Parser::parseTypeDef()
	{
		auto startToken = currentToken();
		expect(TK_TYPEDEF);
		auto typeRef = parseTypeRef();
		assert(typeRef);
		auto name = tokenText();
		if (!expect(TK_IDENT))
			return nullptr;
		auto endToken = currentToken();
		if (!expect(';'))
			return nullptr;
		return std::make_unique<AstTypedef>(std::move(name), std::move(typeRef), startToken, endToken);
	}

	AstDeclPtr Parser::parseVarOrFuncDef()
	{
		BackTracker bt(*this);
		auto startToken = currentToken();
		auto declFlags = DF_NONE;
		if (accept(TK_STATIC))
			declFlags = DeclFlags(declFlags | DF_STATIC);
		auto typeRef = parseTypeRef();
		if (!typeRef)
			return nullptr;
		auto name = tokenText();
		if (!expect(TK_IDENT))
			return nullptr;
		AstDeclList params;
		// Function
		if (accept('('))
		{
			if (!accept(')'))
			{
				while (true)
				{
					if (auto param = parseParameter())
					{
						if (!isIgnored(param))
							params.push_back(std::move(param));
						if (!accept(','))
							break;
					}
					else
						break;
				}
				if (!expect(')'))
					return nullptr;
			}
			if (!expect('{'))
				return nullptr;
			AstStmtList stmts;
			auto endToken = currentToken();
			if (!accept('}'))
			{
				while (true)
				{
					if (auto stmt = parseLocalStmt())
					{
						if (!isIgnored(stmt))
							stmts.push_back(std::move(stmt));
					}
					else
						break;
				}
				endToken = currentToken();
				if (!expect('}'))
					return nullptr;
			}
			auto fun = std::make_unique<AstFuncDecl>(std::move(name), std::move(typeRef), std::move(params), std::move(stmts), startToken, endToken);
			fun->flags = DeclFlags(fun->flags | declFlags);
			bt.cancel();
			return fun;
		}
		// Variable
		else
		{
			AstExprPtr initExpr;
			if (accept('='))
				initExpr = parseExpr();
			auto endToken = currentToken();
			if (!expect(';'))
				return nullptr;
			auto var = std::make_unique<AstVarDecl>(std::move(name), std::move(typeRef), std::move(initExpr), startToken, endToken);
			var->flags = DeclFlags(var->flags | declFlags);
			bt.cancel();
			return var;
		}
	}

	AstDeclPtr Parser::parseParameter()
	{
		auto startToken = currentToken();
		auto typeRef = parseTypeRef();
		assert(typeRef);
		auto name = tokenText();
		auto endToken = currentToken();
		if (!expect(TK_IDENT))
			return nullptr;
		AstExprPtr defExpr;
		if (accept('=')) {
			defExpr = parseExpr();
			assert(defExpr);
			endToken = defExpr->end;
		}
		return std::make_unique<AstParamDecl>(std::move(name), std::move(typeRef), std::move(defExpr), startToken, endToken);
	}

	AstDeclPtr Parser::parseTopLevel()
	{
		auto startToken = currentToken();
		if (currentToken()->kind == ';')
		{
			auto endToken = currentToken();
			expect(';');
			return std::make_unique<AstEmptyDecl>(startToken, endToken);
		}
		else if (currentToken()->kind == TK_COMMENT)
		{
			std::string text;
			currentToken()->getText(text);
			auto endToken = currentToken();
			expect(TK_COMMENT);
			return std::make_unique<AstCommentDecl>(text, startToken, endToken);
		}
		else if (currentToken()->kind == TK_TYPEDEF)
		{
			return parseTypeDef();
		}
		else
		{
			return parseVarOrFuncDef();
		}
	}

	std::unique_ptr<AstModule> parseTokens(Compiler &compiler, TokenList &tokenList)
	{
		Parser parser(compiler, tokenList);
		auto mod = std::make_unique<AstModule>();
		while (true)
		{
			if (auto decl = parser.parseTopLevel())
			{
				if (!parser.isIgnored(decl))
					mod->members.push_back(std::move(decl));
			}
			else
			{
				break;
			}
		}
		return mod;
	}

} // namespace Soda