#include "Tokenizer.h"
#include "Compiler.h"
#include "SourceFile.h"
#include <iostream>

namespace SODA
{
	struct LEXER
	{
		COMPILER &Compiler;
		SOURCE_FILE &SourceFile;
		int Char;
		size_t Offset;
		size_t FileSize;

		LEXER(COMPILER &compiler, SOURCE_FILE &sourceFile) 
			: Compiler(compiler),
			  SourceFile(sourceFile), 
			  Char(' '), 
			  Offset(0), 
			  FileSize(sourceFile.GetSize()) {}

		int NextChar() 
		{
			if (Offset >= FileSize)
				return (Char = EOF);
			return (Char = SourceFile[Offset++]);
		}

		int PeekChar()
		{
			if (Offset >= FileSize)
				return EOF;
			return SourceFile[Offset];
		}

		bool IsSpace() const
		{
			switch (Char)
			{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '\f':
			case '\v':
				return true;
			default:
				return false;
			}
		}

		bool IsAlpha() const
		{
			return ((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z'));
		}

		bool IsDigit() const
		{
			return (Char >= '0' && Char <= '9');
		}

		bool IsAlnum() const
		{
			return (IsAlpha() || IsDigit());
		}

		bool IsBinary() const
		{
			return (Char == '1' || Char == '0');
		}

		bool IsDecimal() const
		{
			return IsDigit();
		}

		bool IsHexidecimal() const
		{
			return ((Char >= 'a' && Char <= 'f') || (Char >= 'A' && Char <= 'F') || IsDigit());
		}

		bool IsOctal() const
		{
			return (Char >= '0' && Char <= '7');
		}

		void SetIdKwdKind(TOKEN &token);
		TOKEN_KIND NextToken_(TOKEN &token);
		TOKEN_KIND NextToken(TOKEN &token)
		{
			auto kind = NextToken_(token);
			token.Start--;
			token.End--;
			return kind;
		}
	};

	void LEXER::SetIdKwdKind(TOKEN & token)
	{
		static const std::pair<std::string, TOKEN_KIND> kwds[] = {
			{ "nil", TK_NIL },
			{ "true", TK_TRUE },
			{ "false", TK_FALSE },
			{ "const", TK_CONST },
			{ "static", TK_CONST },
			{ "typedef", TK_TYPEDEF },
			{ "if", TK_IF },
			{ "else", TK_ELSE },
			{ "switch", TK_SWITCH },
			{ "case", TK_CASE },
			{ "default", TK_DEFAULT },
			{ "break", TK_BREAK },
			{ "continue", TK_CONTINUE },
			{ "return", TK_RETURN },
			{ "for", TK_FOR },
			{ "while", TK_WHILE },
		};
		auto len = token.End - token.Start;
		for (auto &kwd : kwds)
		{
			if (kwd.first.length() != len)
				continue;
			for (auto i = 0u; i < len; i++)
			{
				if (SourceFile[token.Start - 1 + i] != kwd.first[i])
					goto endOfLoopIter;
			}
			token.Kind = kwd.second;
			return;
		endOfLoopIter:
			;
		}
		token.Kind = TK_IDENT;
	}

	TOKEN_KIND LEXER::NextToken_(TOKEN &token)
	{
		while (IsSpace())
			NextChar();

		token.Start = Offset;
		if (Char == '/')
		{
			NextChar();
			if (Char == '/')
			{
				do
					NextChar();
				while (Char != '\n');
				token.End = Offset;
				token.Kind = TK_COMMENT;
#if 0
				return token.Kind;
#else 
				return NextToken_(token);
#endif
			}
			else if (Char == '*')
			{
				char lastChar;
				bool terminated = false;
				do {
					lastChar = Char;
					NextChar();
					if (lastChar == '*' && Char == '/')
					{
						NextChar();
						terminated = true;
						break;
					}
				} while (Char != EOF);
				if (!terminated)
					Compiler.Error(token, "EOF inside of multi-line comment");
				token.End = Offset;
				token.Kind = TK_COMMENT;
#if 0
				return token.Kind;
#else 
				return NextToken_(token);
#endif
			}
			else
			{
				Compiler.Error(token, "stray '/' in input, expecting '//' or '/*' to begin a comment");
			}
		}
		if (IsAlpha() || Char == '_')
		{
			while (IsAlnum() || Char == '_')
				NextChar();
			token.End = Offset;
			SetIdKwdKind(token);
			return token.Kind;
		}
		else if (Char == '"' || Char == '\'')
		{
			char quote = Char;
			char lastChar;
			bool terminated = false;
			do {
				lastChar = Char;
				NextChar();
				if (Char == quote && lastChar != '\\')
				{
					NextChar();
					terminated = true;
					break;
				}
			} while (Char != EOF);
			token.End = Offset;
			if (!terminated)
				Compiler.Error(token, "EOF inside of % literal", (Char == '"') ? "string" : "char");
			else if (quote == '"')
				token.Kind = TK_STRING;
			else
				token.Kind = TK_CHAR;
			return token.Kind;
		}
		else if (IsDigit() || Char == '.')
		{
			if (Char == '0')
			{
				NextChar();
				switch (PeekChar())
				{
				case '.': // 0.xxxx
					NextChar();
					do
						NextChar();
					while (IsDecimal());
					token.End = Offset;
					token.Kind = TK_FLOAT;
					return token.Kind;
				case 'b':
				case 'B':
					NextChar();
					do
						NextChar();
					while (IsBinary());
					token.End = Offset;
					token.Kind = TK_INT;
					return token.Kind;
				case 'd':
				case 'D':
					NextChar();
					do
						NextChar();
					while (IsDecimal());
					token.End = Offset;
					token.Kind = TK_INT;
					return token.Kind;
				case 'x':
				case 'X':
					NextChar();
					do
						NextChar();
					while (IsHexidecimal());
					token.End = Offset;
					token.Kind = TK_INT;
					return token.Kind;
				case 'o':
				case 'O':
					NextChar();
					// fall-through
				default:
					while (IsOctal())
						NextChar();
					token.End = Offset;
					token.Kind = TK_INT;
					return token.Kind;
				}
			}
			else
			{
				bool isFloat = false;
				do {
					if (Char == '.')
					{
						if (isFloat)
							Compiler.Error(token, "multiple decimal points in floating-point literal");
						else
							isFloat = true;
					}
					NextChar();
				} while (IsDecimal() || Char == '.');
				token.End = Offset;
				token.Kind = TK_FLOAT;
				return token.Kind;
			}
		}
		else if (Char == '+')
		{
			NextChar();
			if (Char == '+')
			{
				NextChar();
				token.Kind = TK_INCR;
			}
			else if (Char == '=')
			{
				NextChar();
				token.Kind = TK_IADD;
			}
			else
			{
				token.Kind = TOKEN_KIND('+');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '-')
		{
			NextChar();
			if (Char == '-')
			{
				NextChar();
				token.Kind = TK_DECR;
			}
			else if (Char == '=')
			{
				NextChar();
				token.Kind = TK_ISUB;
			}
			else
			{
				token.Kind = TOKEN_KIND('-');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '!')
		{
			NextChar();
			if (Char == '=')
			{
				NextChar();
				token.Kind = TK_NE;
			}
			else
			{
				token.Kind = TOKEN_KIND('!');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '=')
		{
			NextChar();
			if (Char == '=')
			{
				NextChar();
				token.Kind = TK_EQ;
			}
			else
			{
				token.Kind = TOKEN_KIND('=');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '<')
		{
			NextChar();
			if (Char == '=')
			{
				NextChar();
				token.Kind = TK_LE;
			}
			else if (Char == '<')
			{
				NextChar();
				if (Char == '=')
				{
					NextChar();
					token.Kind = TK_ILEFT;
				}
				else
				{
					token.Kind = TK_LEFT;
				}
			}
			else
			{
				token.Kind = TOKEN_KIND('<');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '>')
		{
			NextChar();
			if (Char == '=')
			{
				NextChar();
				token.Kind = TK_GE;
			}
			else if (Char == '>')
			{
				NextChar();
				if (Char == '=')
				{
					NextChar();
					token.Kind = TK_IRIGHT;
				}
				else
				{
					token.Kind = TK_RIGHT;
				}
			}
			else
			{
				token.Kind = TOKEN_KIND('>');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '&')
		{
			NextChar();
			if (Char == '&')
			{
				NextChar();
				token.Kind = TK_AND;
			}
			else if (Char == '=')
			{
				NextChar();
				token.Kind = TK_IAND;
			}
			else
			{
				token.Kind = TOKEN_KIND('&');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '|')
		{
			NextChar();
			if (Char == '|')
			{
				NextChar();
				token.Kind = TK_OR;
			}
			else if (Char == '=')
			{
				NextChar();
				token.Kind = TK_IOR;
			}
			else
			{
				token.Kind = TOKEN_KIND('|');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == '^')
		{
			NextChar();
			if (Char == '=')
			{
				NextChar();
				token.Kind = TK_IXOR;
			}
			else
			{
				token.Kind = TOKEN_KIND('^');
			}
			token.End = Offset;
			return token.Kind;
		}
		else if (Char == EOF)
		{
			token.End = Offset;
			token.Kind = TK_EOF;
			return token.Kind;
		}
		else
		{
			char c = Char;
			NextChar();
			token.End = Offset;
			token.Kind = static_cast<TOKEN_KIND>(c);
			return token.Kind;
		}
	}

	void TOKEN::GetText(std::string &text) const
	{
		text.clear();
		text.reserve(End - Start);
		for (auto i = Start; i < End; i++)
			text += File[i];
	}

	bool TokenizeFile(COMPILER &compiler, SOURCE_FILE &sourceFile, TOKEN_LIST &tokenList)
	{
		bool ok = true;
		LEXER lexer(compiler, sourceFile);
		TOKEN token(sourceFile);

		while (lexer.NextToken(token) != TK_EOF)
		{
			if (token.Kind != TK_ERROR)
				tokenList.push_back(token);
			else
			{
				ok = false;
				break;
			}
		}

		return ok;
	}

	const char *TokenKindName(TOKEN_KIND kind)
	{
		switch (kind)
		{
		case TK_ERROR:     return "ERROR";
		case TK_EOF:       return "EOF";
		case TK_LPAREN:    return "LPAREN";
		case TK_RPAREN:    return "RPAREN";
		case TK_LBRACE:    return "LBRACE";
		case TK_RBRACE:    return "RBRACE";
		case TK_LBRACKET:  return "LBRACKET";
		case TK_RBRACKET:  return "RBRACKET";
		case TK_COLON:     return "COLON";
		case TK_SEMICOLON: return "SEMICOLON";
		case TK_COMMA:     return "COMMA";
		case TK_PLUS:      return "PLUS_OP";
		case TK_MINUS:     return "MINUS_OP";
		case TK_MUL:       return "MUL_OP";
		case TK_DIV:       return "DIV_OP";
		case TK_MOD:       return "MOD_OP";
		case TK_BAND:      return "BAND_OP";
		case TK_BOR:       return "BOR_OP";
		case TK_BXOR:      return "BXOR_OP";
		case TK_LT:        return "LT_OP";
		case TK_GT:        return "GT_OP";
		case TK_IDENT:     return "IDENT";
		case TK_CONST:     return "CONST_KWD";
		case TK_STATIC:    return "STATIC_KWD";
		case TK_TYPEDEF:   return "TYPEDEF_KWD";
		case TK_IF:        return "IF_KWD";
		case TK_ELSE:      return "ELSE_KWD";
		case TK_SWITCH:    return "SWITCH_KWD";
		case TK_CASE:      return "CASE_KWD";
		case TK_DEFAULT:   return "DEFAULT_KWD";
		case TK_BREAK:     return "BREAK_KWD";
		case TK_CONTINUE:  return "CONTINUE_KWD";
		case TK_RETURN:    return "RETURN_KWD";
		case TK_FOR:       return "FOR_KWD";
		case TK_WHILE:     return "WHILE_KWD";
		case TK_COMMENT:   return "COMMENT";
		case TK_NIL:       return "NIL_LIT";
		case TK_TRUE:      return "TRUE_LIT";
		case TK_FALSE:     return "FALSE_LIT";
		case TK_INT:       return "INT_LIT";
		case TK_FLOAT:     return "FLOAT_LIT";
		case TK_STRING:    return "STRING_LIT";
		case TK_CHAR:      return "CHAR_LIT";
		case TK_INCR:      return "INC_OP";
		case TK_DECR:      return "DEC_OP";
		case TK_LEFT:      return "LEFT_OP";
		case TK_RIGHT:     return "RIGHT_OP";
		case TK_EQ:        return "EQ_OP";
		case TK_NE:        return "NE_OP";
		case TK_LE:        return "LE_OP";
		case TK_GE:        return "GE_OP";
		case TK_AND:       return "AND_OP";
		case TK_OR:        return "OR_OP";
		case TK_IADD:      return "IADD_OP";
		case TK_ISUB:      return "ISUB_OP";
		case TK_IMUL:      return "IMUL_OP";
		case TK_IDIV:      return "IDIV_OP";
		case TK_IMOD:      return "IMOD_OP";
		case TK_ILEFT:     return "ILEFT_OP";
		case TK_IRIGHT:    return "IRIGHT_OP";
		case TK_IAND:      return "IAND_OP";
		case TK_IOR:       return "IOR_OP";
		case TK_IXOR:      return "IXOR_OP";
		default:           return "UNKNOWN";
		}
	}

}