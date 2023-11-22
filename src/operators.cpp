#include "operators.hpp"

#include "utils.hpp"

#include <utility>

namespace soda {

  std::string_view to_string(operator_kind op) {
    switch (op) {
      case operator_kind::pos:
        return "pos";
      case operator_kind::neg:
        return "neg";
      case operator_kind::bit_not:
        return "bit_not";
      case operator_kind::log_not:
        return "log_not";
      case operator_kind::pre_inc:
        return "pre_inc";
      case operator_kind::pre_dec:
        return "pre_dec";
      case operator_kind::post_inc:
        return "post_inc";
      case operator_kind::post_dec:
        return "post_dec";
      case operator_kind::add:
        return "add";
      case operator_kind::sub:
        return "sub";
      case operator_kind::mul:
        return "mul";
      case operator_kind::div:
        return "div";
      case operator_kind::mod:
        return "mod";
      case operator_kind::pow:
        return "pow";
      case operator_kind::bit_and:
        return "bit_and";
      case operator_kind::bit_xor:
        return "bit_xor";
      case operator_kind::bit_or:
        return "bit_or";
      case operator_kind::log_and:
        return "log_and";
      case operator_kind::log_or:
        return "log_or";
      case operator_kind::lt:
        return "lt";
      case operator_kind::gt:
        return "gt";
      case operator_kind::le:
        return "le";
      case operator_kind::ge:
        return "ge";
      case operator_kind::eq:
        return "eq";
      case operator_kind::ne:
        return "ne";
      case operator_kind::lshift:
        return "lshift";
      case operator_kind::rshift:
        return "rshift";
      case operator_kind::assign:
        return "assign";
      case operator_kind::add_assign:
        return "add_assign";
      case operator_kind::sub_assign:
        return "sub_assign";
      case operator_kind::mul_assign:
        return "mul_assign";
      case operator_kind::div_assign:
        return "div_assign";
      case operator_kind::mod_assign:
        return "mod_assign";
      case operator_kind::and_assign:
        return "and_assign";
      case operator_kind::xor_assign:
        return "xor_assign";
      case operator_kind::or_assign:
        return "or_assign";
      case operator_kind::lshift_assign:
        return "lshift_assign";
      case operator_kind::rshift_assign:
        return "rshift_assign";
      case operator_kind::comma:
        return "comma";
      case operator_kind::member:
        return "member";
      case operator_kind::index:
        return "index";
      case operator_kind::ifexpr:
        return "ifexpr";
      case operator_kind::call:
        return "call";
    }
    unreachable();
    return "unknown_op";
  }

} // namespace soda
