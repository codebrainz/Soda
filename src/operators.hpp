#pragma once

#include <ostream>
#include <string_view>

namespace soda {

  enum class operator_kind {
    // unary operations
    pos,      // +
    neg,      // -
    bit_not,  // ~
    log_not,  // !
    pre_inc,  // ++
    pre_dec,  // --
    post_inc, // ++
    post_dec, // --
    // binary operations
    add,           // +
    sub,           // -
    mul,           // *
    div,           // /
    mod,           // %
    pow,           // **
    bit_and,       // &
    bit_xor,       // ^
    bit_or,        // |
    log_and,       // &&
    log_or,        // ||
    lt,            // <
    gt,            // >
    le,            // <=
    ge,            // >=
    eq,            // ==
    ne,            // !=
    lshift,        // <<
    rshift,        // >>
    assign,        // =
    add_assign,    // +=
    sub_assign,    // -=
    mul_assign,    // *=
    div_assign,    // /=
    mod_assign,    // %=
    and_assign,    // &=
    xor_assign,    // ^=
    or_assign,     // |=
    lshift_assign, // <<=
    rshift_assign, // >>=
    comma,         // ,
    member,        // .
    index,         // []
    // ternary operations
    ifexpr, // ?:
    // other operations
    call, // ()
  };

  std::string_view to_string(operator_kind op);

  inline std::ostream &operator<<(std::ostream &out, operator_kind op) {
    return out << to_string(op);
  }

} // namespace soda
