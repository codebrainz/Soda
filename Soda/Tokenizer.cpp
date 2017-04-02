#include "Compiler.h"
#include "SourceFile.h"
#include "Tokenizer.h"
#include <iostream>

namespace Soda
{
    struct Lexer
    {
        Compiler &compiler;
        SourceFile &sourceFile;
        int chr;
        size_t offset;
        size_t fileSize;

        Lexer(Compiler &compiler, SourceFile &sourceFile)
            : compiler(compiler)
            , sourceFile(sourceFile)
            , chr(' ')
            , offset(0)
            , fileSize(sourceFile.getSize())
        {
        }

        int nextChar()
        {
            if (offset >= fileSize)
                return (chr = EOF);
            return (chr = sourceFile[offset++]);
        }

        int peekChar()
        {
            if (offset >= fileSize)
                return EOF;
            return sourceFile[offset];
        }

        bool isSpace() const
        {
            switch (chr) {
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

        bool isAlpha() const
        {
            return ((chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z'));
        }

        bool isDigit() const
        {
            return (chr >= '0' && chr <= '9');
        }

        bool isAlnum() const
        {
            return (isAlpha() || isDigit());
        }

        bool isBinary() const
        {
            return (chr == '1' || chr == '0');
        }

        bool isDecimal() const
        {
            return isDigit();
        }

        bool isHexidecimal() const
        {
            return ((chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F')
                || isDigit());
        }

        bool isOctal() const
        {
            return (chr >= '0' && chr <= '7');
        }

        void setIdKwdKind(Token &token);
        TokenKind nextToken_(Token &token);
        TokenKind nextToken(Token &token)
        {
            auto kind = nextToken_(token);
            token.start--;
            token.end--;
            return kind;
        }
    };

    void Lexer::setIdKwdKind(Token &token)
    {
        static const std::pair< std::string, TokenKind > kwds[]
            = { { "nil", TK_NIL }, { "true", TK_TRUE }, { "false", TK_FALSE },
                { "const", TK_CONST }, { "static", TK_CONST },
                { "typedef", TK_TYPEDEF }, { "if", TK_IF }, { "else", TK_ELSE },
                { "switch", TK_SWITCH }, { "case", TK_CASE },
                { "default", TK_DEFAULT }, { "break", TK_BREAK },
                { "continue", TK_CONTINUE }, { "goto", TK_GOTO },
                { "return", TK_RETURN }, { "for", TK_FOR },
                { "while", TK_WHILE }, { "struct", TK_STRUCT },
                { "enum", TK_ENUM }, { "namespace", TK_NAMESPACE },
                { "using", TK_USING } };
        auto len = token.end - token.start;
        for (auto &kwd : kwds) {
            if (kwd.first.length() != len)
                continue;
            for (auto i = 0u; i < len; i++) {
                if (sourceFile[token.start - 1 + i] != kwd.first[i])
                    goto endOfLoopIter;
            }
            token.kind = kwd.second;
            return;
        endOfLoopIter:;
        }
        token.kind = TK_IDENT;
    }

    TokenKind Lexer::nextToken_(Token &token)
    {
        while (isSpace())
            nextChar();

        token.start = offset;
        if (chr == '/') {
            nextChar();
            if (chr == '/') {
                do
                    nextChar();
                while (chr != '\n');
                token.end = offset;
                token.kind = TK_COMMENT;
#if 0
                return token.kind;
#else
                return nextToken_(token);
#endif
            } else if (chr == '*') {
                char lastChar;
                bool terminated = false;
                do {
                    lastChar = chr;
                    nextChar();
                    if (lastChar == '*' && chr == '/') {
                        nextChar();
                        terminated = true;
                        break;
                    }
                } while (chr != EOF);
                if (!terminated)
                    compiler.error(token, "EOF inside of multi-line comment");
                token.end = offset;
                token.kind = TK_COMMENT;
#if 0
                return token.kind;
#else
                return nextToken_(token);
#endif
            } else {
                compiler.error(token, "stray '/' in input, expecting '//' or "
                                      "'/*' to begin a comment");
            }
        }
        if (isAlpha() || chr == '_') {
            while (isAlnum() || chr == '_')
                nextChar();
            token.end = offset;
            setIdKwdKind(token);
            return token.kind;
        } else if (chr == '"' || chr == '\'') {
            char quote = chr;
            char lastChar;
            bool terminated = false;
            do {
                lastChar = chr;
                nextChar();
                if (chr == quote && lastChar != '\\') {
                    nextChar();
                    terminated = true;
                    break;
                }
            } while (chr != EOF);
            token.end = offset;
            if (!terminated)
                compiler.error(token, "EOF inside of % literal",
                    (chr == '"') ? "string" : "char");
            else if (quote == '"')
                token.kind = TK_STRING;
            else
                token.kind = TK_CHAR;
            return token.kind;
        } else if (isDigit() || chr == '.') {
            if (chr == '0') {
                nextChar();
                switch (peekChar()) {
                case '.': // 0.xxxx
                    nextChar();
                    do
                        nextChar();
                    while (isDecimal());
                    token.end = offset;
                    token.kind = TK_FLOAT;
                    return token.kind;
                case 'b':
                case 'B':
                    nextChar();
                    do
                        nextChar();
                    while (isBinary());
                    token.end = offset;
                    token.kind = TK_INT;
                    return token.kind;
                case 'd':
                case 'D':
                    nextChar();
                    do
                        nextChar();
                    while (isDecimal());
                    token.end = offset;
                    token.kind = TK_INT;
                    return token.kind;
                case 'x':
                case 'X':
                    nextChar();
                    do
                        nextChar();
                    while (isHexidecimal());
                    token.end = offset;
                    token.kind = TK_INT;
                    return token.kind;
                case 'o':
                case 'O':
                    nextChar();
                // fall-through
                default:
                    while (isOctal())
                        nextChar();
                    token.end = offset;
                    token.kind = TK_INT;
                    return token.kind;
                }
            } else {
                bool isFloat = false;
                do {
                    if (chr == '.') {
                        if (isFloat)
                            compiler.error(token, "multiple decimal points in "
                                                  "floating-point literal");
                        else
                            isFloat = true;
                    }
                    nextChar();
                } while (isDecimal() || chr == '.');
                token.end = offset;
                if (isFloat)
                    token.kind = TK_FLOAT;
                else
                    token.kind = TK_INT;
                return token.kind;
            }
        } else if (chr == '+') {
            nextChar();
            if (chr == '+') {
                nextChar();
                token.kind = TK_INCR;
            } else if (chr == '=') {
                nextChar();
                token.kind = TK_IADD;
            } else {
                token.kind = TokenKind('+');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '-') {
            nextChar();
            if (chr == '-') {
                nextChar();
                token.kind = TK_DECR;
            } else if (chr == '=') {
                nextChar();
                token.kind = TK_ISUB;
            } else {
                token.kind = TokenKind('-');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '!') {
            nextChar();
            if (chr == '=') {
                nextChar();
                token.kind = TK_NE;
            } else {
                token.kind = TokenKind('!');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '=') {
            nextChar();
            if (chr == '=') {
                nextChar();
                token.kind = TK_EQ;
            } else {
                token.kind = TokenKind('=');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '<') {
            nextChar();
            if (chr == '=') {
                nextChar();
                token.kind = TK_LE;
            } else if (chr == '<') {
                nextChar();
                if (chr == '=') {
                    nextChar();
                    token.kind = TK_ILEFT;
                } else {
                    token.kind = TK_LEFT;
                }
            } else {
                token.kind = TokenKind('<');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '>') {
            nextChar();
            if (chr == '=') {
                nextChar();
                token.kind = TK_GE;
            } else if (chr == '>') {
                nextChar();
                if (chr == '=') {
                    nextChar();
                    token.kind = TK_IRIGHT;
                } else {
                    token.kind = TK_RIGHT;
                }
            } else {
                token.kind = TokenKind('>');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '&') {
            nextChar();
            if (chr == '&') {
                nextChar();
                token.kind = TK_AND;
            } else if (chr == '=') {
                nextChar();
                token.kind = TK_IAND;
            } else {
                token.kind = TokenKind('&');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '|') {
            nextChar();
            if (chr == '|') {
                nextChar();
                token.kind = TK_OR;
            } else if (chr == '=') {
                nextChar();
                token.kind = TK_IOR;
            } else {
                token.kind = TokenKind('|');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == '^') {
            nextChar();
            if (chr == '=') {
                nextChar();
                token.kind = TK_IXOR;
            } else {
                token.kind = TokenKind('^');
            }
            token.end = offset;
            return token.kind;
        } else if (chr == EOF) {
            token.end = offset;
            token.kind = TK_EOF;
            return token.kind;
        } else {
            char c = chr;
            nextChar();
            token.end = offset;
            token.kind = static_cast< TokenKind >(c);
            return token.kind;
        }
    }

    void Token::getText(std::string &text) const
    {
        text.clear();
        text.reserve(end - start);
        for (auto i = start; i < end; i++)
            text += file[i];
    }

    bool tokenizeFile(
        Compiler &compiler, SourceFile &sourceFile, TokenList &tokenList)
    {
        bool ok = true;
        Lexer lexer(compiler, sourceFile);
        Token token(sourceFile);

        while (lexer.nextToken(token) != TK_EOF) {
            if (token.kind != TK_ERROR)
                tokenList.push_back(token);
            else {
                ok = false;
                break;
            }
        }

        return ok;
    }

    const char *tokenKindName(TokenKind kind)
    {
        switch (kind) {
        case TK_ERROR:
            return "ERROR";
        case TK_EOF:
            return "EOF";
        case TK_LPAREN:
            return "LPAREN";
        case TK_RPAREN:
            return "RPAREN";
        case TK_LBRACE:
            return "LBRACE";
        case TK_RBRACE:
            return "RBRACE";
        case TK_LBRACKET:
            return "LBRACKET";
        case TK_RBRACKET:
            return "RBRACKET";
        case TK_COLON:
            return "COLON";
        case TK_SEMICOLON:
            return "SEMICOLON";
        case TK_COMMA:
            return "COMMA";
        case TK_PLUS:
            return "PLUS_OP";
        case TK_MINUS:
            return "MINUS_OP";
        case TK_MUL:
            return "MUL_OP";
        case TK_DIV:
            return "DIV_OP";
        case TK_MOD:
            return "MOD_OP";
        case TK_BAND:
            return "BAND_OP";
        case TK_BOR:
            return "BOR_OP";
        case TK_BXOR:
            return "BXOR_OP";
        case TK_LT:
            return "LT_OP";
        case TK_GT:
            return "GT_OP";
        case TK_IDENT:
            return "IDENT";
        case TK_CONST:
            return "CONST_KWD";
        case TK_STATIC:
            return "STATIC_KWD";
        case TK_TYPEDEF:
            return "TYPEDEF_KWD";
        case TK_IF:
            return "IF_KWD";
        case TK_ELSE:
            return "ELSE_KWD";
        case TK_SWITCH:
            return "SWITCH_KWD";
        case TK_CASE:
            return "CASE_KWD";
        case TK_DEFAULT:
            return "DEFAULT_KWD";
        case TK_BREAK:
            return "BREAK_KWD";
        case TK_CONTINUE:
            return "CONTINUE_KWD";
        case TK_GOTO:
            return "GOTO_KWD";
        case TK_RETURN:
            return "RETURN_KWD";
        case TK_FOR:
            return "FOR_KWD";
        case TK_WHILE:
            return "WHILE_KWD";
        case TK_STRUCT:
            return "STRUCT_KWD";
        case TK_ENUM:
            return "ENUM_KWD";
        case TK_NAMESPACE:
            return "NAMESPACE_KWD";
        case TK_USING:
            return "USING_KWD";
        case TK_COMMENT:
            return "COMMENT";
        case TK_NIL:
            return "NIL_LIT";
        case TK_TRUE:
            return "TRUE_LIT";
        case TK_FALSE:
            return "FALSE_LIT";
        case TK_INT:
            return "INT_LIT";
        case TK_FLOAT:
            return "FLOAT_LIT";
        case TK_STRING:
            return "STRING_LIT";
        case TK_CHAR:
            return "CHAR_LIT";
        case TK_INCR:
            return "INC_OP";
        case TK_DECR:
            return "DEC_OP";
        case TK_LEFT:
            return "LEFT_OP";
        case TK_RIGHT:
            return "RIGHT_OP";
        case TK_EQ:
            return "EQ_OP";
        case TK_NE:
            return "NE_OP";
        case TK_LE:
            return "LE_OP";
        case TK_GE:
            return "GE_OP";
        case TK_AND:
            return "AND_OP";
        case TK_OR:
            return "OR_OP";
        case TK_IADD:
            return "IADD_OP";
        case TK_ISUB:
            return "ISUB_OP";
        case TK_IMUL:
            return "IMUL_OP";
        case TK_IDIV:
            return "IDIV_OP";
        case TK_IMOD:
            return "IMOD_OP";
        case TK_ILEFT:
            return "ILEFT_OP";
        case TK_IRIGHT:
            return "IRIGHT_OP";
        case TK_IAND:
            return "IAND_OP";
        case TK_IOR:
            return "IOR_OP";
        case TK_IXOR:
            return "IXOR_OP";
        default:
            return "UNKNOWN";
        }
    }
}
