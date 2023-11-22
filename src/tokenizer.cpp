#include "tokenizer.hpp"

#include "utils.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace soda {

  void token::start(source_position start_pos) {
    kind = token::kind::error;
    text.clear();
    range.start = start_pos;
    range.end = start_pos;
  }

  void token::end(enum token::kind kind_, source_position end_pos) {
    kind = kind_;
    range.end = end_pos;
  }

  std::string_view to_string(enum token::kind kind_) {
    switch (kind_) {
      case token::kind::error:
        return "error";
      case token::kind::end:
        return "end";
      case token::kind::langle:
        return "langle";
      case token::kind::rangle:
        return "rangle";
      case token::kind::plus:
        return "plus";
      case token::kind::dash:
        return "dash";
      case token::kind::asterisk:
        return "asterisk";
      case token::kind::slash:
        return "slash";
      case token::kind::percent:
        return "percent";
      case token::kind::ampersand:
        return "ampersand";
      case token::kind::caret:
        return "caret";
      case token::kind::pipe:
        return "pipe";
      case token::kind::equal:
        return "equal";
      case token::kind::exclamation:
        return "exclamation";
      case token::kind::semicolon:
        return "semicolon";
      case token::kind::lbrace:
        return "lbrace";
      case token::kind::rbrace:
        return "rbrace";
      case token::kind::comma:
        return "comma";
      case token::kind::colon:
        return "colon";
      case token::kind::lparen:
        return "lparen";
      case token::kind::rparen:
        return "rparen";
      case token::kind::lbracket:
        return "lbracket";
      case token::kind::rbracket:
        return "rbracket";
      case token::kind::dot:
        return "dot";
      case token::kind::tilde:
        return "tilde";
      case token::kind::question:
        return "question";
      case token::kind::comment:
        return "comment";
      case token::kind::int_lit:
        return "int_lit";
      case token::kind::float_lit:
        return "float_lit";
      case token::kind::char_lit:
        return "char_lit";
      case token::kind::string_lit:
        return "string_lit";
      case token::kind::ident:
        return "ident";
      case token::kind::lshift:
        return "lshift";
      case token::kind::rshift:
        return "rshift";
      case token::kind::inc:
        return "inc";
      case token::kind::dec:
        return "dec";
      case token::kind::pow:
        return "pow";
      case token::kind::log_and:
        return "log_and";
      case token::kind::log_or:
        return "log_or";
      case token::kind::le:
        return "le";
      case token::kind::ge:
        return "ge";
      case token::kind::eq:
        return "eq";
      case token::kind::ne:
        return "ne";
      case token::kind::lshift_assign:
        return "lshift_assign";
      case token::kind::rshift_assign:
        return "rshift_assign";
      case token::kind::add_assign:
        return "add_assign";
      case token::kind::sub_assign:
        return "sub_assign";
      case token::kind::mul_assign:
        return "mul_assign";
      case token::kind::div_assign:
        return "div_assign";
      case token::kind::mod_assign:
        return "mod_assign";
      case token::kind::and_assign:
        return "and_assign";
      case token::kind::xor_assign:
        return "xor_assign";
      case token::kind::or_assign:
        return "or_assign";
      case token::kind::kw_break:
        return "kw_break";
      case token::kind::kw_case:
        return "kw_case";
      case token::kind::kw_continue:
        return "kw_continue";
      case token::kind::kw_default:
        return "kw_default";
      case token::kind::kw_do:
        return "kw_do";
      case token::kind::kw_else:
        return "kw_else";
      case token::kind::kw_false:
        return "kw_false";
      case token::kind::kw_for:
        return "kw_for";
      case token::kind::kw_foreach:
        return "kw_foreach";
      case token::kind::kw_fun:
        return "kw_fun";
      case token::kind::kw_goto:
        return "kw_goto";
      case token::kind::kw_if:
        return "kw_if";
      case token::kind::kw_let:
        return "let";
      case token::kind::kw_return:
        return "kw_return";
      case token::kind::kw_switch:
        return "kw_switch";
      case token::kind::kw_true:
        return "kw_true";
      case token::kind::kw_while:
        return "kw_while";
    }
    unreachable();
    return "unknown";
  }

  std::string to_string(token const &tok) {
    std::stringstream ss;
    ss << tok;
    return ss.str();
  }

  static std::string escape_token_text(std::string const &s) {
    std::string res;
    for (auto ch : s) {
      if (ch == '\'') {
        res += "\\'";
      } else if (!std::isprint(ch) ||
                 (std::isspace(ch) && ch != ' ' && ch != '\t')) {
        char buf[8] = {0};
        std::snprintf(buf, 8, "\\x%02X", ch);
        res += buf;
      } else {
        res += ch;
      }
    }
    return res;
  }

  std::ostream &operator<<(std::ostream &out, token const &tok) {
    out << '(' << tok.kind << " '" << tok.range << "' '"
        << escape_token_text(tok.text) << "')";
    return out;
  }

  source_position tokenizer::start_pos() {
    return source_position{offset_, line_ - 1, column_ > 0 ? column_ - 1 : 0};
  }

  source_position tokenizer::end_pos() {
    return source_position{offset_, line_ - 1, column_ > 0 ? column_ - 1 : 0};
  }

  int tokenizer::get_char() {
    assert(input_);
    ch_ = input_->get();
    if (ch_ == eof) {
      return ch_;
    }
    offset_++;
    if (ch_ == '\n') {
      line_++;
      column_ = 0;
    } else if (ch_ == '\r') {
      if (peek_char() == '\n') {
        input_->get();
        offset_++;
      }
      line_++;
      column_ = 0;
    } else {
      column_++;
    }
    return ch_;
  }

  int tokenizer::peek_char() {
    assert(input_);
    return input_->peek();
  }

  static inline bool is_bin(int ch) {
    return ch == '0' || ch == '1';
  }

  static inline bool is_dec(int ch) {
    return ch >= '0' && ch <= '9';
  }

  static inline bool is_hex(int ch) {
    return (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') || is_dec(ch);
  }

  static inline bool is_oct(int ch) {
    return ch >= '0' && ch <= '7';
  }

  static inline bool is_alpha(int ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
  }

  static inline bool is_alnum(int ch) {
    return is_alpha(ch) || is_dec(ch);
  }

  static inline bool is_whitespace(int ch) {
    switch (ch) {
      case ' ':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
      case '\v':
        return true;
      default:
        break;
    }
    return false;
  }

  static enum token::kind kw_kind(std::string_view name) {
    static const std::unordered_map<std::string_view, enum token::kind> tab {
      {"break", token::kind::kw_break},
      {"case", token::kind::kw_case},
      {"continue", token::kind::kw_continue},
      {"default", token::kind::kw_default},
      {"do", token::kind::kw_do},
      {"else", token::kind::kw_else},
      {"false", token::kind::kw_false},
      {"for", token::kind::kw_for},
      {"foreach", token::kind::kw_foreach},
      {"goto", token::kind::kw_goto},
      {"if", token::kind::kw_if},
      {"return", token::kind::kw_return},
      {"switch", token::kind::kw_switch},
      {"true", token::kind::kw_true},
      {"while", token::kind::kw_while},
    };
    if (auto found = tab.find(name); found != tab.end()) {
      return found->second;
    }
    return token::kind::ident;
  }

  source_position start_pos();
  source_position end_pos();
  void tokenizer::next_token() {

    while (is_whitespace(ch_))
      get_char();

    tok_.start(start_pos());

    // identifiers and keywords
    if (is_alpha(ch_) || ch_ == '_') {
      do {
        tok_.text += static_cast<char>(ch_);
      } while (is_alnum(get_char()) || ch_ == '_');
      return tok_.end(kw_kind(tok_.text), end_pos());
    }

    // integer and floating-point numbers
    else if (std::isdigit(ch_) || ch_ == '.') {
      if (ch_ == '0') {
        tok_.text += static_cast<char>(ch_);
        switch (get_char()) {
          case 'b':
          case 'B':
            tok_.text += static_cast<char>(ch_);
            while (is_bin(get_char())) {
              tok_.text += static_cast<char>(ch_);
            }
            return tok_.end(token::kind::int_lit, end_pos());
          case 'd':
          case 'D':
            tok_.text += static_cast<char>(ch_);
            while (is_dec(get_char())) {
              tok_.text += static_cast<char>(ch_);
            }
            return tok_.end(token::kind::int_lit, end_pos());
          case 'o':
          case 'O':
            tok_.text += static_cast<char>(ch_);
            while (is_oct(get_char())) {
              tok_.text += static_cast<char>(ch_);
            }
            return tok_.end(token::kind::int_lit, end_pos());
          case 'x':
          case 'X':
            tok_.text += static_cast<char>(ch_);
            while (is_hex(get_char())) {
              tok_.text += static_cast<char>(ch_);
            }
            return tok_.end(token::kind::int_lit, end_pos());
          default:
            break;
        }
      }

      bool has_dot = false;
      while (is_dec(ch_) || (ch_ == '.' && !has_dot)) {
        if (ch_ == '.')
          has_dot = true;
        tok_.text += static_cast<char>(ch_);
        get_char();
      }

      if (has_dot) {
        return tok_.end(token::kind::float_lit, end_pos());
      } else {
        return tok_.end(token::kind::int_lit, end_pos());
      }
    }

    // character and string literals
    else if (ch_ == '"' || ch_ == '\'') {
      auto quote = ch_;
      auto last = ch_;
      tok_.text += static_cast<char>(ch_);
      while (get_char() != eof) {
        tok_.text += static_cast<char>(ch_);
        if (ch_ == quote && last != '\\') {
          get_char();
          break;
        }
        last = ch_;
      }
      if (ch_ == eof) {
        tok_.text = "eof encountered inside quoted literal";
        return tok_.end(token::kind::error, end_pos());
      } else if (quote == '\'') {
        return tok_.end(token::kind::char_lit, end_pos());
      } else if (quote == '"') {
        return tok_.end(token::kind::string_lit, end_pos());
      }
    }

    else if (ch_ == '>') { // >, >>, >=, >>=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '>') { // >>
        tok_.text += static_cast<char>(ch_);
        get_char();
        if (ch_ == '=') { // >>=
          tok_.text += static_cast<char>(ch_);
          get_char();
          return tok_.end(token::kind::rshift_assign, end_pos());
        }
        return tok_.end(token::kind::rshift, end_pos());
      } else if (ch_ == '=') { // >=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::ge, end_pos());
      }
      return tok_.end(token::kind::rangle, end_pos());
    }

    else if (ch_ == '<') { // <, <<, <=, <<=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '<') { // <<
        tok_.text += static_cast<char>(ch_);
        get_char();
        if (ch_ == '=') { // <<=
          tok_.text += static_cast<char>(ch_);
          get_char();
          return tok_.end(token::kind::lshift_assign, end_pos());
        }
        return tok_.end(token::kind::lshift, end_pos());
      } else if (ch_ == '=') { // <=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::le, end_pos());
      }
      return tok_.end(token::kind::langle, end_pos());
    }

    else if (ch_ == '+') { // +, ++, +=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '+') { // ++
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::inc, end_pos());
      } else if (ch_ == '=') { // +=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::add_assign, end_pos());
      }
      return tok_.end(token::kind::plus, end_pos());
    }

    else if (ch_ == '-') { // -, --, -=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '-') { // --
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::dec, end_pos());
      } else if (ch_ == '=') { // -=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::sub_assign, end_pos());
      }
      return tok_.end(token::kind::dash, end_pos());
    }

    else if (ch_ == '*') { // *, **, *=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '*') { // **
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::pow, end_pos());
      } else if (ch_ == '=') { // *=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::mul_assign, end_pos());
      }
      return tok_.end(token::kind::asterisk, end_pos());
    }

    else if (ch_ == '/') { // /, /=, //, /**/
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '=') { // /=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::div_assign, end_pos());
      } else if (ch_ == '/') { // // (comment)
        tok_.text += static_cast<char>(ch_);
        while (get_char() != '\n') {
          tok_.text += static_cast<char>(ch_);
        }
        return tok_.end(token::kind::comment, end_pos());
      } else if (ch_ == '*') { // /* (comment)
        tok_.text += static_cast<char>(ch_);
        get_char();
        int depth = 1;
        while (depth > 0) {
          if (ch_ == '/' && peek_char() == '*') {
            tok_.text += "/*";
            get_char(); // advance over /
            get_char(); // advance over *
            depth++;
            continue;
          } else if (ch_ == '*' && peek_char() == '/') {
            tok_.text += "*/";
            get_char(); // advance over *
            get_char(); // advance over /
            depth--;
            continue;
          } else if (ch_ == eof) {
            tok_.text = "eof encountered inside multi-line comment";
            return tok_.end(token::kind::error, end_pos());
          } else {
            tok_.text += static_cast<char>(ch_);
            get_char();
          }
        }
        return tok_.end(token::kind::comment, end_pos());
      }
      return tok_.end(token::kind::slash, end_pos());
    }

    else if (ch_ == '%') { // %, %=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '=') { // %=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::mod_assign, end_pos());
      }
      return tok_.end(token::kind::percent, end_pos());
    }

    else if (ch_ == '&') { // &, &&, &=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '=') { // &=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::and_assign, end_pos());
      } else if (ch_ == '&') { // &&
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::log_and, end_pos());
      }
      return tok_.end(token::kind::ampersand, end_pos());
    }

    else if (ch_ == '^') { // ^, &=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '=') { // ^=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::xor_assign, end_pos());
      }
      return tok_.end(token::kind::caret, end_pos());
    }

    else if (ch_ == '|') { // |, ||, |=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '=') { // |=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::or_assign, end_pos());
      } else if (ch_ == '|') { // ||
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::log_or, end_pos());
      }
      return tok_.end(token::kind::pipe, end_pos());
    }

    else if (ch_ == '=') { // =, ==
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '=') { // ==
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::eq, end_pos());
      }
      return tok_.end(token::kind::equal, end_pos());
    }

    else if (ch_ == '!') { // !, !=
      tok_.text += static_cast<char>(ch_);
      get_char();
      if (ch_ == '=') { // !=
        tok_.text += static_cast<char>(ch_);
        get_char();
        return tok_.end(token::kind::ne, end_pos());
      }
      return tok_.end(token::kind::exclamation, end_pos());
    }

    else if (ch_ == ';') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::semicolon, end_pos());
    }

    else if (ch_ == '{') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::lbrace, end_pos());
    }

    else if (ch_ == '}') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::rbrace, end_pos());
    }

    else if (ch_ == ',') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::comma, end_pos());
    }

    else if (ch_ == ':') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::colon, end_pos());
    }

    else if (ch_ == '(') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::lparen, end_pos());
    }

    else if (ch_ == ')') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::rparen, end_pos());
    }

    else if (ch_ == '[') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::lbracket, end_pos());
    }

    else if (ch_ == ']') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::rbracket, end_pos());
    }

    else if (ch_ == '.') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::dot, end_pos());
    }

    else if (ch_ == '~') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::tilde, end_pos());
    }

    else if (ch_ == '?') {
      tok_.text += static_cast<char>(ch_);
      get_char();
      return tok_.end(token::kind::question, end_pos());
    }

    else if (ch_ == eof) {
      return tok_.end(token::kind::end, end_pos());
    }

    else {
      auto c = ch_;
      get_char();
      std::stringstream ss;
      if (std::isprint(c) && !std::isspace(c)) {
        ss << "invalid character \"" << static_cast<char>(c) << "\"";
      } else {
        char buf[8] = {0};
        std::snprintf(buf, 8, "\\x%02X", c);
        ss << "invalid character \"" << buf << "\"";
      }
      tok_.text = ss.str();
      return tok_.end(token::kind::error, end_pos());
    }
  }

} // namespace soda
