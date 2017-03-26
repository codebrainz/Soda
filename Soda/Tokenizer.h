#pragma once

#include <ostream>
#include <string>
#include <vector>

namespace Soda
{

	class Compiler;
	class SourceFile;

	enum TokenKind
	{
		TK_ERROR = -1,
		TK_EOF = 0,

		TK_LPAREN = '(',
		TK_RPAREN = ')',
		TK_LBRACE = '{',
		TK_RBRACE = '}',
		TK_LBRACKET = '[',
		TK_RBRACKET = ']',
		TK_COLON = ':',
		TK_SEMICOLON = ';',
		TK_COMMA = ',',

		TK_PLUS = '+',
		TK_MINUS = '-',
		TK_MUL = '*',
		TK_DIV = '/',
		TK_MOD = '%',
		TK_BAND = '&',
		TK_BOR = '|',
		TK_BXOR = '^',
		TK_LT = '<',
		TK_GT = '>',

		TK_IDENT = 256,
		TK_CONST,
		TK_STATIC,
		TK_TYPEDEF,
		TK_IF,
		TK_ELSE,
		TK_SWITCH,
		TK_CASE,
		TK_DEFAULT,
		TK_BREAK,
		TK_CONTINUE,
		TK_RETURN,
		TK_FOR,
		TK_WHILE,

		TK_COMMENT,
		TK_NIL,
		TK_TRUE,
		TK_FALSE,
		TK_INT,
		TK_FLOAT,
		TK_STRING,
		TK_CHAR,

		TK_INCR,
		TK_DECR,

		TK_LEFT,
		TK_RIGHT,

		TK_EQ,
		TK_NE,
		TK_LE,
		TK_GE,

		TK_AND,
		TK_OR,

		TK_IADD,
		TK_ISUB,
		TK_IMUL,
		TK_IDIV,
		TK_IMOD,
		TK_ILEFT,
		TK_IRIGHT,
		TK_IAND,
		TK_IOR,
		TK_IXOR,
	};

	const char *tokenKindName(TokenKind kind);

	struct Token
	{
		TokenKind kind;
		SourceFile &file;
		size_t start, end;

		Token(SourceFile &file, TokenKind kind = TK_EOF, size_t start = 0, size_t end = 0)
			: kind(kind), file(file), start(start), end(end) {}

		void getText(std::string &text) const;

		const char *getKindName() const 
		{ 
			return tokenKindName(kind); 
		}

		friend std::ostream &operator<<(std::ostream &os, const Token &token)
		{
			std::string tokenText;
			token.getText(tokenText);
			os << "(TOKEN " 
				<< token.getKindName() << " '" 
				<< tokenText << "' " 
				<< token.start << " " 
				<< token.end << ")";
			return os;
		}
	};

	typedef std::vector<Token> TokenList;

	bool tokenizeFile(Compiler &compiler, SourceFile &sourceFile, TokenList &tokenList);

}