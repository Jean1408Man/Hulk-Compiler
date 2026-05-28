// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "parser.hpp"


// Unqualified %code blocks.
#line 98 "src/parser/grammar.y"

    #define yylex() yylex(driver)

    static hulk::common::Span to_span(const hulk::parser::Parser::location_type& loc) {
        return hulk::common::Span {
            .start = { .index = 0,
                       .line   = static_cast<std::size_t>(loc.begin.line),
                       .column = static_cast<std::size_t>(loc.begin.column) },
            .end   = { .index = 0,
                       .line   = static_cast<std::size_t>(loc.end.line),
                       .column = static_cast<std::size_t>(loc.end.column) },
        };
    }


#line 62 "src/parser/parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 4 "src/parser/grammar.y"
namespace hulk { namespace parser {
#line 155 "src/parser/parser.cpp"

  /// Build a parser object.
  Parser::Parser (hulk::parser::ParserDriver& driver_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg)
  {}

  Parser::~Parser ()
  {}

  Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Parser::symbol_kind_type
  Parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_binding_list: // binding_list
        value.YY_MOVE_OR_COPY< BindingList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_binding: // binding
        value.YY_MOVE_OR_COPY< BindingPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_decl: // decl
      case symbol_kind::S_function_decl: // function_decl
      case symbol_kind::S_type_decl: // type_decl
      case symbol_kind::S_protocol_decl: // protocol_decl
        value.YY_MOVE_OR_COPY< DeclPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_elif_clauses: // elif_clauses
        value.YY_MOVE_OR_COPY< ElifList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_parent_args_opt: // parent_args_opt
      case symbol_kind::S_args_opt: // args_opt
      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_block_body_opt: // block_body_opt
      case symbol_kind::S_expr_list: // expr_list
        value.YY_MOVE_OR_COPY< ExprList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_lambda_expr: // lambda_expr
      case symbol_kind::S_let_expr: // let_expr
      case symbol_kind::S_if_expr: // if_expr
      case symbol_kind::S_while_expr: // while_expr
      case symbol_kind::S_for_expr: // for_expr
      case symbol_kind::S_assign_expr: // assign_expr
      case symbol_kind::S_logic_or: // logic_or
      case symbol_kind::S_logic_and: // logic_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relation: // relation
      case symbol_kind::S_type_test_expr: // type_test_expr
      case symbol_kind::S_concat: // concat
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_block: // block
        value.YY_MOVE_OR_COPY< ExprPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param: // param
      case symbol_kind::S_lambda_param: // lambda_param
        value.YY_MOVE_OR_COPY< Hulk::Param > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_protocol_member: // protocol_member
        value.YY_MOVE_OR_COPY< Hulk::ProtocolMethodSig > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_type_member: // type_member
        value.YY_MOVE_OR_COPY< Hulk::TypeMember > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ctor_params_opt: // ctor_params_opt
      case symbol_kind::S_params_opt: // params_opt
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_lambda_param_list: // lambda_param_list
        value.YY_MOVE_OR_COPY< ParamList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.YY_MOVE_OR_COPY< ProgramPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_protocol_member_list: // protocol_member_list
        value.YY_MOVE_OR_COPY< ProtocolMethodList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_type_member_list: // type_member_list
        value.YY_MOVE_OR_COPY< TypeMemberList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER_LITERAL: // NUMBER_LITERAL
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_protocol_extends_opt: // protocol_extends_opt
      case symbol_kind::S_inherits_opt: // inherits_opt
        value.YY_MOVE_OR_COPY< hulk::parser::InheritsInfo > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_lvalue: // lvalue
        value.YY_MOVE_OR_COPY< hulk::parser::LValueTarget > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_top_level_items: // top_level_items
      case symbol_kind::S_top_level_item: // top_level_item
        value.YY_MOVE_OR_COPY< hulk::parser::TopLevelItems > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
      case symbol_kind::S_ERROR_TOKEN: // ERROR_TOKEN
      case symbol_kind::S_return_ann_opt: // return_ann_opt
      case symbol_kind::S_type_ann_opt: // type_ann_opt
      case symbol_kind::S_type_expr: // type_expr
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_binding_list: // binding_list
        value.move< BindingList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_binding: // binding
        value.move< BindingPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_decl: // decl
      case symbol_kind::S_function_decl: // function_decl
      case symbol_kind::S_type_decl: // type_decl
      case symbol_kind::S_protocol_decl: // protocol_decl
        value.move< DeclPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_elif_clauses: // elif_clauses
        value.move< ElifList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_parent_args_opt: // parent_args_opt
      case symbol_kind::S_args_opt: // args_opt
      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_block_body_opt: // block_body_opt
      case symbol_kind::S_expr_list: // expr_list
        value.move< ExprList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_lambda_expr: // lambda_expr
      case symbol_kind::S_let_expr: // let_expr
      case symbol_kind::S_if_expr: // if_expr
      case symbol_kind::S_while_expr: // while_expr
      case symbol_kind::S_for_expr: // for_expr
      case symbol_kind::S_assign_expr: // assign_expr
      case symbol_kind::S_logic_or: // logic_or
      case symbol_kind::S_logic_and: // logic_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relation: // relation
      case symbol_kind::S_type_test_expr: // type_test_expr
      case symbol_kind::S_concat: // concat
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_block: // block
        value.move< ExprPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_param: // param
      case symbol_kind::S_lambda_param: // lambda_param
        value.move< Hulk::Param > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_protocol_member: // protocol_member
        value.move< Hulk::ProtocolMethodSig > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_type_member: // type_member
        value.move< Hulk::TypeMember > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_ctor_params_opt: // ctor_params_opt
      case symbol_kind::S_params_opt: // params_opt
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_lambda_param_list: // lambda_param_list
        value.move< ParamList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_program: // program
        value.move< ProgramPtr > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_protocol_member_list: // protocol_member_list
        value.move< ProtocolMethodList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_type_member_list: // type_member_list
        value.move< TypeMemberList > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_NUMBER_LITERAL: // NUMBER_LITERAL
        value.move< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_protocol_extends_opt: // protocol_extends_opt
      case symbol_kind::S_inherits_opt: // inherits_opt
        value.move< hulk::parser::InheritsInfo > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_lvalue: // lvalue
        value.move< hulk::parser::LValueTarget > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_top_level_items: // top_level_items
      case symbol_kind::S_top_level_item: // top_level_item
        value.move< hulk::parser::TopLevelItems > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
      case symbol_kind::S_ERROR_TOKEN: // ERROR_TOKEN
      case symbol_kind::S_return_ann_opt: // return_ann_opt
      case symbol_kind::S_type_ann_opt: // type_ann_opt
      case symbol_kind::S_type_expr: // type_expr
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_binding_list: // binding_list
        value.copy< BindingList > (that.value);
        break;

      case symbol_kind::S_binding: // binding
        value.copy< BindingPtr > (that.value);
        break;

      case symbol_kind::S_decl: // decl
      case symbol_kind::S_function_decl: // function_decl
      case symbol_kind::S_type_decl: // type_decl
      case symbol_kind::S_protocol_decl: // protocol_decl
        value.copy< DeclPtr > (that.value);
        break;

      case symbol_kind::S_elif_clauses: // elif_clauses
        value.copy< ElifList > (that.value);
        break;

      case symbol_kind::S_parent_args_opt: // parent_args_opt
      case symbol_kind::S_args_opt: // args_opt
      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_block_body_opt: // block_body_opt
      case symbol_kind::S_expr_list: // expr_list
        value.copy< ExprList > (that.value);
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_lambda_expr: // lambda_expr
      case symbol_kind::S_let_expr: // let_expr
      case symbol_kind::S_if_expr: // if_expr
      case symbol_kind::S_while_expr: // while_expr
      case symbol_kind::S_for_expr: // for_expr
      case symbol_kind::S_assign_expr: // assign_expr
      case symbol_kind::S_logic_or: // logic_or
      case symbol_kind::S_logic_and: // logic_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relation: // relation
      case symbol_kind::S_type_test_expr: // type_test_expr
      case symbol_kind::S_concat: // concat
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_block: // block
        value.copy< ExprPtr > (that.value);
        break;

      case symbol_kind::S_param: // param
      case symbol_kind::S_lambda_param: // lambda_param
        value.copy< Hulk::Param > (that.value);
        break;

      case symbol_kind::S_protocol_member: // protocol_member
        value.copy< Hulk::ProtocolMethodSig > (that.value);
        break;

      case symbol_kind::S_type_member: // type_member
        value.copy< Hulk::TypeMember > (that.value);
        break;

      case symbol_kind::S_ctor_params_opt: // ctor_params_opt
      case symbol_kind::S_params_opt: // params_opt
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_lambda_param_list: // lambda_param_list
        value.copy< ParamList > (that.value);
        break;

      case symbol_kind::S_program: // program
        value.copy< ProgramPtr > (that.value);
        break;

      case symbol_kind::S_protocol_member_list: // protocol_member_list
        value.copy< ProtocolMethodList > (that.value);
        break;

      case symbol_kind::S_type_member_list: // type_member_list
        value.copy< TypeMemberList > (that.value);
        break;

      case symbol_kind::S_NUMBER_LITERAL: // NUMBER_LITERAL
        value.copy< double > (that.value);
        break;

      case symbol_kind::S_protocol_extends_opt: // protocol_extends_opt
      case symbol_kind::S_inherits_opt: // inherits_opt
        value.copy< hulk::parser::InheritsInfo > (that.value);
        break;

      case symbol_kind::S_lvalue: // lvalue
        value.copy< hulk::parser::LValueTarget > (that.value);
        break;

      case symbol_kind::S_top_level_items: // top_level_items
      case symbol_kind::S_top_level_item: // top_level_item
        value.copy< hulk::parser::TopLevelItems > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
      case symbol_kind::S_ERROR_TOKEN: // ERROR_TOKEN
      case symbol_kind::S_return_ann_opt: // return_ann_opt
      case symbol_kind::S_type_ann_opt: // type_ann_opt
      case symbol_kind::S_type_expr: // type_expr
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_binding_list: // binding_list
        value.move< BindingList > (that.value);
        break;

      case symbol_kind::S_binding: // binding
        value.move< BindingPtr > (that.value);
        break;

      case symbol_kind::S_decl: // decl
      case symbol_kind::S_function_decl: // function_decl
      case symbol_kind::S_type_decl: // type_decl
      case symbol_kind::S_protocol_decl: // protocol_decl
        value.move< DeclPtr > (that.value);
        break;

      case symbol_kind::S_elif_clauses: // elif_clauses
        value.move< ElifList > (that.value);
        break;

      case symbol_kind::S_parent_args_opt: // parent_args_opt
      case symbol_kind::S_args_opt: // args_opt
      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_block_body_opt: // block_body_opt
      case symbol_kind::S_expr_list: // expr_list
        value.move< ExprList > (that.value);
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_lambda_expr: // lambda_expr
      case symbol_kind::S_let_expr: // let_expr
      case symbol_kind::S_if_expr: // if_expr
      case symbol_kind::S_while_expr: // while_expr
      case symbol_kind::S_for_expr: // for_expr
      case symbol_kind::S_assign_expr: // assign_expr
      case symbol_kind::S_logic_or: // logic_or
      case symbol_kind::S_logic_and: // logic_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relation: // relation
      case symbol_kind::S_type_test_expr: // type_test_expr
      case symbol_kind::S_concat: // concat
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_block: // block
        value.move< ExprPtr > (that.value);
        break;

      case symbol_kind::S_param: // param
      case symbol_kind::S_lambda_param: // lambda_param
        value.move< Hulk::Param > (that.value);
        break;

      case symbol_kind::S_protocol_member: // protocol_member
        value.move< Hulk::ProtocolMethodSig > (that.value);
        break;

      case symbol_kind::S_type_member: // type_member
        value.move< Hulk::TypeMember > (that.value);
        break;

      case symbol_kind::S_ctor_params_opt: // ctor_params_opt
      case symbol_kind::S_params_opt: // params_opt
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_lambda_param_list: // lambda_param_list
        value.move< ParamList > (that.value);
        break;

      case symbol_kind::S_program: // program
        value.move< ProgramPtr > (that.value);
        break;

      case symbol_kind::S_protocol_member_list: // protocol_member_list
        value.move< ProtocolMethodList > (that.value);
        break;

      case symbol_kind::S_type_member_list: // type_member_list
        value.move< TypeMemberList > (that.value);
        break;

      case symbol_kind::S_NUMBER_LITERAL: // NUMBER_LITERAL
        value.move< double > (that.value);
        break;

      case symbol_kind::S_protocol_extends_opt: // protocol_extends_opt
      case symbol_kind::S_inherits_opt: // inherits_opt
        value.move< hulk::parser::InheritsInfo > (that.value);
        break;

      case symbol_kind::S_lvalue: // lvalue
        value.move< hulk::parser::LValueTarget > (that.value);
        break;

      case symbol_kind::S_top_level_items: // top_level_items
      case symbol_kind::S_top_level_item: // top_level_item
        value.move< hulk::parser::TopLevelItems > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
      case symbol_kind::S_ERROR_TOKEN: // ERROR_TOKEN
      case symbol_kind::S_return_ann_opt: // return_ann_opt
      case symbol_kind::S_type_ann_opt: // type_ann_opt
      case symbol_kind::S_type_expr: // type_expr
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  Parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  Parser::state_type
  Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  Parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::operator() ()
  {
    return parse ();
  }

  int
  Parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex ());
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_binding_list: // binding_list
        yylhs.value.emplace< BindingList > ();
        break;

      case symbol_kind::S_binding: // binding
        yylhs.value.emplace< BindingPtr > ();
        break;

      case symbol_kind::S_decl: // decl
      case symbol_kind::S_function_decl: // function_decl
      case symbol_kind::S_type_decl: // type_decl
      case symbol_kind::S_protocol_decl: // protocol_decl
        yylhs.value.emplace< DeclPtr > ();
        break;

      case symbol_kind::S_elif_clauses: // elif_clauses
        yylhs.value.emplace< ElifList > ();
        break;

      case symbol_kind::S_parent_args_opt: // parent_args_opt
      case symbol_kind::S_args_opt: // args_opt
      case symbol_kind::S_arg_list: // arg_list
      case symbol_kind::S_block_body_opt: // block_body_opt
      case symbol_kind::S_expr_list: // expr_list
        yylhs.value.emplace< ExprList > ();
        break;

      case symbol_kind::S_expr: // expr
      case symbol_kind::S_lambda_expr: // lambda_expr
      case symbol_kind::S_let_expr: // let_expr
      case symbol_kind::S_if_expr: // if_expr
      case symbol_kind::S_while_expr: // while_expr
      case symbol_kind::S_for_expr: // for_expr
      case symbol_kind::S_assign_expr: // assign_expr
      case symbol_kind::S_logic_or: // logic_or
      case symbol_kind::S_logic_and: // logic_and
      case symbol_kind::S_equality: // equality
      case symbol_kind::S_relation: // relation
      case symbol_kind::S_type_test_expr: // type_test_expr
      case symbol_kind::S_concat: // concat
      case symbol_kind::S_additive: // additive
      case symbol_kind::S_multiplicative: // multiplicative
      case symbol_kind::S_power: // power
      case symbol_kind::S_unary: // unary
      case symbol_kind::S_postfix: // postfix
      case symbol_kind::S_primary: // primary
      case symbol_kind::S_block: // block
        yylhs.value.emplace< ExprPtr > ();
        break;

      case symbol_kind::S_param: // param
      case symbol_kind::S_lambda_param: // lambda_param
        yylhs.value.emplace< Hulk::Param > ();
        break;

      case symbol_kind::S_protocol_member: // protocol_member
        yylhs.value.emplace< Hulk::ProtocolMethodSig > ();
        break;

      case symbol_kind::S_type_member: // type_member
        yylhs.value.emplace< Hulk::TypeMember > ();
        break;

      case symbol_kind::S_ctor_params_opt: // ctor_params_opt
      case symbol_kind::S_params_opt: // params_opt
      case symbol_kind::S_param_list: // param_list
      case symbol_kind::S_lambda_param_list: // lambda_param_list
        yylhs.value.emplace< ParamList > ();
        break;

      case symbol_kind::S_program: // program
        yylhs.value.emplace< ProgramPtr > ();
        break;

      case symbol_kind::S_protocol_member_list: // protocol_member_list
        yylhs.value.emplace< ProtocolMethodList > ();
        break;

      case symbol_kind::S_type_member_list: // type_member_list
        yylhs.value.emplace< TypeMemberList > ();
        break;

      case symbol_kind::S_NUMBER_LITERAL: // NUMBER_LITERAL
        yylhs.value.emplace< double > ();
        break;

      case symbol_kind::S_protocol_extends_opt: // protocol_extends_opt
      case symbol_kind::S_inherits_opt: // inherits_opt
        yylhs.value.emplace< hulk::parser::InheritsInfo > ();
        break;

      case symbol_kind::S_lvalue: // lvalue
        yylhs.value.emplace< hulk::parser::LValueTarget > ();
        break;

      case symbol_kind::S_top_level_items: // top_level_items
      case symbol_kind::S_top_level_item: // top_level_item
        yylhs.value.emplace< hulk::parser::TopLevelItems > ();
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
      case symbol_kind::S_ERROR_TOKEN: // ERROR_TOKEN
      case symbol_kind::S_return_ann_opt: // return_ann_opt
      case symbol_kind::S_type_ann_opt: // type_ann_opt
      case symbol_kind::S_type_expr: // type_expr
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // program: top_level_items
#line 170 "src/parser/grammar.y"
      {
          if (!yystack_[0].value.as < hulk::parser::TopLevelItems > ().hasGlobalExpr) {
              driver.report_syntax_error("el programa debe contener una expresion global final");
              yylhs.value.as < ProgramPtr > () = std::make_unique<Hulk::Program>(
                  std::move(yystack_[0].value.as < hulk::parser::TopLevelItems > ().decls),
                  std::make_unique<Hulk::ExprBlock>(ExprList {})
              );
          } else {
              yylhs.value.as < ProgramPtr > () = std::make_unique<Hulk::Program>(
                  std::move(yystack_[0].value.as < hulk::parser::TopLevelItems > ().decls),
                  std::move(yystack_[0].value.as < hulk::parser::TopLevelItems > ().globalExpr)
              );
          }
          driver.set_result(std::move(yylhs.value.as < ProgramPtr > ()));
      }
#line 1108 "src/parser/parser.cpp"
    break;

  case 3: // top_level_items: top_level_item opt_semi
#line 189 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = std::move(yystack_[1].value.as < hulk::parser::TopLevelItems > ());
      }
#line 1116 "src/parser/parser.cpp"
    break;

  case 4: // top_level_items: top_level_items top_level_item opt_semi
#line 193 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < hulk::parser::TopLevelItems > ().hasGlobalExpr) {
              driver.report_syntax_error("Solo se permite una expresion global final", to_span(yystack_[1].location));
          } else if (yystack_[1].value.as < hulk::parser::TopLevelItems > ().hasGlobalExpr) {
              yystack_[2].value.as < hulk::parser::TopLevelItems > ().globalExpr = std::move(yystack_[1].value.as < hulk::parser::TopLevelItems > ().globalExpr);
              yystack_[2].value.as < hulk::parser::TopLevelItems > ().hasGlobalExpr = true;
          } else {
              for (auto& decl : yystack_[1].value.as < hulk::parser::TopLevelItems > ().decls) {
                  yystack_[2].value.as < hulk::parser::TopLevelItems > ().decls.push_back(std::move(decl));
              }
          }
          yylhs.value.as < hulk::parser::TopLevelItems > () = std::move(yystack_[2].value.as < hulk::parser::TopLevelItems > ());
      }
#line 1134 "src/parser/parser.cpp"
    break;

  case 5: // top_level_item: decl
#line 210 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = hulk::parser::TopLevelItems {};
          yylhs.value.as < hulk::parser::TopLevelItems > ().decls.push_back(std::move(yystack_[0].value.as < DeclPtr > ()));
      }
#line 1143 "src/parser/parser.cpp"
    break;

  case 6: // top_level_item: expr
#line 215 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = hulk::parser::TopLevelItems {};
          yylhs.value.as < hulk::parser::TopLevelItems > ().globalExpr = std::move(yystack_[0].value.as < ExprPtr > ());
          yylhs.value.as < hulk::parser::TopLevelItems > ().hasGlobalExpr = true;
      }
#line 1153 "src/parser/parser.cpp"
    break;

  case 9: // decl: function_decl
#line 229 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1161 "src/parser/parser.cpp"
    break;

  case 10: // decl: type_decl
#line 233 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1169 "src/parser/parser.cpp"
    break;

  case 11: // decl: protocol_decl
#line 237 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1177 "src/parser/parser.cpp"
    break;

  case 12: // function_decl: FUNCTION IDENTIFIER LPAREN params_opt RPAREN return_ann_opt FATARROW expr SEMICOLON
#line 244 "src/parser/grammar.y"
      {
          if (yystack_[3].value.as < std::string > ().empty()) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[7].value.as < std::string > (), std::move(yystack_[5].value.as < ParamList > ()), std::move(yystack_[1].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[7].value.as < std::string > (), std::move(yystack_[5].value.as < ParamList > ()), yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < ExprPtr > ()));
          }
          yylhs.value.as < DeclPtr > ()->span = to_span(yylhs.location);
      }
#line 1190 "src/parser/parser.cpp"
    break;

  case 13: // function_decl: FUNCTION IDENTIFIER LPAREN params_opt RPAREN return_ann_opt block
#line 253 "src/parser/grammar.y"
      {
          if (yystack_[1].value.as < std::string > ().empty()) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), yystack_[1].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < DeclPtr > ()->span = to_span(yylhs.location);
      }
#line 1203 "src/parser/parser.cpp"
    break;

  case 14: // type_decl: TYPE IDENTIFIER ctor_params_opt inherits_opt LBRACE type_member_list RBRACE
#line 265 "src/parser/grammar.y"
      {
          if (yystack_[4].value.as < ParamList > ().empty() && !yystack_[3].value.as < hulk::parser::InheritsInfo > ().hasParent) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::TypeDecl>(yystack_[5].value.as < std::string > (), std::move(yystack_[1].value.as < TypeMemberList > ()));
          } else if (!yystack_[4].value.as < ParamList > ().empty() && !yystack_[3].value.as < hulk::parser::InheritsInfo > ().hasParent) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::TypeDecl>(yystack_[5].value.as < std::string > (), std::move(yystack_[4].value.as < ParamList > ()), std::move(yystack_[1].value.as < TypeMemberList > ()));
          } else if (yystack_[4].value.as < ParamList > ().empty() && yystack_[3].value.as < hulk::parser::InheritsInfo > ().hasParent && yystack_[3].value.as < hulk::parser::InheritsInfo > ().parentArgs.empty()) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::TypeDecl>(yystack_[5].value.as < std::string > (), yystack_[3].value.as < hulk::parser::InheritsInfo > ().parentName, std::move(yystack_[1].value.as < TypeMemberList > ()));
          } else {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::TypeDecl>(
                  yystack_[5].value.as < std::string > (),
                  std::move(yystack_[4].value.as < ParamList > ()),
                  yystack_[3].value.as < hulk::parser::InheritsInfo > ().parentName,
                  std::move(yystack_[3].value.as < hulk::parser::InheritsInfo > ().parentArgs),
                  std::move(yystack_[1].value.as < TypeMemberList > ())
              );
          }
          yylhs.value.as < DeclPtr > ()->span = to_span(yylhs.location);
      }
#line 1226 "src/parser/parser.cpp"
    break;

  case 15: // protocol_decl: PROTOCOL IDENTIFIER protocol_extends_opt LBRACE protocol_member_list RBRACE
#line 287 "src/parser/grammar.y"
      {
          if (yystack_[3].value.as < hulk::parser::InheritsInfo > ().hasParent) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::ProtocolDecl>(yystack_[4].value.as < std::string > (), yystack_[3].value.as < hulk::parser::InheritsInfo > ().parentName, std::move(yystack_[1].value.as < ProtocolMethodList > ()));
          } else {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::ProtocolDecl>(yystack_[4].value.as < std::string > (), std::move(yystack_[1].value.as < ProtocolMethodList > ()));
          }
          yylhs.value.as < DeclPtr > ()->span = to_span(yylhs.location);
      }
#line 1239 "src/parser/parser.cpp"
    break;

  case 16: // protocol_extends_opt: EXTENDS IDENTIFIER
#line 299 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo { yystack_[0].value.as < std::string > (), ExprList {}, true };
      }
#line 1247 "src/parser/parser.cpp"
    break;

  case 17: // protocol_extends_opt: %empty
#line 303 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo {};
      }
#line 1255 "src/parser/parser.cpp"
    break;

  case 18: // protocol_member_list: %empty
#line 310 "src/parser/grammar.y"
      {
          yylhs.value.as < ProtocolMethodList > () = hulk::parser::ProtocolMethodList {};
      }
#line 1263 "src/parser/parser.cpp"
    break;

  case 19: // protocol_member_list: protocol_member_list protocol_member
#line 314 "src/parser/grammar.y"
      {
          yystack_[1].value.as < ProtocolMethodList > ().push_back(std::move(yystack_[0].value.as < Hulk::ProtocolMethodSig > ()));
          yylhs.value.as < ProtocolMethodList > () = std::move(yystack_[1].value.as < ProtocolMethodList > ());
      }
#line 1272 "src/parser/parser.cpp"
    break;

  case 20: // protocol_member: IDENTIFIER LPAREN params_opt RPAREN return_ann_opt SEMICOLON
#line 322 "src/parser/grammar.y"
      {
          yylhs.value.as < Hulk::ProtocolMethodSig > () = Hulk::ProtocolMethodSig(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), yystack_[1].value.as < std::string > ());
      }
#line 1280 "src/parser/parser.cpp"
    break;

  case 21: // ctor_params_opt: LPAREN params_opt RPAREN
#line 329 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = std::move(yystack_[1].value.as < ParamList > ());
      }
#line 1288 "src/parser/parser.cpp"
    break;

  case 22: // ctor_params_opt: %empty
#line 333 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = ParamList {};
      }
#line 1296 "src/parser/parser.cpp"
    break;

  case 23: // inherits_opt: INHERITS IDENTIFIER parent_args_opt
#line 340 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo { yystack_[1].value.as < std::string > (), std::move(yystack_[0].value.as < ExprList > ()), true };
      }
#line 1304 "src/parser/parser.cpp"
    break;

  case 24: // inherits_opt: %empty
#line 344 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo {};
      }
#line 1312 "src/parser/parser.cpp"
    break;

  case 25: // parent_args_opt: LPAREN args_opt RPAREN
#line 351 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[1].value.as < ExprList > ());
      }
#line 1320 "src/parser/parser.cpp"
    break;

  case 26: // parent_args_opt: %empty
#line 355 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 1328 "src/parser/parser.cpp"
    break;

  case 27: // type_member_list: %empty
#line 362 "src/parser/grammar.y"
      {
          yylhs.value.as < TypeMemberList > () = TypeMemberList {};
      }
#line 1336 "src/parser/parser.cpp"
    break;

  case 28: // type_member_list: type_member_list type_member
#line 366 "src/parser/grammar.y"
      {
          yystack_[1].value.as < TypeMemberList > ().push_back(std::move(yystack_[0].value.as < Hulk::TypeMember > ()));
          yylhs.value.as < TypeMemberList > () = std::move(yystack_[1].value.as < TypeMemberList > ());
      }
#line 1345 "src/parser/parser.cpp"
    break;

  case 29: // type_member: IDENTIFIER type_ann_opt ASSIGN expr SEMICOLON
#line 374 "src/parser/grammar.y"
      {
          if (yystack_[3].value.as < std::string > ().empty()) {
              yylhs.value.as < Hulk::TypeMember > () = Hulk::TypeMember(
                  Hulk::TypeMember::Kind::Attribute,
                  std::make_unique<Hulk::TypeMemberAttribute>(yystack_[4].value.as < std::string > (), std::move(yystack_[1].value.as < ExprPtr > ()))
              );
          } else {
              yylhs.value.as < Hulk::TypeMember > () = Hulk::TypeMember(
                  Hulk::TypeMember::Kind::Attribute,
                  std::make_unique<Hulk::TypeMemberAttribute>(yystack_[4].value.as < std::string > (), yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < ExprPtr > ()))
              );
          }
          yylhs.value.as < Hulk::TypeMember > ().node->span = to_span(yylhs.location);
      }
#line 1364 "src/parser/parser.cpp"
    break;

  case 30: // type_member: IDENTIFIER LPAREN params_opt RPAREN return_ann_opt FATARROW expr SEMICOLON
#line 389 "src/parser/grammar.y"
      {
          if (yystack_[3].value.as < std::string > ().empty()) {
              yylhs.value.as < Hulk::TypeMember > () = Hulk::TypeMember(
                  Hulk::TypeMember::Kind::Method,
                  std::make_unique<Hulk::TypeMemberMethod>(yystack_[7].value.as < std::string > (), std::move(yystack_[5].value.as < ParamList > ()), std::move(yystack_[1].value.as < ExprPtr > ()))
              );
          } else {
              yylhs.value.as < Hulk::TypeMember > () = Hulk::TypeMember(
                  Hulk::TypeMember::Kind::Method,
                  std::make_unique<Hulk::TypeMemberMethod>(yystack_[7].value.as < std::string > (), std::move(yystack_[5].value.as < ParamList > ()), yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < ExprPtr > ()))
              );
          }
          yylhs.value.as < Hulk::TypeMember > ().node->span = to_span(yylhs.location);
      }
#line 1383 "src/parser/parser.cpp"
    break;

  case 31: // type_member: IDENTIFIER LPAREN params_opt RPAREN return_ann_opt block
#line 404 "src/parser/grammar.y"
      {
          if (yystack_[1].value.as < std::string > ().empty()) {
              yylhs.value.as < Hulk::TypeMember > () = Hulk::TypeMember(
                  Hulk::TypeMember::Kind::Method,
                  std::make_unique<Hulk::TypeMemberMethod>(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), std::move(yystack_[0].value.as < ExprPtr > ()))
              );
          } else {
              yylhs.value.as < Hulk::TypeMember > () = Hulk::TypeMember(
                  Hulk::TypeMember::Kind::Method,
                  std::make_unique<Hulk::TypeMemberMethod>(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), yystack_[1].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()))
              );
          }
          yylhs.value.as < Hulk::TypeMember > ().node->span = to_span(yylhs.location);
      }
#line 1402 "src/parser/parser.cpp"
    break;

  case 32: // params_opt: param_list
#line 422 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = std::move(yystack_[0].value.as < ParamList > ());
      }
#line 1410 "src/parser/parser.cpp"
    break;

  case 33: // params_opt: %empty
#line 426 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = ParamList {};
      }
#line 1418 "src/parser/parser.cpp"
    break;

  case 34: // param_list: param
#line 433 "src/parser/grammar.y"
      {
          ParamList params;
          params.push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(params);
      }
#line 1428 "src/parser/parser.cpp"
    break;

  case 35: // param_list: param_list COMMA param
#line 439 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ParamList > ().push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(yystack_[2].value.as < ParamList > ());
      }
#line 1437 "src/parser/parser.cpp"
    break;

  case 36: // param: IDENTIFIER type_ann_opt
#line 447 "src/parser/grammar.y"
      {
          if (yystack_[0].value.as < std::string > ().empty()) {
              yylhs.value.as < Hulk::Param > () = Hulk::Param(yystack_[1].value.as < std::string > ());
          } else {
              yylhs.value.as < Hulk::Param > () = Hulk::Param(yystack_[1].value.as < std::string > (), yystack_[0].value.as < std::string > ());
          }
      }
#line 1449 "src/parser/parser.cpp"
    break;

  case 37: // return_ann_opt: COLON type_expr
#line 458 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1457 "src/parser/parser.cpp"
    break;

  case 38: // return_ann_opt: %empty
#line 462 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "";
      }
#line 1465 "src/parser/parser.cpp"
    break;

  case 39: // type_ann_opt: COLON type_expr
#line 469 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1473 "src/parser/parser.cpp"
    break;

  case 40: // type_ann_opt: %empty
#line 473 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "";
      }
#line 1481 "src/parser/parser.cpp"
    break;

  case 41: // type_expr: IDENTIFIER
#line 480 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1489 "src/parser/parser.cpp"
    break;

  case 42: // type_expr: type_expr STAR
#line 484 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[1].value.as < std::string > ()) + "*";
      }
#line 1497 "src/parser/parser.cpp"
    break;

  case 43: // type_expr: UNDERSCORE
#line 488 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "_";
      }
#line 1505 "src/parser/parser.cpp"
    break;

  case 44: // type_expr: AUTO
#line 492 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "auto";
      }
#line 1513 "src/parser/parser.cpp"
    break;

  case 45: // expr: lambda_expr
#line 499 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1521 "src/parser/parser.cpp"
    break;

  case 46: // expr: let_expr
#line 503 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1529 "src/parser/parser.cpp"
    break;

  case 47: // expr: if_expr
#line 507 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1537 "src/parser/parser.cpp"
    break;

  case 48: // expr: while_expr
#line 511 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1545 "src/parser/parser.cpp"
    break;

  case 49: // expr: for_expr
#line 515 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1553 "src/parser/parser.cpp"
    break;

  case 50: // expr: assign_expr
#line 519 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1561 "src/parser/parser.cpp"
    break;

  case 51: // lambda_expr: LPAREN lambda_param_list RPAREN return_ann_opt FATARROW expr
#line 526 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < std::string > ().empty()) {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Lambda>(std::move(yystack_[4].value.as < ParamList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Lambda>(std::move(yystack_[4].value.as < ParamList > ()), yystack_[2].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1574 "src/parser/parser.cpp"
    break;

  case 52: // lambda_param_list: lambda_param
#line 538 "src/parser/grammar.y"
      {
          ParamList params;
          params.push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(params);
      }
#line 1584 "src/parser/parser.cpp"
    break;

  case 53: // lambda_param_list: lambda_param_list COMMA lambda_param
#line 544 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ParamList > ().push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(yystack_[2].value.as < ParamList > ());
      }
#line 1593 "src/parser/parser.cpp"
    break;

  case 54: // lambda_param: IDENTIFIER COLON type_expr
#line 552 "src/parser/grammar.y"
      {
          yylhs.value.as < Hulk::Param > () = Hulk::Param(yystack_[2].value.as < std::string > (), yystack_[0].value.as < std::string > ());
      }
#line 1601 "src/parser/parser.cpp"
    break;

  case 55: // let_expr: LET binding_list IN expr
#line 559 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LetIn>(std::move(yystack_[2].value.as < BindingList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1610 "src/parser/parser.cpp"
    break;

  case 56: // binding_list: binding
#line 567 "src/parser/grammar.y"
      {
          BindingList bindings;
          bindings.push_back(std::move(yystack_[0].value.as < BindingPtr > ()));
          yylhs.value.as < BindingList > () = std::move(bindings);
      }
#line 1620 "src/parser/parser.cpp"
    break;

  case 57: // binding_list: binding_list COMMA binding
#line 573 "src/parser/grammar.y"
      {
          yystack_[2].value.as < BindingList > ().push_back(std::move(yystack_[0].value.as < BindingPtr > ()));
          yylhs.value.as < BindingList > () = std::move(yystack_[2].value.as < BindingList > ());
      }
#line 1629 "src/parser/parser.cpp"
    break;

  case 58: // binding: IDENTIFIER type_ann_opt ASSIGN expr
#line 581 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < std::string > ().empty()) {
              yylhs.value.as < BindingPtr > () = std::make_unique<Hulk::VariableBinding>(yystack_[3].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < BindingPtr > () = std::make_unique<Hulk::VariableBinding>(yystack_[3].value.as < std::string > (), yystack_[2].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < BindingPtr > ()->span = to_span(yylhs.location);
      }
#line 1642 "src/parser/parser.cpp"
    break;

  case 59: // if_expr: IF LPAREN expr RPAREN expr elif_clauses ELSE expr
#line 593 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::IfStmt>(std::move(yystack_[5].value.as < ExprPtr > ()), std::move(yystack_[3].value.as < ExprPtr > ()), std::move(yystack_[2].value.as < ElifList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1651 "src/parser/parser.cpp"
    break;

  case 60: // elif_clauses: %empty
#line 601 "src/parser/grammar.y"
      {
          yylhs.value.as < ElifList > () = ElifList {};
      }
#line 1659 "src/parser/parser.cpp"
    break;

  case 61: // elif_clauses: elif_clauses ELIF LPAREN expr RPAREN expr
#line 605 "src/parser/grammar.y"
      {
          yystack_[5].value.as < ElifList > ().emplace_back(std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ElifList > () = std::move(yystack_[5].value.as < ElifList > ());
      }
#line 1668 "src/parser/parser.cpp"
    break;

  case 62: // while_expr: WHILE LPAREN expr RPAREN expr
#line 613 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::WhileStmt>(std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1677 "src/parser/parser.cpp"
    break;

  case 63: // for_expr: FOR LPAREN IDENTIFIER IN expr RPAREN expr
#line 621 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::For>(yystack_[4].value.as < std::string > (), std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1686 "src/parser/parser.cpp"
    break;

  case 64: // assign_expr: lvalue DESTRUCTIVE_ASSIGN expr
#line 629 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < hulk::parser::LValueTarget > ().isMember) {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::DestructiveAssignMember>(std::move(yystack_[2].value.as < hulk::parser::LValueTarget > ().object), yystack_[2].value.as < hulk::parser::LValueTarget > ().name, std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::DestructiveAssign>(yystack_[2].value.as < hulk::parser::LValueTarget > ().name, std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1699 "src/parser/parser.cpp"
    break;

  case 65: // assign_expr: logic_or
#line 638 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1707 "src/parser/parser.cpp"
    break;

  case 66: // lvalue: IDENTIFIER
#line 645 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { nullptr, yystack_[0].value.as < std::string > (), false };
      }
#line 1715 "src/parser/parser.cpp"
    break;

  case 67: // lvalue: postfix DOT IDENTIFIER
#line 649 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > (), true };
      }
#line 1723 "src/parser/parser.cpp"
    break;

  case 68: // lvalue: SELF
#line 653 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { nullptr, "self", false };
      }
#line 1731 "src/parser/parser.cpp"
    break;

  case 69: // logic_or: logic_or OR logic_and
#line 660 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Or, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1742 "src/parser/parser.cpp"
    break;

  case 70: // logic_or: logic_and
#line 667 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1750 "src/parser/parser.cpp"
    break;

  case 71: // logic_and: logic_and AND equality
#line 674 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::And, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1761 "src/parser/parser.cpp"
    break;

  case 72: // logic_and: equality
#line 681 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1769 "src/parser/parser.cpp"
    break;

  case 73: // equality: equality EQUAL_EQUAL relation
#line 688 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Equal, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1780 "src/parser/parser.cpp"
    break;

  case 74: // equality: equality NOT_EQUAL relation
#line 695 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::NotEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1791 "src/parser/parser.cpp"
    break;

  case 75: // equality: relation
#line 702 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1799 "src/parser/parser.cpp"
    break;

  case 76: // relation: relation LESS type_test_expr
#line 709 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Less, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1810 "src/parser/parser.cpp"
    break;

  case 77: // relation: relation LESS_EQUAL type_test_expr
#line 716 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::LessEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1821 "src/parser/parser.cpp"
    break;

  case 78: // relation: relation GREATER type_test_expr
#line 723 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Greater, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1832 "src/parser/parser.cpp"
    break;

  case 79: // relation: relation GREATER_EQUAL type_test_expr
#line 730 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::GreaterEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1843 "src/parser/parser.cpp"
    break;

  case 80: // relation: type_test_expr
#line 737 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1851 "src/parser/parser.cpp"
    break;

  case 81: // type_test_expr: concat
#line 744 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1859 "src/parser/parser.cpp"
    break;

  case 82: // type_test_expr: concat IS type_expr
#line 748 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::IsExpr>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1868 "src/parser/parser.cpp"
    break;

  case 83: // type_test_expr: concat AS type_expr
#line 753 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::AsExpr>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1877 "src/parser/parser.cpp"
    break;

  case 84: // concat: concat CONCAT additive
#line 761 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::StringBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::StringOp::Concat, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1888 "src/parser/parser.cpp"
    break;

  case 85: // concat: concat DOUBLECONCAT additive
#line 768 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::StringBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::StringOp::SpaceConcat, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1899 "src/parser/parser.cpp"
    break;

  case 86: // concat: additive
#line 775 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1907 "src/parser/parser.cpp"
    break;

  case 87: // additive: additive PLUS multiplicative
#line 782 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Plus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1918 "src/parser/parser.cpp"
    break;

  case 88: // additive: additive MINUS multiplicative
#line 789 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Minus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1929 "src/parser/parser.cpp"
    break;

  case 89: // additive: multiplicative
#line 796 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1937 "src/parser/parser.cpp"
    break;

  case 90: // multiplicative: multiplicative STAR power
#line 803 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Mult, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1948 "src/parser/parser.cpp"
    break;

  case 91: // multiplicative: multiplicative SLASH power
#line 810 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Div, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1959 "src/parser/parser.cpp"
    break;

  case 92: // multiplicative: multiplicative PERCENT power
#line 817 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Mod, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1970 "src/parser/parser.cpp"
    break;

  case 93: // multiplicative: power
#line 824 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1978 "src/parser/parser.cpp"
    break;

  case 94: // power: unary CARET power
#line 831 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Pow, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1989 "src/parser/parser.cpp"
    break;

  case 95: // power: unary
#line 838 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1997 "src/parser/parser.cpp"
    break;

  case 96: // unary: MINUS unary
#line 845 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticUnaryOp>(
              Hulk::ArithUnaryType::Minus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2008 "src/parser/parser.cpp"
    break;

  case 97: // unary: NOT unary
#line 852 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicUnaryOp>(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2017 "src/parser/parser.cpp"
    break;

  case 98: // unary: postfix
#line 857 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 2025 "src/parser/parser.cpp"
    break;

  case 99: // postfix: primary
#line 864 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 2033 "src/parser/parser.cpp"
    break;

  case 100: // postfix: postfix LPAREN args_opt RPAREN
#line 868 "src/parser/grammar.y"
      {
          if (const auto* callee = dynamic_cast<const Hulk::VariableReference*>(yystack_[3].value.as < ExprPtr > ().get())) {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::FunctionCall>(callee->GetName(), std::move(yystack_[1].value.as < ExprList > ()));
          } else if (auto* access = dynamic_cast<Hulk::MemberAccess*>(yystack_[3].value.as < ExprPtr > ().get())) {
              auto object = access->TakeObject();
              auto memberName = access->TakeMemberName();
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::MethodCall>(std::move(object), memberName, std::move(yystack_[1].value.as < ExprList > ()));
          } else {
              driver.report_syntax_error("solo se pueden invocar identificadores o accesos a metodo");
              yylhs.value.as < ExprPtr > () = std::move(yystack_[3].value.as < ExprPtr > ());
          }
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2051 "src/parser/parser.cpp"
    break;

  case 101: // postfix: postfix DOT IDENTIFIER
#line 882 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::MemberAccess>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2060 "src/parser/parser.cpp"
    break;

  case 102: // primary: NUMBER_LITERAL
#line 890 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(yystack_[0].value.as < double > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2069 "src/parser/parser.cpp"
    break;

  case 103: // primary: STRING_LITERAL
#line 895 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::String>(yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2078 "src/parser/parser.cpp"
    break;

  case 104: // primary: TRUE
#line 900 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Boolean>(true);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2087 "src/parser/parser.cpp"
    break;

  case 105: // primary: FALSE
#line 905 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Boolean>(false);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2096 "src/parser/parser.cpp"
    break;

  case 106: // primary: IDENTIFIER
#line 910 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::VariableReference>(yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2105 "src/parser/parser.cpp"
    break;

  case 107: // primary: SELF
#line 915 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::SelfRef>();
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2114 "src/parser/parser.cpp"
    break;

  case 108: // primary: BASE LPAREN args_opt RPAREN
#line 920 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BaseCall>(std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2123 "src/parser/parser.cpp"
    break;

  case 109: // primary: NEW IDENTIFIER LPAREN args_opt RPAREN
#line 925 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::NewExpr>(yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2132 "src/parser/parser.cpp"
    break;

  case 110: // primary: LPAREN expr RPAREN
#line 930 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[1].value.as < ExprPtr > ());
      }
#line 2140 "src/parser/parser.cpp"
    break;

  case 111: // primary: block
#line 934 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 2148 "src/parser/parser.cpp"
    break;

  case 112: // primary: PRINT LPAREN expr RPAREN
#line 938 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Print>(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2157 "src/parser/parser.cpp"
    break;

  case 113: // primary: SQRT LPAREN expr RPAREN
#line 943 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Sqrt, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2168 "src/parser/parser.cpp"
    break;

  case 114: // primary: SIN LPAREN expr RPAREN
#line 950 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Sin, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2179 "src/parser/parser.cpp"
    break;

  case 115: // primary: COS LPAREN expr RPAREN
#line 957 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Cos, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2190 "src/parser/parser.cpp"
    break;

  case 116: // primary: RAND LPAREN RPAREN
#line 964 "src/parser/grammar.y"
      {
          ExprList args;
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Rand, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2200 "src/parser/parser.cpp"
    break;

  case 117: // primary: EXP LPAREN expr RPAREN
#line 970 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Exp, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2211 "src/parser/parser.cpp"
    break;

  case 118: // primary: LOG LPAREN expr COMMA expr RPAREN
#line 977 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[3].value.as < ExprPtr > ()));
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Log, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2223 "src/parser/parser.cpp"
    break;

  case 119: // primary: PI_CONST
#line 985 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(3.14159265358979323846);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2232 "src/parser/parser.cpp"
    break;

  case 120: // primary: E_CONST
#line 990 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(2.71828182845904523536);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2241 "src/parser/parser.cpp"
    break;

  case 121: // args_opt: arg_list
#line 998 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[0].value.as < ExprList > ());
      }
#line 2249 "src/parser/parser.cpp"
    break;

  case 122: // args_opt: %empty
#line 1002 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 2257 "src/parser/parser.cpp"
    break;

  case 123: // arg_list: expr
#line 1009 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(args);
      }
#line 2267 "src/parser/parser.cpp"
    break;

  case 124: // arg_list: arg_list COMMA expr
#line 1015 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ExprList > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(yystack_[2].value.as < ExprList > ());
      }
#line 2276 "src/parser/parser.cpp"
    break;

  case 125: // block: LBRACE block_body_opt RBRACE
#line 1023 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ExprBlock>(std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2285 "src/parser/parser.cpp"
    break;

  case 126: // block_body_opt: %empty
#line 1031 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 2293 "src/parser/parser.cpp"
    break;

  case 127: // block_body_opt: expr_list opt_semi
#line 1035 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[1].value.as < ExprList > ());
      }
#line 2301 "src/parser/parser.cpp"
    break;

  case 128: // expr_list: expr
#line 1042 "src/parser/grammar.y"
      {
          ExprList nodes;
          nodes.push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(nodes);
      }
#line 2311 "src/parser/parser.cpp"
    break;

  case 129: // expr_list: expr_list SEMICOLON expr
#line 1048 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ExprList > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(yystack_[2].value.as < ExprList > ());
      }
#line 2320 "src/parser/parser.cpp"
    break;


#line 2324 "src/parser/parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
  Parser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "END", "error", "invalid token", "IDENTIFIER", "STRING_LITERAL",
  "ERROR_TOKEN", "NUMBER_LITERAL", "TRUE", "FALSE", "PRINT", "SQRT", "SIN",
  "COS", "EXP", "LOG", "RAND", "PI_CONST", "E_CONST", "LET", "IN", "IF",
  "ELIF", "ELSE", "WHILE", "FOR", "FUNCTION", "TYPE", "PROTOCOL",
  "EXTENDS", "INHERITS", "NEW", "IS", "AS", "SELF", "BASE", "PLUS",
  "MINUS", "STAR", "SLASH", "PERCENT", "CARET", "ASSIGN",
  "DESTRUCTIVE_ASSIGN", "EQUAL_EQUAL", "NOT_EQUAL", "LESS", "LESS_EQUAL",
  "GREATER", "GREATER_EQUAL", "AND", "OR", "NOT", "CONCAT", "DOUBLECONCAT",
  "FATARROW", "LPAREN", "RPAREN", "LBRACE", "RBRACE", "COMMA", "SEMICOLON",
  "COLON", "DOT", "UNDERSCORE", "AUTO", "UMINUS", "$accept", "program",
  "top_level_items", "top_level_item", "opt_semi", "decl", "function_decl",
  "type_decl", "protocol_decl", "protocol_extends_opt",
  "protocol_member_list", "protocol_member", "ctor_params_opt",
  "inherits_opt", "parent_args_opt", "type_member_list", "type_member",
  "params_opt", "param_list", "param", "return_ann_opt", "type_ann_opt",
  "type_expr", "expr", "lambda_expr", "lambda_param_list", "lambda_param",
  "let_expr", "binding_list", "binding", "if_expr", "elif_clauses",
  "while_expr", "for_expr", "assign_expr", "lvalue", "logic_or",
  "logic_and", "equality", "relation", "type_test_expr", "concat",
  "additive", "multiplicative", "power", "unary", "postfix", "primary",
  "args_opt", "arg_list", "block", "block_body_opt", "expr_list", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  // Parser::context.
  Parser::context::context (const Parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  Parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
  Parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short Parser::yypact_ninf_ = -216;

  const signed char Parser::yytable_ninf_ = -69;

  const short
  Parser::yypact_[] =
  {
     256,   -23,  -216,  -216,  -216,  -216,   -24,    24,    33,    36,
      40,    41,    43,  -216,  -216,    42,    44,    45,    47,    82,
      98,   100,   101,     7,    50,   366,   366,   192,   311,   106,
     256,    48,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,    65,    59,    61,   -18,    -6,  -216,   -19,
      46,    37,  -216,    71,   -46,  -216,  -216,   311,   311,   311,
     311,   311,   311,    56,    53,   -13,  -216,   311,   311,   113,
      62,    64,    92,    66,   311,  -216,  -216,   311,  -216,   -40,
    -216,   -32,    75,   -39,  -216,  -216,    74,    81,  -216,    48,
    -216,  -216,   311,   366,   366,   366,   366,   366,   366,   366,
     366,     1,     1,   366,   366,   366,   366,   366,   366,   366,
     366,   311,   140,    88,    89,    90,    91,    93,    94,  -216,
       1,   107,   311,    42,    95,    96,   131,   151,   151,   126,
     153,   102,   311,  -216,   104,    99,   154,     1,  -216,   103,
     158,  -216,   311,  -216,  -216,  -216,    61,   -18,    -6,    -6,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,   125,   125,    46,
      46,    37,    37,  -216,  -216,  -216,  -216,   109,   121,  -216,
    -216,  -216,  -216,  -216,   311,   125,   311,  -216,  -216,   311,
     311,   311,    53,   110,   108,  -216,   112,   166,   114,  -216,
    -216,   116,  -216,   311,  -216,   125,     1,   119,   117,  -216,
    -216,  -216,  -216,   127,  -216,  -216,  -216,   128,  -216,   103,
     151,  -216,   115,  -216,     5,  -216,  -216,   125,   311,  -216,
      15,   311,   -22,  -216,   311,  -216,     8,   120,  -216,  -216,
    -216,   122,   311,  -216,   311,  -216,   129,   -37,  -216,  -216,
     151,   311,  -216,   130,  -216,   151,   141,   132,   133,  -216,
     135,   311,   103,   311,   103,   134,   137,  -216,   -10,  -216,
    -216,   311,  -216,   157,  -216
  };

  const unsigned char
  Parser::yydefact_[] =
  {
       0,   106,   103,   102,   104,   105,     0,     0,     0,     0,
       0,     0,     0,   119,   120,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,   126,     0,
       2,     8,     5,     9,    10,    11,     6,    45,    46,    47,
      48,    49,    50,     0,    65,    70,    72,    75,    80,    81,
      86,    89,    93,    95,    98,    99,   111,     0,     0,     0,
       0,     0,     0,     0,    40,     0,    56,     0,     0,     0,
       0,    22,    17,     0,   122,   106,   107,     0,    96,    98,
      97,   106,     0,     0,    52,   128,     0,     8,     1,     8,
       7,     3,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   122,     0,     0,     0,     0,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,    33,    33,    24,
       0,     0,   122,   123,     0,   121,     0,     0,   110,    38,
       0,   125,     7,   127,     4,    64,    69,    71,    73,    74,
      76,    77,    78,    79,    41,    43,    44,    82,    83,    84,
      85,    87,    88,    90,    91,    92,    94,     0,   101,   112,
     113,   114,   115,   117,     0,    39,     0,    55,    57,     0,
       0,     0,    40,     0,    32,    34,     0,     0,     0,    16,
      18,     0,   108,     0,   101,    54,     0,     0,     0,    53,
     129,    42,   100,     0,    58,    60,    62,     0,    36,    38,
       0,    21,    26,    27,     0,   109,   124,    37,     0,   118,
       0,     0,     0,    35,   122,    23,     0,     0,    15,    19,
      51,     0,     0,    63,     0,    13,     0,    40,    14,    28,
      33,     0,    59,     0,    25,    33,     0,     0,     0,    12,
       0,     0,    38,     0,    38,     0,     0,    61,     0,    29,
      20,     0,    31,     0,    30
  };

  const short
  Parser::yypgoto_[] =
  {
    -216,  -216,  -216,   156,    -9,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -127,  -216,     3,
    -204,  -168,   -99,     0,  -216,  -216,    52,  -216,  -216,    97,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,   118,   136,   -12,
     -27,  -216,   -17,   -16,   -56,    68,    30,  -216,  -109,  -216,
    -215,  -216,  -216
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,    29,    30,    31,    91,    32,    33,    34,    35,   131,
     214,   229,   129,   188,   225,   226,   239,   183,   184,   185,
     197,   121,   157,   133,    37,    83,    84,    38,    65,    66,
      39,   220,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   134,   135,
      56,    86,    87
  };

  const short
  Parser::yytable_[] =
  {
      36,   186,   167,   158,   154,   222,   122,   235,   227,   111,
     -66,   237,   101,   102,   208,   111,   112,   139,   245,   -66,
     140,   175,   136,   191,   120,    95,    96,    82,    85,   137,
      36,    57,   234,   103,   104,    28,   231,   232,   195,    97,
      98,    99,   100,   262,   261,    64,   123,    28,   256,   -68,
     258,   163,   164,   165,   166,    79,    79,   113,   114,   115,
     116,   117,   118,   228,   155,   156,   238,   124,   125,   246,
     150,   151,   152,   153,   107,   108,   109,    82,   143,    58,
     144,   105,   106,   148,   149,    70,   159,   160,    59,   161,
     162,    60,   145,    78,    80,    61,    62,   217,    63,    67,
      68,    71,    69,    72,    73,    74,    88,    92,    90,    93,
      94,   110,   119,   247,   120,   236,   126,   127,   250,   128,
     130,   132,   177,    79,    79,    79,    79,    79,    79,    79,
      79,   138,   141,    79,    79,    79,    79,    79,    79,    79,
      79,   142,   200,   168,   169,   170,   171,   172,   176,   173,
     181,   179,   180,   174,   182,   187,   189,   194,   193,   190,
     192,   198,   201,   -67,   196,   202,   209,   210,   211,   212,
     224,   213,   215,   218,   203,   240,   204,   241,   137,   205,
     206,   207,   251,   219,   221,   244,    89,     0,   252,   253,
     249,   254,   199,   216,   259,    81,     2,   260,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,   146,    16,   223,     0,    17,    18,   264,   230,     0,
     178,   233,    22,     0,     0,    23,    24,     0,    25,     0,
     147,     0,   242,     0,   243,     0,     0,     0,     0,     0,
       0,   248,     0,    26,     0,     0,     0,    27,     0,    28,
       0,   255,     0,   257,     0,     0,     0,     0,     0,     1,
       2,   263,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,     0,    16,     0,     0,    17,
      18,    19,    20,    21,     0,     0,    22,     0,     0,    23,
      24,     0,    25,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,    27,     0,    28,     1,     2,     0,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
       0,    16,     0,     0,    17,    18,     0,     0,     0,     0,
       0,    22,     0,     0,    23,    24,     0,    25,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    26,     0,     0,     0,    27,     0,    28,    75,
       2,     0,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     0,     0,    76,
      24,     0,    25,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,    77,     0,    28
  };

  const short
  Parser::yycheck_[] =
  {
       0,   128,   111,   102,     3,   209,    19,   222,     3,    55,
      42,     3,    31,    32,   182,    55,    62,    56,    55,    42,
      59,   120,    62,   132,    61,    43,    44,    27,    28,    61,
      30,    55,    54,    52,    53,    57,    21,    22,   137,    45,
      46,    47,    48,   258,    54,     3,    59,    57,   252,    42,
     254,   107,   108,   109,   110,    25,    26,    57,    58,    59,
      60,    61,    62,    58,    63,    64,    58,    67,    68,   237,
      97,    98,    99,   100,    37,    38,    39,    77,    87,    55,
      89,    35,    36,    95,    96,     3,   103,   104,    55,   105,
     106,    55,    92,    25,    26,    55,    55,   196,    55,    55,
      55,     3,    55,     3,     3,    55,     0,    42,    60,    50,
      49,    40,    56,   240,    61,   224,     3,    55,   245,    55,
      28,    55,   122,    93,    94,    95,    96,    97,    98,    99,
     100,    56,    58,   103,   104,   105,   106,   107,   108,   109,
     110,    60,   142,     3,    56,    56,    56,    56,    41,    56,
      19,    56,    56,    59,     3,    29,     3,     3,    59,    57,
      56,     3,    37,    42,    61,    56,    56,    59,    56,     3,
      55,    57,    56,    54,   174,    55,   176,    55,    61,   179,
     180,   181,    41,    56,    56,    56,    30,    -1,    56,    56,
      60,    56,   140,   193,    60,     3,     4,    60,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    93,    20,   210,    -1,    23,    24,    60,   218,    -1,
     123,   221,    30,    -1,    -1,    33,    34,    -1,    36,    -1,
      94,    -1,   232,    -1,   234,    -1,    -1,    -1,    -1,    -1,
      -1,   241,    -1,    51,    -1,    -1,    -1,    55,    -1,    57,
      -1,   251,    -1,   253,    -1,    -1,    -1,    -1,    -1,     3,
       4,   261,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    -1,    20,    -1,    -1,    23,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    55,    -1,    57,     3,     4,    -1,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      -1,    20,    -1,    -1,    23,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    55,    -1,    57,     3,
       4,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    55,    -1,    57
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     3,     4,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    20,    23,    24,    25,
      26,    27,    30,    33,    34,    36,    51,    55,    57,    67,
      68,    69,    71,    72,    73,    74,    89,    90,    93,    96,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   116,    55,    55,    55,
      55,    55,    55,    55,     3,    94,    95,    55,    55,    55,
       3,     3,     3,     3,    55,     3,    33,    55,   111,   112,
     111,     3,    89,    91,    92,    89,   117,   118,     0,    69,
      60,    70,    42,    50,    49,    43,    44,    45,    46,    47,
      48,    31,    32,    52,    53,    35,    36,    37,    38,    39,
      40,    55,    62,    89,    89,    89,    89,    89,    89,    56,
      61,    87,    19,    59,    89,    89,     3,    55,    55,    78,
      28,    75,    55,    89,   114,   115,    62,    61,    56,    56,
      59,    58,    60,    70,    70,    89,   103,   104,   105,   105,
     106,   106,   106,   106,     3,    63,    64,    88,    88,   108,
     108,   109,   109,   110,   110,   110,   110,   114,     3,    56,
      56,    56,    56,    56,    59,    88,    41,    89,    95,    56,
      56,    19,     3,    83,    84,    85,    83,    29,    79,     3,
      57,   114,    56,    59,     3,    88,    61,    86,     3,    92,
      89,    37,    56,    89,    89,    89,    89,    89,    87,    56,
      59,    56,     3,    57,    76,    56,    89,    88,    54,    56,
      97,    56,    86,    85,    55,    80,    81,     3,    58,    77,
      89,    21,    22,    89,    54,   116,   114,     3,    58,    82,
      55,    55,    89,    89,    56,    55,    87,    83,    89,    60,
      83,    41,    56,    56,    56,    89,    86,    89,    86,    60,
      60,    54,   116,    89,    60
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    66,    67,    68,    68,    69,    69,    70,    70,    71,
      71,    71,    72,    72,    73,    74,    75,    75,    76,    76,
      77,    78,    78,    79,    79,    80,    80,    81,    81,    82,
      82,    82,    83,    83,    84,    84,    85,    86,    86,    87,
      87,    88,    88,    88,    88,    89,    89,    89,    89,    89,
      89,    90,    91,    91,    92,    93,    94,    94,    95,    96,
      97,    97,    98,    99,   100,   100,   101,   101,   101,   102,
     102,   103,   103,   104,   104,   104,   105,   105,   105,   105,
     105,   106,   106,   106,   107,   107,   107,   108,   108,   108,
     109,   109,   109,   109,   110,   110,   111,   111,   111,   112,
     112,   112,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   114,   114,   115,   115,   116,   117,   117,   118,   118
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     1,     2,     3,     1,     1,     1,     0,     1,
       1,     1,     9,     7,     7,     6,     2,     0,     0,     2,
       6,     3,     0,     3,     0,     3,     0,     0,     2,     5,
       8,     6,     1,     0,     1,     3,     2,     2,     0,     2,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     6,     1,     3,     3,     4,     1,     3,     4,     8,
       0,     6,     5,     7,     3,     1,     1,     3,     1,     3,
       1,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     1,     3,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     1,     3,     1,     2,     2,     1,     1,
       4,     3,     1,     1,     1,     1,     1,     1,     4,     5,
       3,     1,     4,     4,     4,     4,     3,     4,     6,     1,
       1,     1,     0,     1,     3,     3,     0,     2,     1,     3
  };




#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   169,   169,   188,   192,   209,   214,   223,   224,   228,
     232,   236,   243,   252,   264,   286,   298,   303,   310,   313,
     321,   328,   333,   339,   344,   350,   355,   362,   365,   373,
     388,   403,   421,   426,   432,   438,   446,   457,   462,   468,
     473,   479,   483,   487,   491,   498,   502,   506,   510,   514,
     518,   525,   537,   543,   551,   558,   566,   572,   580,   592,
     601,   604,   612,   620,   628,   637,   644,   648,   652,   659,
     666,   673,   680,   687,   694,   701,   708,   715,   722,   729,
     736,   743,   747,   752,   760,   767,   774,   781,   788,   795,
     802,   809,   816,   823,   830,   837,   844,   851,   856,   863,
     867,   881,   889,   894,   899,   904,   909,   914,   919,   924,
     929,   933,   937,   942,   949,   956,   963,   969,   976,   984,
     989,   997,  1002,  1008,  1014,  1022,  1031,  1034,  1041,  1047
  };

  void
  Parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  Parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 4 "src/parser/grammar.y"
} } // hulk::parser
#line 2966 "src/parser/parser.cpp"

#line 1054 "src/parser/grammar.y"


void hulk::parser::Parser::error(const location_type& loc,
                                 const std::string& msg) {
    hulk::common::Span span {
        .start = {
            .index = 0,
            .line = static_cast<std::size_t>(loc.begin.line),
            .column = static_cast<std::size_t>(loc.begin.column),
        },
        .end = {
            .index = 0,
            .line = static_cast<std::size_t>(loc.end.line),
            .column = static_cast<std::size_t>(loc.end.column),
        },
    };

    driver.report_syntax_error(msg, span);
}
