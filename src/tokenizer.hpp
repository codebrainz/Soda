#pragma once

#include "source_range.hpp"

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <istream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>

namespace soda {

  class token {
    friend class tokenizer;

  public:
    enum class kind {
      error = -1,
      end = 0,
      langle = '<',
      rangle = '>',
      plus = '+',
      dash = '-',
      asterisk = '*',
      slash = '/',
      percent = '%',
      ampersand = '&',
      caret = '^',
      pipe = '|',
      equal = '=',
      exclamation = '!',
      semicolon = ';',
      lbrace = '{',
      rbrace = '}',
      comma = ',',
      colon = ':',
      lparen = '(',
      rparen = ')',
      lbracket = '[',
      rbracket = ']',
      dot = '.',
      tilde = '~',
      question = '?',
      // ...
      lshift = 256,
      rshift,
      inc,
      dec,
      pow,
      log_and,
      log_or,
      le,
      ge,
      eq,
      ne,
      lshift_assign,
      rshift_assign,
      add_assign,
      sub_assign,
      mul_assign,
      div_assign,
      mod_assign,
      and_assign,
      xor_assign,
      or_assign,
      // ...
      comment = 512,
      int_lit,
      float_lit,
      char_lit,
      string_lit,
      ident,
      // ...
      kw_break,
      kw_case,
      kw_continue,
      kw_default,
      kw_do,
      kw_else,
      kw_false,
      kw_for,
      kw_foreach,
      kw_fun,
      kw_goto,
      kw_if,
      kw_let,
      kw_return,
      kw_switch,
      kw_true,
      kw_while,
    };

    enum kind kind = token::kind::error;
    std::string text;
    source_range range;

    token() = default;

    token(std::filesystem::path fn)
        : kind{token::kind::error}, text{}, range{fn, {0, 0, 0}, {0, 0, 0}} {
    }

  private:
    void start(source_position start_pos);
    void end(enum kind kind_, source_position end_pos);

    token(token const &) = delete;
    token &operator=(token const &) = delete;
  };

  std::string_view to_string(enum token::kind kind_);

  inline std::ostream &operator<<(std::ostream &out, enum token::kind kind_) {
    return out << to_string(kind_);
  }

  std::string to_string(token const &tok);
  std::ostream &operator<<(std::ostream &out, token const &tok);

  class tokenizer : public std::ranges::view_interface<tokenizer> {
  public:
    struct sentinel {};

    tokenizer(std::istream &input) : tokenizer{input, std::filesystem::path{}} {
    }

    tokenizer(std::filesystem::path fn)
        : fn_{fn}, input_{nullptr}, tok_{fn_}, ch_{'\0'}, offset_{0}, line_{1},
          column_{0}, delete_input{true} {
      input_ = new std::ifstream{fn};
      ch_ = input_->get();
      column_ = 1;
    }

    tokenizer(std::istream &input, std::filesystem::path fn)
        : fn_{fn}, input_{&input}, tok_{fn_}, ch_{input_->get()}, offset_{0},
          line_{1}, column_{1}, delete_input{false} {
    }

    ~tokenizer() {
      if (delete_input) {
        delete input_;
      }
    }

    auto begin() {
      return iterator{this};
    }

    auto end() const noexcept {
      return sentinel{};
    }

    auto const &token() {
      return tok_;
    }

    class iterator {
    public:
      using difference_type = std::ptrdiff_t;
      using value_type = soda::token;

      iterator() = default;

      iterator(tokenizer *p) : parent_{p} {
      }

      bool operator==(iterator const &) const {
        return false;
      }

      bool operator==(sentinel) const {
        return parent_->tok_.kind == token::kind::end;
      }

      soda::token const &operator*() const {
        parent_->next_token();
        return parent_->tok_;
      }

      iterator &operator++() {
        return *this;
      }

      iterator &operator++(int) {
        return *this;
      }

    private:
      tokenizer *parent_ = nullptr;
    };

  private:
    static constexpr int eof = std::istream::traits_type::eof();

    std::filesystem::path fn_;
    std::istream *input_;
    soda::token tok_;
    int ch_;
    std::size_t offset_;
    std::size_t line_;
    std::size_t column_;
    bool delete_input;

    int get_char();
    int peek_char();
    void next_token();
    source_position start_pos();
    source_position end_pos();
  };

} // namespace soda
