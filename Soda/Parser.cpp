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

		//> primary_expr: NIL
		//>             | TRUE
		//>             | FALSE
		//>             | INT
		//>             | FLOAT
		//>             | CHAR
		//>             | STRING
		//>             | IDENT
		//>             | '(' expr ')'
		//>             ;
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
			// TODO: function expression
			else
			{
				return nullptr;
			}
		}

		//> postfix_expr: primary_expr
		//>             | primary_expr '[' expr ']'
		//>             | primary_expr '(' argument_list? ')'
		//>             | primary_expr '.' IDENT
		//>             | primary_expr INCR
		//>             | primary_expr DECR
		//>             ;
		AstExprPtr parsePostfixExpr()
		{
			auto startToken = currentToken();
			auto expr = parsePrimaryExpr();
			if (!expr)
				return nullptr;
			while (true)
			{
				if (accept('['))
				{
					auto index = parseExpr();
					auto endToken = currentToken();
					if (!expect(']'))
						return nullptr;
					expr = std::make_unique<AstIndexExpr>(std::move(expr), std::move(index), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept('('))
				{
					AstExprList args;
					auto endToken = currentToken();
					if (!accept(')'))
					{
						while (true)
						{
							if (auto arg = parseExpr())
							{
								args.push_back(std::move(arg));
								if (!accept(','))
									break;
							}
							else
								break;
						}
						endToken = currentToken();
						if (!expect(')'))
							return nullptr;
					}
					expr = std::make_unique<AstCallExpr>(std::move(expr), std::move(args), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept('.'))
				{
					auto member = tokenText();
					auto endToken = currentToken();
					if (!expect(TK_IDENT))
						return nullptr;
					expr = std::make_unique<AstMemberExpr>(std::move(expr), std::move(member), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					auto endToken = currentToken();
					if (accept(TK_INCR))
						expr = std::make_unique<AstUnary>(UOP_POSTINC, std::move(expr), startToken, endToken);
					else if (accept(TK_DECR))
						expr = std::make_unique<AstUnary>(UOP_POSTDEC, std::move(expr), startToken, endToken);
					else
						break;
					startToken = currentToken();
				}
			}
			return expr;
		}

		//> prefix_expr: postfix_expr
		//>            | '*' postfix_expr
		//>            | '~' postfix_expr
		//>            | '!' postfix_expr
		//>            | '+' postfix_expr
		//>            | '-' postfix_expr
		//>            | INCR postfix_expr
		//>            | DECR postfix_expr
		//>            ;
		AstExprPtr parsePrefixExpr()
		{
			auto startToken = currentToken();
			if (accept('*'))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_DEREF, std::move(expr), startToken, endToken);
			}
			else if (accept('~'))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_COMPL, std::move(expr), startToken, endToken);
			}
			else if (accept('!'))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_NOT, std::move(expr), startToken, endToken);
			}
			else if (accept('+'))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_POS, std::move(expr), startToken, endToken);
			}
			else if (accept('-'))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_NEG, std::move(expr), startToken, endToken);
			}
			else if (accept(TK_INCR))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_PREINC, std::move(expr), startToken, endToken);
			}
			else if (accept(TK_DECR))
			{
				auto expr = parsePrefixExpr();
				auto endToken = expr->end;
				return std::make_unique<AstUnary>(UOP_PREDEC, std::move(expr), startToken, endToken);
			}
			else
			{
				return parsePostfixExpr();
			}
		}

		//> cast_expr: prefix_expr
		//>          | '(' typeref ')' prefix_expr
		//>          ;
		AstExprPtr parseCastExpr()
		{
			auto startToken = currentToken();
			{
				BackTracker bt(*this);
				if (accept('('))
				{
					if (auto typeRef = parseTypeRef())
					{
						if (accept(')'))
						{
							if (auto expr = parsePrefixExpr())
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

		//> multiplicative_expr: cast_expr
		//>                    | cast_expr '*' cast_expr
		//>                    | cast_expr '/' cast_expr
		//>                    | cast_expr '%' cast_expr
		//>                    ;
		AstExprPtr parseMultiplicativeExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseCastExpr();
			while (true)
			{
				if (accept('*'))
				{
					auto rhs = parseCastExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_MUL, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept('/'))
				{
					auto rhs = parseCastExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_DIV, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept('%'))
				{
					auto rhs = parseCastExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_MOD, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> additive_expr: multiplicative_expr
		//>              | multiplicative_expr '+' multiplicative_expr
		//>              | multiplicative_expr '-' multiplicative_expr
		//>              ;
		AstExprPtr parseAdditiveExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseMultiplicativeExpr();
			while (true)
			{
				if (accept('+'))
				{
					auto rhs = parseMultiplicativeExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_ADD, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept('-'))
				{
					auto rhs = parseMultiplicativeExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_SUB, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> shift_expr: shift_expr
		//>           | shift_expr LSHIFT shift_expr
		//>           | shift_expr RSHIFT shift_expr
		//>           ;
		AstExprPtr parseShiftExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseAdditiveExpr();
			while (true)
			{
				if (accept(TK_LEFT))
				{
					auto rhs = parseAdditiveExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_LSHIFT, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept(TK_RIGHT))
				{
					auto rhs = parseAdditiveExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_RSHIFT, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> relational_expr: shift_expr
		//>                | shift_expr '<' shift_expr
		//>                | shift_expr '>' shift_expr
		//>                | shift_expr LE shift_expr
		//>                | shift_expr GT shift_expr
		//>                ;
		AstExprPtr parseRelationalExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseAdditiveExpr();
			while (true)
			{
				if (accept('<'))
				{
					auto rhs = parseAdditiveExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_LT, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept('>'))
				{
					auto rhs = parseAdditiveExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_GT, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept(TK_LE))
				{
					auto rhs = parseAdditiveExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_LE, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept(TK_GE))
				{
					auto rhs = parseAdditiveExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_GE, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> equality_expr: relational_expr EQ relational_expr
		//>              | relational_expr NE relational_expr
		//>              ;
		AstExprPtr parseEqualityExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseRelationalExpr();
			while (true)
			{
				if (accept(TK_EQ))
				{
					auto rhs = parseRelationalExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_EQ, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else if (accept(TK_NE))
				{
					auto rhs = parseRelationalExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_NE, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> and_expr: equality_expr
		//>         | equality_expr '&' equality_expr
		//>         ;
		AstExprPtr parseAndExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseEqualityExpr();
			while (true)
			{
				if (accept('&'))
				{
					auto rhs = parseEqualityExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_BIT_AND, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> xor_expr: and_expr
		//>         | and_expr '^' and_expr
		//>         ;
		AstExprPtr parseXorExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseAndExpr();
			while (true)
			{
				if (accept('^'))
				{
					auto rhs = parseAndExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_BIT_XOR, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> or_expr: xor_expr
		//>        | xor_expr '|' xor_expr
		//>        ;
		AstExprPtr parseOrExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseXorExpr();
			while (true)
			{
				if (accept('|'))
				{
					auto rhs = parseXorExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_BIT_OR, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> logical_and_expr: or_expr
		//>                 | or_expr AND or_expr
		//>                 ;
		AstExprPtr parseLogicalAndExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseOrExpr();
			while (true)
			{
				if (accept(TK_AND))
				{
					auto rhs = parseOrExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_LOG_AND, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> logical_or_expr: logical_and_expr
		//>                | logical_and_expr OR logical_and_expr
		//>                ;
		AstExprPtr parseLogicalOrExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseAndExpr();
			while (true)
			{
				if (accept(TK_OR))
				{
					auto rhs = parseAndExpr();
					auto endToken = rhs->end;
					lhs = std::make_unique<AstBinary>(BOP_LOG_OR, std::move(lhs), std::move(rhs), startToken, endToken);
					startToken = currentToken();
				}
				else
				{
					break;
				}
			}
			return lhs;
		}

		//> if_expr: logical_or_expr
		//>        | logical_or_expr '?' logical_or_expr ':' logical_or_expr
		//>        ;
		AstExprPtr parseIfExpr()
		{
			auto startToken = currentToken();
			auto condExpr = parseLogicalOrExpr();
			if (accept('?'))
			{
				auto thenExpr = parseLogicalOrExpr();
				if (!expect(':'))
					return nullptr;
				auto elseExpr = parseLogicalOrExpr();
				auto endToken = elseExpr->end;
				return std::make_unique<AstIfExpr>(std::move(condExpr), std::move(thenExpr), std::move(elseExpr), startToken, endToken);
			}
			return condExpr;
		}

		//> assign_expr: if_expr
		//>            | if_expr '=' if_expr
		//>            | if_expr MUL_ASSIGN if_expr
		//>            | if_expr DIV_ASSIGN if_expr
		//>            | if_expr MOD_ASSIGN if_expr
		//>            | if_expr ADD_ASSIGN if_expr
		//>            | if_expr SUB_ASSIGN if_expr
		//>            | if_expr LSHIFT_ASSIGN if_expr
		//>            | if_expr RSHIFT_ASSIGN if_expr
		//>            | if_expr AND_ASSIGN if_expr
		//>            | if_expr OR_ASSIGN if_expr
		//>            | if_expr XOR_ASSIGN if_expr
		//>            ;
		AstExprPtr parseAssignExpr()
		{
			auto startToken = currentToken();
			auto lhs = parseIfExpr();
			if (!lhs)
				return nullptr;
			if (accept('='))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_ASSIGN, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IMUL))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IMUL, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IDIV))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IDIV, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IMOD))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IMOD, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IADD))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IADD, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_ISUB))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_ISUB, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_ILEFT))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_ILSHIFT, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IRIGHT))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IRSHIFT, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IAND))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IAND, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IOR))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IOR, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else if (accept(TK_IXOR))
			{
				auto rhs = parseIfExpr();
				auto endToken = rhs->end;
				return std::make_unique<AstBinary>(BOP_IXOR, std::move(lhs), std::move(rhs), startToken, endToken);
			}
			else
			{
				return lhs;
			}
		}

		//> expr: assign_expr
		//>     ;
		AstExprPtr parseExpr()
		{
			return parseAssignExpr();
		}

		//> expr_stmt: expr ';'
		//>          ;
		AstStmtPtr parseExprStmt()
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

		//> block_stmt: '{' local_stmt_list '}'
		//>           ;
		AstStmtPtr parseBlockStmt()
		{
			auto startToken = currentToken();
			if (!expect('{'))
				return nullptr;
			auto stmtList = parseLocalStmtList();
			auto endToken = currentToken();
			if (!expect('}'))
				return nullptr;
			return std::make_unique<AstBlockStmt>(std::move(stmtList), startToken, endToken);
		}

		//> return_stmt: RETURN expr? ';'
		//>            ;
		AstStmtPtr parseReturnStmt()
		{
			auto startToken = currentToken();
			if (!expect(TK_RETURN))
				return nullptr;
			auto endToken = currentToken();
			if (!accept(';'))
			{
				auto expr = parseExpr();
				assert(expr);
				endToken = expr->end;
				if (!expect(';'))
					return nullptr;
				return std::make_unique<AstReturnStmt>(std::move(expr), startToken, endToken);
			}
			else
			{
				return std::make_unique<AstReturnStmt>(startToken, endToken);
			}
		}

		//> break_stmt: BREAK ';'
		//>           ;
		AstStmtPtr parseBreakStmt()
		{
			auto startToken = currentToken();
			if (!expect(TK_BREAK))
				return nullptr;
			auto endToken = currentToken();
			if (!expect(';'))
				return nullptr;
			return std::make_unique<AstBreakStmt>(startToken, endToken);
		}

		//> continue_stmt: CONTINUE ';'
		//>              ;
		AstStmtPtr parseContinueStmt()
		{
			auto startToken = currentToken();
			if (!expect(TK_CONTINUE))
				return nullptr;
			auto endToken = currentToken();
			if (!expect(';'))
				return nullptr;
			return std::make_unique<AstContinueStmt>(startToken, endToken);
		}

		//> goto_stmt: GOTO IDENT ';'
		//>          ;
		AstStmtPtr parseGotoStmt()
		{
			auto startToken = currentToken();
			if (!expect(TK_GOTO))
				return nullptr;
			auto label = tokenText();
			if (!expect(TK_IDENT))
				return nullptr;
			auto endToken = currentToken();
			if (!expect(';'))
				return nullptr;
			return std::make_unique<AstGotoStmt>(std::move(label), startToken, endToken);
		}

		//> if_stmt: IF '(' expr ')' stmt
		//>        | IF '(' expr ')' stmt ELSE stmt
		//>        ;
		AstStmtPtr parseIfStmt()
		{
			auto startToken = currentToken();
			if (!expect(TK_IF))
				return nullptr;
			if (!expect('('))
				return nullptr;
			auto condExpr = parseExpr();
			if (!expect(')'))
				return nullptr;
			auto thenStmt = parseLocalStmt();
			auto endToken = thenStmt->end;
			AstStmtPtr elseStmt;
			if (accept(TK_ELSE))
			{
				elseStmt = parseLocalStmt();
				endToken = elseStmt->end;
			}
			return std::make_unique<AstIfStmt>(std::move(condExpr), std::move(thenStmt), std::move(elseStmt), startToken, endToken);
		}

		//> local_stmt: ';'
		//>           | COMMENT
		//>           | block_stmt
		//>           | return_stmt
		//>           | break_stmt
		//>           | continue_stmt
		//>           | goto_stmt
		//>           | if_stmt
		//>           | decl
		//>           | expr_stmt
		//>           ;
		AstStmtPtr parseLocalStmt()
		{
			auto startToken = currentToken();
			auto kind = currentToken()->kind;
			if (accept(';'))
				return std::make_unique<AstEmptyStmt>(startToken, startToken);
			else if (kind == TK_COMMENT)
			{
				auto text = tokenText();
				auto endToken = currentToken();
				if (!expect(TK_COMMENT))
					return nullptr;
				return std::make_unique<AstCommentDecl>(text, startToken, endToken);
			}
			else if (kind == '{')
				return parseBlockStmt();
			else if (kind == TK_RETURN)
				return parseReturnStmt();
			else if (kind == TK_BREAK)
				return parseBreakStmt();
			else if (kind == TK_CONTINUE)
				return parseContinueStmt();
			else if (kind == TK_GOTO)
				return parseGotoStmt();
			else if (kind == TK_IF)
				return parseIfStmt();
			else if (auto decl = parseDecl())
				return decl;
			else if (auto stmt = parseExprStmt())
				return stmt;
			return nullptr;
		}

		//> local_stmt_list: local_stmt
		//>                | local_stmt_list local_stmt
		//>                ;
		AstStmtList parseLocalStmtList()
		{
			AstStmtList stmts;
			while (true)
			{
				if (auto stmt = parseLocalStmt())
				{
					if (!isIgnored(stmt))
						stmts.push_back(std::move(stmt));
				}
				else
				{
					break;
				}
			}
			return stmts;
		}
		
		//> type_member: TK_IDENT
		//>            | type_member '.' TK_IDENT
		//>            ;
		bool Parser::parseTypeMember(std::string &composedName, Token *endToken)
		{
			while (true)
			{
				auto name = tokenText();
				endToken = currentToken();
				if (accept(TK_IDENT))
				{
					composedName += name;
					if (!accept('.'))
						break;
				}
				else
				{
					return false;
				}
			}
			return true;
		}

		//> type_ref: CONST? type_member [ ('*'| '[' ']') CONST? ]*
		//>         ;
		AstTypeRefPtr parseTypeRef()
		{
			BackTracker bt(*this);
			auto startToken = currentToken();
			auto typeFlags = TF_NONE;
			if (accept(TK_CONST))
				typeFlags = TypeFlags(typeFlags | TF_CONST);
			std::string name;
			Token *endToken = nullptr;
			if (!parseTypeMember(name, endToken))
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

		//> decl_specifiers: %empty
		//>                | STATIC
		//>                ;
		//>
		//> variable: decl_specifiers type_ref IDENT ( '=' expr )? ';'
		//>         ;
		//>
		//> function: decl_specifiers type_ref IDENT '(' parameter_list? ')' '{' stmt_list? '}'
		//>         ;
		//>
		//> parameter_list: parameter
		//>               | parameter_list ',' parameter
		//>               ;
		//>
		//> stmt_list: stmt
		//>          | stmt_list stmt
		//>          ;
		AstDeclPtr parseVarOrFuncDef()
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

		//> parameter: type_ref IDENT
		//>          | type_ref IDENT [ '=' expr ]
		//>          ;
		AstDeclPtr parseParameter()
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

		//> typedef: TYPEDEF type_ref IDENT
		//>        ;
		AstDeclPtr parseTypeDef()
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

		//> decl: ';'
		//>     | COMMENT
		//>     | typedef
		//>     | variable
		//>     | function
		//>     ;
		AstDeclPtr parseDecl()
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
	};

	std::unique_ptr<AstModule> parseTokens(Compiler &compiler, TokenList &tokenList)
	{
		Parser parser(compiler, tokenList);
		Token *startToken = nullptr;
		Token *endToken = nullptr;
		if (!tokenList.empty())
		{
			startToken = &tokenList.front();
			endToken = &tokenList.back();
		}
		auto mod = std::make_unique<AstModule>(startToken, endToken);
		while (true)
		{
			if (auto decl = parser.parseDecl())
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