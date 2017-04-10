#include "Ast.h"
#include "Compiler.h"
#include "Parser.h"
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
            BackTracker(Parser &p)
                : parser(p)
                , cancelled(false)
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
                if (!cancelled) {
                    parser.dropOffset();
                    cancelled = true;
                }
            }
        };

        Compiler &compiler;
        TokenList &tokens;
        size_t offset;
        std::stack< size_t > offsetStack;
        std::stack< Token * > tokenStack;
        std::vector< AstAttributePtr > attributes;

        Parser(Compiler &compiler, TokenList &tokenList)
            : compiler(compiler)
            , tokens(tokenList)
            , offset(0)
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
            if (tokens[offset].kind == kind) {
                offset++;
                if (offset >= tokens.size())
                    offset = tokens.size() - 1;
                return true;
            }
            return false;
        }

        bool accept(int kind)
        {
            return accept(static_cast< TokenKind >(kind));
        }

#ifdef NDEBUG
        bool expect(TokenKind kind)
        {
            if (!accept(kind)) {
                compiler.error(tokens[offset],
                    "unexpected token '%', expecting '%'",
                    tokens[offset].getKindName(), tokenKindName(kind));
                return false;
            }
            return true;
        }

        bool expect(int kind)
        {
            return expect(static_cast< TokenKind >(kind));
        }
#else
        bool expect_(const char *file, unsigned int line, TokenKind kind)
        {
            if (!accept(kind)) {
                compiler.error(tokens[offset],
                    "unexpected token '%', expecting '%' [%:%]",
                    tokens[offset].getKindName(), tokenKindName(kind), file,
                    line);
                return false;
            }
            return true;
        }
#define expect(kind) expect_(__FILE__, __LINE__, TokenKind(kind))
#endif

        template < class NodePtrT >
        bool isIgnored(NodePtrT &node)
        {
            if (!node)
                return true;
            switch (node->kind) {
            case NK_EMPTY_DECL:
            case NK_EMPTY_STMT:
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

        unsigned long long parseIntLiteral(
            Token *start, const std::string &text)
        {
            int base = 0;
            size_t offset = 0;
            if (text[0] == '0' && text.size() > 1) {
                switch (text[2]) {
                case 'b':
                case 'B':
                    base = 2;
                    offset = 2;
                case 'd':
                case 'D':
                    base = 10;
                    offset = 2;
                case 'o':
                case 'O':
                    base = 8;
                    offset = 2;
                case 'x':
                case 'X':
                    base = 16;
                    offset = 2;
                default:
                    base = 8;
                    offset = 0;
                    break;
                }
            } else {
                base = 10;
                offset = 0;
            }
            try {
                return std::stoull(text.substr(offset), 0, base);
            } catch (std::invalid_argument &e) {
                std::string msg
                    = "failed to parse int '" + text + "': " + e.what();
                compiler.error(*start, "%", msg);
                return 0;
            } catch (std::out_of_range &e) {
                std::string msg = "integer literal '" + text
                    + "' out of range?: " + e.what();
                compiler.error(*start, "%", msg);
                return 0;
            }
        }

        long double parseFloatLiteral(Token *start, const std::string &text)
        {
            try {
                return std::stold(text, 0);
            } catch (std::invalid_argument &e) {
                std::string msg
                    = "failed to parse float '" + text + "': " + e.what();
                compiler.error(*start, "%", msg);
                return 0.0;
            } catch (std::out_of_range &e) {
                std::string msg = "floating-point literal '" + text
                    + "' out of range: " + e.what();
                compiler.error(*start, "%", msg);
                return 0.0;
            }

            return 0.0;
        }

        //> nil_lit: NIL
        //>        ;
        AstExprPtr parseNilLit()
        {
            auto startToken = currentToken();
            if (!expect(TK_NIL))
                return nullptr;
            return std::make_unique< AstNil >(startToken, startToken);
        }

        //> bool_lit: TRUE
        //>         | FALSE
        //>         ;
        AstExprPtr parseBoolLit()
        {
            auto startToken = currentToken();
            if (accept(TK_TRUE)) {
                return std::make_unique< AstBool >(
                    true, startToken, startToken);
            } else if (accept(TK_FALSE)) {
                return std::make_unique< AstBool >(
                    false, startToken, startToken);
            } else {
                return nullptr;
            }
        }

        //> int_lit: INT
        //>        ;
        AstExprPtr parseIntLit()
        {
            auto startToken = currentToken();
            auto text = tokenText();
            if (!expect(TK_INT))
                return nullptr;
            auto val = parseIntLiteral(startToken, text);
            return std::make_unique< AstInt >(val, startToken, startToken);
        }

        //> float_lit: FLOAT
        //>          ;
        AstExprPtr parseFloatLit()
        {
            auto startToken = currentToken();
            auto text = tokenText();
            if (!expect(TK_FLOAT))
                return nullptr;
            auto val = parseFloatLiteral(startToken, text);
            return std::make_unique< AstFloat >(val, startToken, startToken);
        }

        //> primary_expr: nil_lit
        //>             | bool_lit
        //>             | int_lit
        //>             | float_lit
        //>             | CHAR
        //>             | STRING
        //>             | IDENT
        //>             | '(' expr ')'
        //>             ;
        AstExprPtr parsePrimaryExpr()
        {
            auto startToken = currentToken();
            auto text = tokenText();
            auto kind = currentToken()->kind;
            if (kind == TK_NIL)
                return parseNilLit();
            else if (kind == TK_TRUE || kind == TK_FALSE)
                return parseBoolLit();
            else if (kind == TK_INT)
                return parseIntLit();
            else if (kind == TK_FLOAT)
                return parseFloatLit();
            else if (accept(TK_CHAR))
                return std::make_unique< AstChar >(
                    text, startToken, startToken);
            else if (accept(TK_STRING))
                return std::make_unique< AstString >(
                    text, startToken, startToken);
            else if (accept(TK_IDENT))
                return std::make_unique< AstIdentifier >(
                    text, startToken, startToken);
            else if (accept('(')) {
                auto expr = parseExpr();
                auto endToken = currentToken();
                if (!expect(')'))
                    return nullptr;
                expr->start = startToken;
                expr->end = endToken;
                return expr;
            }
            // TODO: function expression
            else {
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
            while (true) {
                if (accept('[')) {
                    auto index = parseExpr();
                    auto endToken = currentToken();
                    if (!expect(']'))
                        return nullptr;
                    expr = std::make_unique< AstIndexExpr >(std::move(expr),
                        std::move(index), startToken, endToken);
                    startToken = currentToken();
                } else if (accept('(')) {
                    AstExprList args;
                    auto endToken = currentToken();
                    if (!accept(')')) {
                        while (true) {
                            if (auto arg = parseExpr()) {
                                args.push_back(std::move(arg));
                                if (!accept(','))
                                    break;
                            } else
                                break;
                        }
                        endToken = currentToken();
                        if (!expect(')'))
                            return nullptr;
                    }
                    expr = std::make_unique< AstCallExpr >(
                        std::move(expr), std::move(args), startToken, endToken);
                    startToken = currentToken();
                } else if (accept('.')) {
                    auto member = parseExpr();
                    auto endToken = member->end;
                    expr = std::make_unique< AstMemberExpr >(std::move(expr),
                        std::move(member), startToken, endToken);
                    startToken = currentToken();
                } else {
                    auto endToken = currentToken();
                    if (accept(TK_INCR))
                        expr = std::make_unique< AstUnary >(
                            UOP_POSTINC, std::move(expr), startToken, endToken);
                    else if (accept(TK_DECR))
                        expr = std::make_unique< AstUnary >(
                            UOP_POSTDEC, std::move(expr), startToken, endToken);
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
            if (accept('*')) {
                auto expr = parsePrefixExpr();
                auto endToken = expr->end;
                return std::make_unique< AstUnary >(
                    UOP_DEREF, std::move(expr), startToken, endToken);
            } else if (accept('~')) {
                auto expr = parsePrefixExpr();
                auto endToken = expr->end;
                return std::make_unique< AstUnary >(
                    UOP_COMPL, std::move(expr), startToken, endToken);
            } else if (accept('!')) {
                auto expr = parsePrefixExpr();
                auto endToken = expr->end;
                return std::make_unique< AstUnary >(
                    UOP_NOT, std::move(expr), startToken, endToken);
            } else if (accept('+')) {
                auto expr = parsePrefixExpr();
                auto endToken = expr->end;
                return std::make_unique< AstUnary >(
                    UOP_POS, std::move(expr), startToken, endToken);
            } else if (accept('-')) {
                auto expr = parsePrefixExpr();
                auto endToken = expr->end;
                return std::make_unique< AstUnary >(
                    UOP_NEG, std::move(expr), startToken, endToken);
            } else if (accept(TK_INCR)) {
                auto expr = parsePrefixExpr();
                auto endToken = expr->end;
                return std::make_unique< AstUnary >(
                    UOP_PREINC, std::move(expr), startToken, endToken);
            } else if (accept(TK_DECR)) {
                auto expr = parsePrefixExpr();
                auto endToken = expr->end;
                return std::make_unique< AstUnary >(
                    UOP_PREDEC, std::move(expr), startToken, endToken);
            } else {
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
                if (accept('(')) {
                    if (auto typeRef = parseTypeRef()) {
                        if (accept(')')) {
                            if (auto expr = parsePrefixExpr()) {
                                auto endToken = currentToken();
                                bt.cancel();
                                return std::make_unique< AstCast >(
                                    std::move(typeRef), std::move(expr),
                                    startToken, endToken);
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
            while (true) {
                if (accept('*')) {
                    auto rhs = parseCastExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_MUL, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept('/')) {
                    auto rhs = parseCastExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_DIV, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept('%')) {
                    auto rhs = parseCastExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_MOD, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept('+')) {
                    auto rhs = parseMultiplicativeExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_ADD, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept('-')) {
                    auto rhs = parseMultiplicativeExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_SUB, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept(TK_LEFT)) {
                    auto rhs = parseAdditiveExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_LSHIFT,
                        std::move(lhs), std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept(TK_RIGHT)) {
                    auto rhs = parseAdditiveExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_RSHIFT,
                        std::move(lhs), std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept('<')) {
                    auto rhs = parseAdditiveExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_LT, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept('>')) {
                    auto rhs = parseAdditiveExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_GT, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept(TK_LE)) {
                    auto rhs = parseAdditiveExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_LE, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept(TK_GE)) {
                    auto rhs = parseAdditiveExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_GE, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept(TK_EQ)) {
                    auto rhs = parseRelationalExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_EQ, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else if (accept(TK_NE)) {
                    auto rhs = parseRelationalExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_NE, std::move(lhs),
                        std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept('&')) {
                    auto rhs = parseEqualityExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_BIT_AND,
                        std::move(lhs), std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept('^')) {
                    auto rhs = parseAndExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_BIT_XOR,
                        std::move(lhs), std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept('|')) {
                    auto rhs = parseXorExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_BIT_OR,
                        std::move(lhs), std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept(TK_AND)) {
                    auto rhs = parseOrExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_LOG_AND,
                        std::move(lhs), std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            while (true) {
                if (accept(TK_OR)) {
                    auto rhs = parseAndExpr();
                    auto endToken = rhs->end;
                    lhs = std::make_unique< AstBinary >(BOP_LOG_OR,
                        std::move(lhs), std::move(rhs), startToken, endToken);
                    startToken = currentToken();
                } else {
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
            if (accept('?')) {
                auto thenExpr = parseLogicalOrExpr();
                if (!expect(':'))
                    return nullptr;
                auto elseExpr = parseLogicalOrExpr();
                auto endToken = elseExpr->end;
                return std::make_unique< AstIfExpr >(std::move(condExpr),
                    std::move(thenExpr), std::move(elseExpr), startToken,
                    endToken);
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
            if (accept('=')) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_ASSIGN, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_IMUL)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IMUL, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_IDIV)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IDIV, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_IMOD)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IMOD, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_IADD)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IADD, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_ISUB)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_ISUB, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_ILEFT)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_ILSHIFT,
                    std::move(lhs), std::move(rhs), startToken, endToken);
            } else if (accept(TK_IRIGHT)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IRSHIFT,
                    std::move(lhs), std::move(rhs), startToken, endToken);
            } else if (accept(TK_IAND)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IAND, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_IOR)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IOR, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else if (accept(TK_IXOR)) {
                auto rhs = parseIfExpr();
                auto endToken = rhs->end;
                return std::make_unique< AstBinary >(BOP_IXOR, std::move(lhs),
                    std::move(rhs), startToken, endToken);
            } else {
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
            if (auto expr = parseExpr()) {
                auto endToken = currentToken();
                if (!expect(';'))
                    return nullptr;
                return std::make_unique< AstExprStmt >(
                    std::move(expr), startToken, endToken);
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
            return std::make_unique< AstBlockStmt >(
                std::move(stmtList), startToken, endToken);
        }

        //> return_stmt: RETURN expr? ';'
        //>            ;
        AstStmtPtr parseReturnStmt()
        {
            auto startToken = currentToken();
            if (!expect(TK_RETURN))
                return nullptr;
            auto endToken = currentToken();
            if (!accept(';')) {
                auto expr = parseExpr();
                assert(expr);
                endToken = expr->end;
                if (!expect(';'))
                    return nullptr;
                return std::make_unique< AstReturnStmt >(
                    std::move(expr), startToken, endToken);
            } else {
                return std::make_unique< AstReturnStmt >(startToken, endToken);
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
            return std::make_unique< AstBreakStmt >(startToken, endToken);
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
            return std::make_unique< AstContinueStmt >(startToken, endToken);
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
            return std::make_unique< AstGotoStmt >(
                std::move(label), startToken, endToken);
        }

        //> variable_init: var_or_func_prefix '=' expr ';'
        //>              ;
        AstDeclPtr parseVariableInit()
        {
            BackTracker bt(*this);
            auto startToken = currentToken();
            DeclFlags flags = DF_NONE;
            AstTypeRefPtr typeRef;
            std::string name;
            if (!parseVarOrFuncPrefix(flags, typeRef, name))
                return nullptr;
            if (!expect('='))
                return nullptr;
            auto initExpr = parseExpr();
            if (!initExpr)
                return nullptr;
            auto endToken = initExpr->end;
            auto var = std::make_unique< AstVarDecl >(std::move(name),
                std::move(typeRef), std::move(initExpr), startToken, endToken);
            var->flags = DeclFlags(var->flags | flags);
            bt.cancel();
            return std::move(var);
        }

        //> var_decl_or_expr: variable_init
        //>                 | expr
        //>                 ;
        AstNodePtr parseVarDeclOrExpr()
        {
            if (auto var = parseVariableInit())
                return var;
            return parseExpr();
        }

        //> if_stmt: IF '(' var_decl_or_expr ')' stmt
        //>        | IF '(' var_decl_or_expr ')' stmt ELSE stmt
        //>        ;
        AstStmtPtr parseIfStmt()
        {
            auto startToken = currentToken();
            if (!expect(TK_IF))
                return nullptr;
            if (!expect('('))
                return nullptr;
            auto testNode = parseVarDeclOrExpr();
            if (!testNode)
                return nullptr;
            if (!expect(')'))
                return nullptr;
            auto thenStmt = parseLocalStmt();
            auto endToken = thenStmt->end;
            AstStmtPtr elseStmt;
            if (accept(TK_ELSE)) {
                elseStmt = parseLocalStmt();
                endToken = elseStmt->end;
            }
            return std::make_unique< AstIfStmt >(std::move(testNode),
                std::move(thenStmt), std::move(elseStmt), startToken, endToken);
        }

        //> case_stmt: CASE expr ':' stmt* ';'
        //>          | DEFAULT ':' stmt* ';'
        //>          ;
        AstStmtPtr parseCaseStmt()
        {
            auto startToken = currentToken();
            AstExprPtr expr;
            AstStmtList stmts;
            if (accept(TK_CASE)) {
                expr = parseExpr();
                if (!expr)
                    return nullptr;
            } else if (accept(TK_DEFAULT)) {
                ;
            } else {
                return nullptr;
            }
            auto endToken = currentToken();
            if (!expect(':'))
                return nullptr;
            while (true) {
                if (auto stmt = parseLocalStmt()) {
                    if (!isIgnored(stmt))
                        stmts.push_back(std::move(stmt));
                } else {
                    break;
                }
            }
            if (!stmts.empty())
                endToken = stmts.back()->end;
            return std::make_unique< AstCaseStmt >(
                std::move(expr), std::move(stmts), startToken, endToken);
        }

        //> switch_stmt: SWITCH '(' var_decl_or_expr ')' '{' case_stmt* '}'
        //>            ;
        AstStmtPtr parseSwitchStmt()
        {
            auto startToken = currentToken();
            if (!expect(TK_SWITCH))
                return nullptr;
            if (!expect('('))
                return nullptr;
            auto testNode = parseVarDeclOrExpr();
            if (!testNode)
                return nullptr;
            if (!expect(')'))
                return nullptr;
            if (!expect('{'))
                return nullptr;
            AstStmtList cases;
            auto endToken = currentToken();
            if (!accept('}')) {
                while (true) {
                    if (auto case_ = parseCaseStmt()) {
                        cases.push_back(std::move(case_));
                    } else {
                        break;
                    }
                }
                endToken = currentToken();
                if (!expect('}'))
                    return nullptr;
            }
            return std::make_unique< AstSwitchStmt >(
                std::move(testNode), std::move(cases), startToken, endToken);
        }

        //> empty_stmt: ';'
        //>           ;
        AstStmtPtr parseEmptyStmt()
        {
            auto startToken = currentToken();
            if (!expect(';'))
                return nullptr;
            return std::make_unique< AstEmptyStmt >(startToken, startToken);
        }

        //> for_init: empty_stmt
        //>         | variable_init ';'
        //>         | expr_stmt
        //>         ;
        AstStmtPtr parseForInit()
        {
            if (currentToken()->kind == ';')
                return parseEmptyStmt();
            else if (auto var = parseVariableInit()) {
                if (!expect(';'))
                    return nullptr;
                return var;
            }
            return parseExprStmt();
        }

        //> for_test: empty_stmt
        //>         | expr_stmt
        //>         ;
        AstStmtPtr parseForTest()
        {
            if (currentToken()->kind == ';')
                return parseEmptyStmt();
            return parseExprStmt();
        }

        //> for_stmt: FOR '(' for_init expr_stmt expr? ')' stmt
        //>         ;
        AstStmtPtr parseForStmt()
        {
            auto startToken = currentToken();
            if (!expect(TK_FOR))
                return nullptr;
            if (!expect('('))
                return nullptr;
            auto initNode = parseForInit();
            if (!initNode)
                return nullptr;
            else if (initNode->kind == NK_EMPTY_STMT)
                initNode.reset(nullptr);
            auto testStmt = parseForTest();
            if (!testStmt)
                return nullptr;
            else if (testStmt->kind == NK_EMPTY_STMT)
                testStmt.reset(nullptr);
            else if (testStmt->kind != NK_EXPR_STMT) {
                compiler.error(
                    *testStmt, "for loop test must be an expression");
                return nullptr;
            }
            auto testExpr = testStmt
                ? std::move(static_cast< AstExprStmt * >(testStmt.get())->expr)
                : nullptr;
            auto incrExpr = parseExpr();
            if (!expect(')'))
                return nullptr;
            auto stmt = parseLocalStmt();
            if (!stmt)
                return nullptr;
            auto endToken = stmt->end;
            return std::make_unique< AstForStmt >(std::move(initNode),
                std::move(testExpr), std::move(incrExpr), std::move(stmt),
                startToken, endToken);
        }

        //> do_stmt: DO stmt WHILE '(' expr ')'
        //>        ;
        AstStmtPtr parseDoStmt()
        {
            auto startToken = currentToken();
            if (!expect(TK_DO))
                return nullptr;
            auto stmt = parseLocalStmt();
            if (!stmt)
                return nullptr;
            if (!expect(TK_WHILE))
                return nullptr;
            if (!expect('('))
                return nullptr;
            auto expr = parseExpr();
            auto endToken = currentToken();
            if (!expect(')'))
                return nullptr;
            return std::make_unique< AstDoStmt >(
                std::move(stmt), std::move(expr), startToken, endToken);
        }

        //> while_stmt: WHILE '(' expr ')' stmt
        //>           ;
        AstStmtPtr parseWhileStmt()
        {
            auto startToken = currentToken();
            if (!expect(TK_WHILE))
                return nullptr;
            if (!expect('('))
                return nullptr;
            auto expr = parseExpr();
            if (!expr)
                return nullptr;
            if (!expect(')'))
                return nullptr;
            auto stmt = parseLocalStmt();
            if (!stmt)
                return nullptr;
            auto endToken = stmt->end;
            return std::make_unique< AstWhileStmt >(
                std::move(expr), std::move(stmt), startToken, endToken);
        }

        //> local_stmt: empty_stmt
        //>           | block_stmt
        //>           | return_stmt
        //>           | break_stmt
        //>           | continue_stmt
        //>           | goto_stmt
        //>           | if_stmt
        //>           | switch_stmt
        //>           | for_stmt
        //>           | do_stmt
        //>           | while_stmt
        //>           | label_decl
        //>           | decl
        //>           | expr_stmt
        //>           ;
        AstStmtPtr parseLocalStmt()
        {
            auto kind = currentToken()->kind;
            if (kind == ';') {
                return parseEmptyStmt();
            } else if (kind == '{')
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
            else if (kind == TK_SWITCH)
                return parseSwitchStmt();
            else if (kind == TK_FOR)
                return parseForStmt();
            else if (kind == TK_DO)
                return parseDoStmt();
            else if (kind == TK_WHILE)
                return parseWhileStmt();
            else if (kind == TK_IDENT && peekToken().kind == ':')
                return parseLabelDecl();
            else if (auto decl = parseDecl())
                return std::move(decl);
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
            while (true) {
                if (auto stmt = parseLocalStmt()) {
                    if (!isIgnored(stmt))
                        stmts.push_back(std::move(stmt));
                } else {
                    break;
                }
            }
            return stmts;
        }

        //> label_decl: IDENT ':'
        //>           ;
        AstDeclPtr parseLabelDecl()
        {
            auto startToken = currentToken();
            auto text = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            if (!expect(':'))
                return nullptr;
            auto stmt = parseLocalStmt();
            if (!stmt)
                return nullptr;
            auto endToken = stmt->end;
            return std::make_unique< AstLabelDecl >(
                std::move(text), std::move(stmt), startToken, endToken);
        }

        //> type_member: TK_IDENT
        //>            | type_member '.' TK_IDENT
        //>            ;
        bool parseTypeMember(std::string &composedName, Token **endToken)
        {
            while (true) {
                auto name = tokenText();
                *endToken = currentToken();
                if (accept(TK_IDENT)) {
                    composedName += name;
                    if (!accept('.'))
                        break;
                } else {
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
            if (!parseTypeMember(name, &endToken))
                return nullptr;
            auto typeRef = std::make_unique< AstTypeRef >(
                name, typeFlags, startToken, endToken);
            while (true) {
                typeFlags = TF_NONE;
                endToken = currentToken();
                if (accept('*')) {
                    typeFlags = TypeFlags(typeFlags | TF_POINTER);
                    if (currentToken()->kind == TK_CONST) {
                        endToken = currentToken();
                        expect(TK_CONST);
                        typeFlags = TypeFlags(typeFlags | TF_CONST);
                    }
                    typeRef = std::make_unique< AstTypeRef >(
                        std::move(typeRef), typeFlags, startToken, endToken);
                } else if (accept('[')) {
                    if (!expect(']'))
                        return nullptr;
                    typeFlags = TypeFlags(typeFlags | TF_ARRAY);
                    if (currentToken()->kind == TK_CONST) {
                        endToken = currentToken();
                        expect(TK_CONST);
                        typeFlags = TypeFlags(typeFlags | TF_CONST);
                    }
                    typeRef = std::make_unique< AstTypeRef >(
                        std::move(typeRef), typeFlags, startToken, endToken);
                } else
                    break;
            }
            bt.cancel();
            return typeRef;
        }

        //> decl_specifiers: %empty
        //>                | STATIC
        //>                ;
        auto parseDeclFlags()
        {
            DeclFlags flags = DF_NONE;
            if (accept(TK_STATIC))
                flags = DeclFlags(flags | DF_STATIC);
            // ...
            return flags;
        }

        //> var_or_func_prefix: decl_specifiers type_ref IDENT
        //>                   ;
        bool parseVarOrFuncPrefix(
            DeclFlags &flags, AstTypeRefPtr &type, std::string &name)
        {
            flags = parseDeclFlags();
            type = parseTypeRef();
            if (!type)
                return false;
            name = tokenText();
            if (!accept(TK_IDENT))
                return false;
            return true;
        }

        //> variable: var_or_func_prefix ( '=' expr )? ';'
        //>         ;
        AstDeclPtr parseVariable()
        {
            BackTracker bt(*this);
            auto startToken = currentToken();
            DeclFlags flags = DF_NONE;
            AstTypeRefPtr typeRef;
            std::string name;
            if (!parseVarOrFuncPrefix(flags, typeRef, name))
                return nullptr;
            AstExprPtr initExpr;
            if (accept('='))
                initExpr = parseExpr();
            auto endToken = currentToken();
            if (!accept(';'))
                return nullptr;
            auto var = std::make_unique< AstVarDecl >(std::move(name),
                std::move(typeRef), std::move(initExpr), startToken, endToken);
            var->flags = DeclFlags(var->flags | flags);
            bt.cancel();
            return std::move(var);
        }

        //> function: var_or_func_prefix '(' parameter* ')' '{' local_stmt* '}'
        //>         ;
        AstDeclPtr parseFunction()
        {
            BackTracker bt(*this);
            auto startToken = currentToken();
            DeclFlags flags = DF_NONE;
            AstTypeRefPtr typeRef;
            std::string name;
            if (!parseVarOrFuncPrefix(flags, typeRef, name))
                return nullptr;
            AstDeclList params;
            if (!accept('('))
                return nullptr;
            if (!accept(')')) {
                while (true) {
                    if (auto param = parseParameter()) {
                        if (!isIgnored(param))
                            params.push_back(std::move(param));
                        if (!accept(','))
                            break;
                    } else {
                        break;
                    }
                }
                if (!expect(')'))
                    return nullptr;
            }
            AstStmtList stmts;
            if (!accept('{'))
                return nullptr;
            auto endToken = currentToken();
            if (!accept('}')) {
                while (true) {
                    if (auto stmt = parseLocalStmt()) {
                        if (!isIgnored(stmt))
                            stmts.push_back(std::move(stmt));
                    } else
                        break;
                }
                endToken = currentToken();
                if (!expect('}'))
                    return nullptr;
            }
            auto fun = std::make_unique< AstFuncDecl >(std::move(name),
                std::move(typeRef), std::move(params), std::move(stmts),
                startToken, endToken);
            fun->flags = DeclFlags(fun->flags | flags);
            bt.cancel();
            return std::move(fun);
        }

        //> var_or_func_def: function
        //>                | variable
        //>                ;
        AstDeclPtr parseVarOrFuncDef()
        {
            if (auto fun = parseFunction())
                return fun;
            return parseVariable();
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
            return std::make_unique< AstParamDecl >(std::move(name),
                std::move(typeRef), std::move(defExpr), startToken, endToken);
        }

        //> typedef: TYPEDEF type_ref IDENT
        //>        ;
        AstDeclPtr parseTypeDef()
        {
            auto startToken = currentToken();
            if (!expect(TK_TYPEDEF))
                return nullptr;
            auto typeRef = parseTypeRef();
            assert(typeRef);
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            auto endToken = currentToken();
            if (!expect(';'))
                return nullptr;
            return std::make_unique< AstTypedef >(
                std::move(name), std::move(typeRef), startToken, endToken);
        }

        //> constructor_decl: IDENT '(' parameter* ')' '{' stmt* '}'
        //>                 ;
        AstDeclPtr parseConstructor(const std::string &structName)
        {
            auto startToken = currentToken();
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            if (name != structName) {
                compiler.error(*startToken,
                    "constructor name '%' does not match struct name '%'", name,
                    structName);
                return nullptr;
            }
            if (!expect('('))
                return nullptr;
            AstDeclList params;
            if (!accept(')')) {
                while (true) {
                    if (auto param = parseParameter()) {
                        params.push_back(std::move(param));
                        if (!accept(','))
                            break;
                    } else {
                        break;
                    }
                }
                if (!expect(')'))
                    return nullptr;
            }
            if (!expect('{'))
                return nullptr;
            AstStmtList stmts;
            auto endToken = currentToken();
            if (!accept('}')) {
                while (true) {
                    if (auto stmt = parseLocalStmt()) {
                        if (!isIgnored(stmt))
                            stmts.push_back(std::move(stmt));
                    } else {
                        break;
                    }
                }
                endToken = currentToken();
                if (!expect('}'))
                    return nullptr;
            }
            return std::make_unique< AstConstructorDecl >(std::move(name),
                std::move(params), std::move(stmts), startToken, endToken);
        }

        //> destructor_decl: '~' IDENT '(' ')' '{' stmt* '}'
        //>                ;
        AstDeclPtr parseDestructor(const std::string &structName)
        {
            auto startToken = currentToken();
            if (!expect('~'))
                return nullptr;
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            if (name != structName) {
                compiler.error(*startToken,
                    "destructor name '%' does not match struct name '%'", name,
                    structName);
                return nullptr;
            }
            name = '~' + name;
            if (!expect('('))
                return nullptr;
            if (!expect(')'))
                return nullptr;
            if (!expect('{'))
                return nullptr;
            AstStmtList stmts;
            auto endToken = currentToken();
            if (!accept('}')) {
                while (true) {
                    if (auto stmt = parseLocalStmt()) {
                        if (!isIgnored(stmt))
                            stmts.push_back(std::move(stmt));
                    } else {
                        break;
                    }
                }
                endToken = currentToken();
                if (!expect('}'))
                    return nullptr;
            }
            return std::make_unique< AstDestructorDecl >(
                std::move(name), std::move(stmts), startToken, endToken);
        }

        //> struct_member: constructor_decl
        //>              | destructor_decl
        //>              | decl
        //>              ;
        AstDeclPtr parseStructMember(const std::string &structName)
        {
            if (currentToken()->kind == TK_IDENT && tokenText() == structName
                && peekToken(1).kind == '(') {
                return parseConstructor(structName);
            } else if (currentToken()->kind == '~'
                && peekToken(1).kind == TK_IDENT && peekToken(2).kind == '(') {
                return parseDestructor(structName);
            }
            return parseDecl();
        }

        //> struct: STRUCT IDENT ';'
        //>       | STRUCT IDENT '{' struct_member* '}'
        //>       ;
        AstDeclPtr parseStruct()
        {
            auto startToken = currentToken();
            if (!expect(TK_STRUCT))
                return nullptr;
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            AstDeclList members;
            auto endToken = currentToken();
            if (!accept(';')) {
                if (!expect('{'))
                    return nullptr;
                endToken = currentToken();
                if (!accept('}')) {
                    while (true) {
                        if (auto decl = parseStructMember(name)) {
                            if (!isIgnored(decl))
                                members.push_back(std::move(decl));
                        } else {
                            break;
                        }
                    }
                    endToken = currentToken();
                    if (!expect('}'))
                        return nullptr;
                }
            }
            return std::make_unique< AstStructDecl >(
                std::move(name), std::move(members), startToken, endToken);
        }

        //> enumerator: IDENT
        //>           | IDENT '=' expr
        //>           ;
        AstEnumeratorDeclPtr parseEnumerator()
        {
            auto startToken = currentToken();
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            AstExprPtr initExpr;
            auto endToken = currentToken();
            if (accept('=')) {
                initExpr = parseExpr();
                if (!initExpr)
                    return nullptr;
                endToken = initExpr->end;
            }
            return std::make_unique< AstEnumeratorDecl >(
                std::move(name), std::move(initExpr), startToken, endToken);
        }

        //> enum: ENUM IDENT ';'
        //>     | ENUM IDENT '{' '}'
        //>     | ENUM IDENT '{' enumerator ( ',' enumerator )* ','? '}'
        //>     ;
        AstDeclPtr parseEnum()
        {
            auto startToken = currentToken();
            if (!expect(TK_ENUM))
                return nullptr;
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            AstEnumeratorList enumerators;
            auto endToken = currentToken();
            if (!accept(';')) {
                if (!expect('{'))
                    return nullptr;
                endToken = currentToken();
                if (!accept('}')) {
                    while (true) {
                        if (auto etor = parseEnumerator()) {
                            enumerators.push_back(std::move(etor));
                            if (!accept(','))
                                break;
                        } else {
                            break;
                        }
                    }
                    endToken = currentToken();
                    if (!expect('}'))
                        return nullptr;
                }
            }
            return std::make_unique< AstEnumDecl >(
                std::move(name), std::move(enumerators), startToken, endToken);
        }

        //> namespace: NAMESPACE dotted_name ';'
        //>          | NAMESPACE dotted_name '{' decl* '}'
        //>          ;
        AstDeclPtr parseNamespace()
        {
            auto startToken = currentToken();
            if (!expect(TK_NAMESPACE))
                return nullptr;
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            //std::cerr << ">>> " << name << std::endl;
            AstStmtList stmts;
            auto endToken = currentToken();
            if (!accept(';')) {
                if (!expect('{'))
                    return nullptr;
                endToken = currentToken();
                if (!accept('}')) {
                    while (true) {
                        if (auto stmt = parseDecl()) {
                            if (!isIgnored(stmt))
                                stmts.push_back(std::move(stmt));
                        } else {
                            break;
                        }
                    }
                    endToken = currentToken();
                    if (!expect('}'))
                        return nullptr;
                }
            }
            return std::make_unique< AstNamespaceDecl >(
                std::move(name), std::move(stmts), startToken, endToken);
        }

        //> using: using IDENT ( '.' IDENT )* ';'
        //>      ;
        AstDeclPtr parseUsing()
        {
            auto startToken = currentToken();
            if (!expect(TK_USING))
                return nullptr;
            auto name = tokenText();
            if (!expect(TK_IDENT))
                return nullptr;
            while (accept('.')) {
                name += tokenText();
                if (!expect(TK_IDENT))
                    return nullptr;
            }
            auto endToken = currentToken();
            if (!expect(';'))
                return nullptr;
            return std::make_unique< AstUsingDecl >(
                std::move(name), startToken, endToken);
        }

        //> bool_attr: BOOL_ATTR
        //>          ;
        AstAttributePtr parseBoolAttribute()
        {
            auto startToken = currentToken();
            if (!expect(TK_BOOL_ATTR))
                return nullptr;
            return std::make_unique< AstBoolAttribute >(startToken, startToken);
        }

        //> int_attr_field: 'rank' '=' INT
        //>               | 'width' '=' INT
        //>               | 'signed' '=' ( TRUE | FALSE )
        //>               | 'min' '=' INT
        //>               | 'max' '=' INT
        //>               ;
        //>
        //> int_attr: INT_ATTR '(' int_attr_field ( ',' int_attr_field )* ')'
        //>         ;
        AstAttributePtr parseIntAttribute()
        {
            auto startToken = currentToken();
            auto text = tokenText();
            if (!expect(TK_INT_ATTR))
                return nullptr;
            int rank = 0;
            int width = 0;
            bool isSigned = false;
            uint64_t min = 0;
            uint64_t max = 0;
            if (!expect('('))
                return nullptr;
            auto endToken = currentToken();
            if (!accept(')')) {
                while (true) {
                    auto fieldName = tokenText();
                    if (!expect(TK_IDENT))
                        break;
                    if (!expect('='))
                        break;
                    auto valueToken = currentToken();
                    if (fieldName == "rank") {
                        if (auto val = parseIntLit()) {
                            assert(val->kind == NK_INT);
                            rank = static_cast< AstInt * >(val.get())->value;
                        } else {
                            compiler.error(*valueToken,
                                "expected an integer literal for '%' field",
                                fieldName);
                            break;
                        }
                    } else if (fieldName == "width") {
                        if (auto val = parseIntLit()) {
                            assert(val->kind == NK_INT);
                            width = static_cast< AstInt * >(val.get())->value;
                        } else {
                            compiler.error(*valueToken,
                                "expected an integer literal for '%' field",
                                fieldName);
                            break;
                        }
                    } else if (fieldName == "signed") {
                        if (auto val = parseBoolLit()) {
                            assert(val->kind == NK_BOOL);
                            isSigned
                                = static_cast< AstBool * >(val.get())->value;
                        } else {
                            compiler.error(*valueToken,
                                "expected an integer literal for '%' field",
                                fieldName);
                            break;
                        }
                    } else if (fieldName == "min") {
                        if (auto val = parseIntLit()) {
                            assert(val->kind == NK_INT);
                            min = static_cast< AstInt * >(val.get())->value;
                        } else {
                            compiler.error(*valueToken,
                                "expected an integer literal for '%' field",
                                fieldName);
                            break;
                        }
                    } else if (fieldName == "max") {
                        if (auto val = parseIntLit()) {
                            assert(val->kind == NK_INT);
                            max = static_cast< AstInt * >(val.get())->value;
                        } else {
                            compiler.error(*valueToken,
                                "expected an integer literal for '%' field",
                                fieldName);
                            break;
                        }
                    } else {
                        compiler.error(*startToken,
                            "unexpected int attribute field '%'", fieldName);
                        break;
                    }
                    if (!accept(','))
                        break;
                }
                endToken = currentToken();
                if (!expect(')'))
                    return nullptr;
            }
            auto attr
                = std::make_unique< AstIntAttribute >(startToken, endToken);
            attr->rank = rank;
            attr->width = width;
            attr->isSigned = isSigned;
            attr->min = min;
            attr->max = max;
            return attr;
        }

        //> float_attr_field: 'rank' '=' INT
        //>                 | 'width' '=' INT
        //>                 ;
        //>
        //> float_attr: FLOAT_ATTR '(' float_attr_field ( ',' float_attr_field )* ')'
        //>           ;
        AstAttributePtr parseFloatAttribute()
        {
            auto startToken = currentToken();
            auto text = tokenText();
            if (!expect(TK_FLOAT_ATTR))
                return nullptr;
            int rank = 0;
            int width = 0;
            if (!expect('('))
                return nullptr;
            auto endToken = currentToken();
            if (!accept(')')) {
                while (true) {
                    auto fieldName = tokenText();
                    if (!expect(TK_IDENT))
                        break;
                    if (!expect('='))
                        break;
                    auto valueToken = currentToken();
                    if (fieldName == "rank") {
                        if (auto val = parseIntLit()) {
                            assert(val->kind == NK_INT);
                            rank = static_cast< AstInt * >(val.get())->value;
                        } else {
                            compiler.error(*valueToken,
                                "expected an integer literal for '%' field",
                                fieldName);
                            break;
                        }
                    } else if (fieldName == "width") {
                        if (auto val = parseIntLit()) {
                            assert(val->kind == NK_INT);
                            width = static_cast< AstInt * >(val.get())->value;
                        } else {
                            compiler.error(*valueToken,
                                "expected an integer literal for '%' field",
                                fieldName);
                            break;
                        }
                    } else {
                        compiler.error(*startToken,
                            "unexpected float attribute field '%'", fieldName);
                        break;
                    }
                    if (!accept(','))
                        break;
                }
                endToken = currentToken();
                if (!expect(')'))
                    return nullptr;
            }
            auto attr
                = std::make_unique< AstFloatAttribute >(startToken, endToken);
            attr->rank = rank;
            attr->width = width;
            return attr;
        }

        //> attribute: bool_attr
        //>          | int_attr
        //>          | float_attr
        //>          ;
        //>
        //> attribute_list : attribute*
        //>                ;
        AstAttributeList parseAttributes()
        {
            AstAttributeList attrs;
            while (true) {
                auto kind = currentToken()->kind;
                if (kind == TK_BOOL_ATTR) {
                    if (auto attr = parseBoolAttribute())
                        attrs.push_back(std::move(attr));
                    else
                        break;
                } else if (kind == TK_INT_ATTR) {
                    if (auto attr = parseIntAttribute())
                        attrs.push_back(std::move(attr));
                    else
                        break;
                } else if (kind == TK_FLOAT_ATTR) {
                    if (auto attr = parseFloatAttribute())
                        attrs.push_back(std::move(attr));
                    else
                        break;
                } else {
                    break;
                }
            }
            return attrs;
        }

        //> decl: ';'
        //>     | attribute_list typedef
        //>     | attribute_list variable
        //>     | attribute_list function
        //>     | attribute_list struct
        //>     | attribute_list enum
        //>     | attribute_list namespace
        //>     | attribute_list using
        //>     ;
        AstDeclPtr parseDecl()
        {
            AstAttributeList attrs = parseAttributes();
            AstDeclPtr decl;
            auto kind = currentToken()->kind;
            auto startToken = currentToken();
            if (kind == ';') {
                auto endToken = currentToken();
                expect(';');
                return std::make_unique< AstEmptyDecl >(startToken, endToken);
            } else if (kind == TK_TYPEDEF) {
                decl = parseTypeDef();
            } else if (kind == TK_STRUCT) {
                decl = parseStruct();
            } else if (kind == TK_ENUM) {
                decl = parseEnum();
            } else if (kind == TK_NAMESPACE) {
                decl = parseNamespace();
            } else if (kind == TK_USING) {
                decl = parseUsing();
            } else {
                decl = parseVarOrFuncDef();
            }
            if (decl)
                decl->attributes = std::move(attrs);
            return decl;
        }
    };

    AstModulePtr parseTokens(Compiler &compiler, TokenList &tokenList)
    {
        Parser parser(compiler, tokenList);
        Token *startToken = nullptr;
        Token *endToken = nullptr;
        if (!tokenList.empty()) {
            startToken = &tokenList.front();
            endToken = &tokenList.back();
        }
        auto mod = std::make_unique< AstModule >(startToken, endToken);
        while (true) {
            if (auto decl = parser.parseDecl()) {
                if (!parser.isIgnored(decl))
                    mod->members.push_back(std::move(decl));
            } else {
                break;
            }
        }
        return mod;
    }

} // namespace Soda
