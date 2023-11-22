#pragma once

#include "operators.hpp"
#include "source_range.hpp"

#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace soda::ast {

  //
  // Node kind
  //

  enum class node_kind {
    error,
    // literal/atomic expressions
    bool_expr,
    int_expr,
    float_expr,
    char_expr,
    string_expr,
    ident_expr,
    // compound expressions
    unop_expr,
    binop_expr,
    if_expr,
    call_expr,
    // statements
    empty_stmt,
    expr_stmt,
    block_stmt,
    goto_stmt,
    break_stmt,
    continue_stmt,
    return_stmt,
    if_stmt,
    switch_stmt,
    case_stmt,
    do_stmt,
    while_stmt,
    for_stmt,
    foreach_stmt,
    // declarations
    let_decl,
    fun_decl,
    // ...
    // type references
    type_ref,
    unresolved_type_ref,
    resolved_type_ref,
    // miscellaneous
    translation_unit,
    program,
  };

  std::string_view to_string(node_kind kind);

  inline std::ostream &operator<<(std::ostream &out, node_kind kind) {
    return out << to_string(kind);
  }

  //
  // Abstract base node
  //

  class node {
  public:
    template <typename T>
    using ptr = std::shared_ptr<T>;
    template <typename T>
    using list = std::vector<ptr<T>>;

    node_kind kind;
    source_range range;

  protected:
    node(node_kind kind, source_range range)
        : kind{kind}, range{std::move(range)} {
    }

  public:
    virtual ~node() = default;

    std::string_view kind_name() const noexcept {
      return to_string(kind);
    }

    template <typename T, typename... Args>
    static ptr<T> make(node_kind kind, source_range range, Args &&...args) {
      static_assert(std::is_base_of_v<node, T>, "T must derive from ast::node");
      return ptr<T>{new T{kind, std::move(range), std::forward<T>(args)...}};
    }

    virtual bool is_error_node() const noexcept {
      return false;
    }

  private:
    node(node const &) = delete;
    node &operator=(node const &) = delete;
  };

  //
  // Error node
  //

  template <typename T>
  class error_node final : public T {

  public:
    std::string message;

    error_node(source_range range, std::string message)
        : T{node_kind::error, std::move(range)}, message{message} {
      static_assert(std::is_base_of_v<node, T>, "T must derive from ast::node");
    }

    bool is_error_node() const noexcept final {
      return true;
    }
  };

  //
  // Abstract expression base nodes
  //

  class expr : public node {
  public:
    using ptr = node::ptr<expr>;
    using list = node::list<ptr>;

  protected:
    expr(node_kind kind, source_range range) : node{kind, std::move(range)} {
    }
  };

  class atomic_expr : public expr {
  protected:
    atomic_expr(node_kind kind, source_range range)
        : expr{kind, std::move(range)} {
    }
  };

  class compound_expr : public expr {
  protected:
    operator_kind op;

    compound_expr(node_kind kind, source_range range, operator_kind op)
        : expr{kind, std::move(range)}, op{op} {
    }

  public:
    virtual int arity() const noexcept = 0;
  };

  //
  // Abstract statement base nodes
  //

  class stmt : public node {
  public:
    using ptr = node::ptr<stmt>;
    using list = node::list<ptr>;

  protected:
    stmt(node_kind kind, source_range range) : node{kind, std::move(range)} {
    }
  };

  class jump_stmt : public stmt {
  protected:
    jump_stmt(node_kind kind, source_range range)
        : stmt{kind, std::move(range)} {
    }
  };

  class iter_stmt : public stmt {
  protected:
    iter_stmt(node_kind kind, source_range range)
        : stmt{kind, std::move(range)} {
    }
  };

  class sel_stmt : public stmt {
  protected:
    sel_stmt(node_kind kind, source_range range)
        : stmt{kind, std::move(range)} {
    }
  };

  class decl : public stmt {
  public:
    using ptr = node::ptr<decl>;
    using list = node::list<ptr>;

    std::string name;

  protected:
    decl(node_kind kind, source_range range, std::string name)
        : stmt{kind, std::move(range)}, name{std::move(name)} {
    }
  };

  //
  // Type references
  //

  class type_ref : public node {
  public:
    std::string name;
    decl::ptr ref;

    type_ref(node_kind kind, source_range range, std::string name,
             decl::ptr ref = nullptr)
        : node{kind, std::move(range)}, name{std::move(name)},
          ref{std::move(ref)} {
    }

    bool is_resolved() const noexcept {
      return ref.get() != nullptr;
    }
  };

  class unresolved_type_ref final : public type_ref {
  public:
    unresolved_type_ref(source_range range, std::string name)
        : type_ref{node_kind::unresolved_type_ref, std::move(range),
                   std::move(name)} {
    }
  };

  class resolved_type_ref final : public type_ref {
  public:
    resolved_type_ref(source_range range, std::string name, decl::ptr ref)
        : type_ref{node_kind::resolved_type_ref, std::move(range),
                   std::move(name), std::move(ref)} {
    }
  };

  //
  // Concrete literal/atomic expression nodes
  //

  class bool_expr final : public atomic_expr {
  public:
    bool value;

    bool_expr(source_range range, bool value)
        : atomic_expr{node_kind::bool_expr, std::move(range)}, value{value} {
    }
  };

  class int_expr final : public atomic_expr {
  public:
    unsigned long long int value;

    int_expr(source_range range, unsigned long long int value)
        : atomic_expr{node_kind::int_expr, std::move(range)}, value{value} {
    }
  };

  class float_expr final : public atomic_expr {
  public:
    long double value;

    float_expr(source_range range, long double value)
        : atomic_expr{node_kind::float_expr, std::move(range)}, value{value} {
    }
  };

  class char_expr final : public atomic_expr {
  public:
    std::string value;

    char_expr(source_range range, std::string value)
        : atomic_expr{node_kind::char_expr, std::move(range)},
          value{std::move(value)} {
    }
  };

  class string_expr final : public atomic_expr {
  public:
    std::string value;

    string_expr(source_range range, std::string value)
        : atomic_expr{node_kind::string_expr, std::move(range)},
          value{std::move(value)} {
    }
  };

  class ident_expr final : public atomic_expr {
  public:
    std::string name;

    ident_expr(source_range range, std::string name)
        : atomic_expr{node_kind::ident_expr, std::move(range)},
          name{std::move(name)} {
    }
  };

  //
  // Concrete compound expression nodes
  //

  class unop_expr final : public compound_expr {
  public:
    expr::ptr operand;

    unop_expr(source_range range, operator_kind op, expr::ptr operand)
        : compound_expr{node_kind::unop_expr, std::move(range), op},
          operand{std::move(operand)} {
    }

    int arity() const noexcept final {
      return 1;
    }
  };

  class binop_expr final : public compound_expr {
  public:
    expr::ptr lhs;
    expr::ptr rhs;

    binop_expr(source_range range, operator_kind op, expr::ptr lhs,
               expr::ptr rhs)
        : compound_expr{node_kind::binop_expr, std::move(range), op},
          lhs{std::move(lhs)}, rhs{std::move(rhs)} {
    }

    int arity() const noexcept final {
      return 2;
    }
  };

  class if_expr final : public compound_expr {
  public:
    expr::ptr cond;
    expr::ptr cons;
    expr::ptr altn;

    if_expr(source_range range, expr::ptr cond, expr::ptr cons, expr::ptr altn)
        : compound_expr{node_kind::if_expr, std::move(range),
                        operator_kind::ifexpr},
          cond{std::move(cond)}, cons{std::move(cons)}, altn{std::move(altn)} {
    }

    int arity() const noexcept final {
      return 3;
    }
  };

  class call_expr final : public compound_expr {
  public:
    expr::ptr callee;
    expr::list arguments;

    call_expr(source_range range, expr::ptr callee, expr::list arguments)
        : compound_expr{node_kind::call_expr, std::move(range),
                        operator_kind::call},
          callee{std::move(callee)}, arguments{std::move(arguments)} {
    }

    int arity() const noexcept final {
      return arguments.size() + 1;
    }
  };

  //
  // Concrete basic statement nodes
  //

  class empty_stmt final : public stmt {
  public:
    empty_stmt(source_range range)
        : stmt{node_kind::empty_stmt, std::move(range)} {
    }
  };

  class expr_stmt final : public stmt {
  public:
    expr::ptr exp;

    expr_stmt(source_range range, expr::ptr expr)
        : stmt{node_kind::expr_stmt, std::move(range)}, exp{std::move(expr)} {
    }
  };

  class block_stmt final : public stmt {
  public:
    stmt::list stmts;

    block_stmt(source_range range, stmt::list stmts)
        : stmt{node_kind::block_stmt, std::move(range)},
          stmts{std::move(stmts)} {
    }
  };

  //
  // Concrete jump statement nodes
  //

  class goto_stmt final : public jump_stmt {
  public:
    std::string label;

    goto_stmt(source_range range, std::string label)
        : jump_stmt{node_kind::goto_stmt, std::move(range)},
          label{std::move(label)} {
    }
  };

  class continue_stmt final : public jump_stmt {
  public:
    std::string label;

    continue_stmt(source_range range, std::string label = std::string{})
        : jump_stmt{node_kind::continue_stmt, std::move(range)},
          label{std::move(label)} {
    }
  };

  class break_stmt final : public jump_stmt {
  public:
    std::string label;

    break_stmt(source_range range, std::string label = std::string{})
        : jump_stmt{node_kind::break_stmt, std::move(range)},
          label{std::move(label)} {
    }
  };

  class return_stmt final : public jump_stmt {
  public:
    expr::ptr exp;

    return_stmt(source_range range, expr::ptr exp = nullptr)
        : jump_stmt{node_kind::return_stmt, std::move(range)},
          exp{std::move(exp)} {
    }
  };

  //
  // Concrete selection statement nodes
  //

  class if_stmt final : public sel_stmt {
  public:
    expr::ptr cond;
    stmt::ptr cons;
    stmt::ptr altn;

    if_stmt(source_range range, expr::ptr cond, stmt::ptr cons,
            stmt::ptr altn = nullptr)
        : sel_stmt{node_kind::if_stmt, std::move(range)}, cond{std::move(cond)},
          cons{std::move(cons)}, altn{std::move(altn)} {
    }
  };

  class case_stmt final : public stmt {
  public:
    expr::ptr exp;
    stmt::list stmts;

    case_stmt(source_range range, expr::ptr exp, stmt::list stmts)
        : stmt{node_kind::case_stmt, std::move(range)}, exp{std::move(exp)},
          stmts{std::move(stmts)} {
    }

    bool is_default_case() const noexcept {
      return exp.get() != nullptr;
    }
  };

  class switch_stmt final : public sel_stmt {
  public:
    expr::ptr exp;
    stmt::list cases;

    switch_stmt(source_range range, expr::ptr exp, stmt::list cases)
        : sel_stmt{node_kind::switch_stmt, std::move(range)},
          exp{std::move(exp)}, cases{std::move(cases)} {
    }
  };

  //
  // Concrete iteration statements
  //

  class do_stmt final : public iter_stmt {
  public:
    stmt::ptr stmt;
    expr::ptr exp;

    do_stmt(source_range range, stmt::ptr stmt, expr::ptr exp)
        : iter_stmt{node_kind::do_stmt, std::move(range)},
          stmt{std::move(stmt)}, exp{std::move(exp)} {
    }
  };

  class while_stmt final : public iter_stmt {
  public:
    expr::ptr exp;
    stmt::ptr stmt;

    while_stmt(source_range range, expr::ptr exp, stmt::ptr stmt)
        : iter_stmt{node_kind::while_stmt, std::move(range)},
          exp{std::move(exp)}, stmt{std::move(stmt)} {
    }
  };

  class for_stmt final : public iter_stmt {
  public:
    stmt::ptr init;
    stmt::ptr test;
    stmt::ptr incr;

    for_stmt(source_range range, stmt::ptr init, stmt::ptr test, stmt::ptr incr)
        : iter_stmt{node_kind::for_stmt, std::move(range)},
          init{std::move(init)}, test{std::move(test)}, incr{std::move(incr)} {
    }
  };

  class foreach_stmt final : public iter_stmt {
  public:
    decl::ptr iter;
    expr::ptr exp;

    foreach_stmt(source_range range, decl::ptr iter, expr::ptr exp)
        : iter_stmt{node_kind::foreach_stmt, std::move(range)},
          iter{std::move(iter)}, exp{std::move(exp)} {
    }
  };

  //
  // Concrete declaration statements
  //

  class let_decl final : public decl {
  public:
    expr::ptr init_exp;

    let_decl(source_range range, std::string name, expr::ptr init_exp = nullptr)
        : decl{node_kind::let_decl, std::move(range), std::move(name)},
          init_exp{std::move(init_exp)} {
    }
  };

  class fun_decl final : public decl {
  public:
    decl::list params;
    stmt::list stmts;

    fun_decl(source_range range, std::string name, decl::list params,
             stmt::list stmts)
        : decl{node_kind::fun_decl, std::move(range), std::move(name)},
          params{std::move(params)}, stmts{std::move(stmts)} {
    }
  };

  //
  // Miscellaneous statements
  //

  class translation_unit final : public node {
  public:
    using ptr = node::ptr<translation_unit>;
    using list = node::list<ptr>;

    decl::list decls;

    translation_unit(std::filesystem::path fn, decl::list decls)
        : node{node_kind::program,
               source_range{std::move(fn), {0, 0, 0}, {0, 0, 0}}},
          decls{std::move(decls)} {
    }
  };

  class program final : public node {
  public:
    translation_unit::list tus;

    program(translation_unit::list tus)
        : node{node_kind::program, {}}, tus{std::move(tus)} {
    }
  };

} // namespace soda::ast
