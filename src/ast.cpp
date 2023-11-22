#include "ast.hpp"
#include "utils.hpp"

namespace soda::ast {

  std::string_view to_string(node_kind kind) {
    switch (kind) {
      case node_kind::error:
        return "error";
      // atomic expresions
      case node_kind::bool_expr:
        return "bool_expr";
      case node_kind::int_expr:
        return "int_expr";
      case node_kind::float_expr:
        return "float_expr";
      case node_kind::char_expr:
        return "char_expr";
      case node_kind::string_expr:
        return "string_expr";
      case node_kind::ident_expr:
        return "ident_expr";
      // compound expressions
      case node_kind::unop_expr:
        return "unop_expr";
      case node_kind::binop_expr:
        return "binop_expr";
      case node_kind::if_expr:
        return "if_expr";
      case node_kind::call_expr:
        return "call_expr";
      // statements
      case node_kind::empty_stmt:
        return "empty_stmt";
      case node_kind::expr_stmt:
        return "expr_stmt";
      case node_kind::block_stmt:
        return "block_stmt";
      case node_kind::goto_stmt:
        return "goto_stmt";
      case node_kind::break_stmt:
        return "break_stmt";
      case node_kind::continue_stmt:
        return "continue_stmt";
      case node_kind::return_stmt:
        return "return_stmt";
      case node_kind::if_stmt:
        return "if_stmt";
      case node_kind::switch_stmt:
        return "switch_stmt";
      case node_kind::case_stmt:
        return "case_stmt";
      case node_kind::do_stmt:
        return "do_stmt";
      case node_kind::while_stmt:
        return "while_stmt";
      case node_kind::for_stmt:
        return "for_stmt";
      case node_kind::foreach_stmt:
        return "foreach_stmt";
      // declarations
      case node_kind::let_decl:
        return "let_decl";
      case node_kind::fun_decl:
        return "fun_decl";
      // miscellaneous
      case node_kind::type_ref:
        return "type_ref";
      case node_kind::unresolved_type_ref:
        return "unresolved_type_ref";
      case node_kind::resolved_type_ref:
        return "resolved_type_ref";
      case node_kind::translation_unit:
        return "translation_unit";
      case node_kind::program:
        return "program";
    }
    unreachable();
    return "unknown_node_kind";
  }

} // namespace soda::ast
