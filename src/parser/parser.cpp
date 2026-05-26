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
#line 96 "src/parser/grammar.y"

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
#line 166 "src/parser/grammar.y"
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
#line 185 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = std::move(yystack_[1].value.as < hulk::parser::TopLevelItems > ());
      }
#line 1116 "src/parser/parser.cpp"
    break;

  case 4: // top_level_items: top_level_items top_level_item opt_semi
#line 189 "src/parser/grammar.y"
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
#line 206 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = hulk::parser::TopLevelItems {};
          yylhs.value.as < hulk::parser::TopLevelItems > ().decls.push_back(std::move(yystack_[0].value.as < DeclPtr > ()));
      }
#line 1143 "src/parser/parser.cpp"
    break;

  case 6: // top_level_item: expr
#line 211 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = hulk::parser::TopLevelItems {};
          yylhs.value.as < hulk::parser::TopLevelItems > ().globalExpr = std::move(yystack_[0].value.as < ExprPtr > ());
          yylhs.value.as < hulk::parser::TopLevelItems > ().hasGlobalExpr = true;
      }
#line 1153 "src/parser/parser.cpp"
    break;

  case 9: // decl: function_decl
#line 225 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1161 "src/parser/parser.cpp"
    break;

  case 10: // decl: type_decl
#line 229 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1169 "src/parser/parser.cpp"
    break;

  case 11: // decl: protocol_decl
#line 233 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1177 "src/parser/parser.cpp"
    break;

  case 12: // function_decl: FUNCTION IDENTIFIER LPAREN params_opt RPAREN return_ann_opt FATARROW expr SEMICOLON
#line 240 "src/parser/grammar.y"
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
#line 249 "src/parser/grammar.y"
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
#line 261 "src/parser/grammar.y"
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
#line 283 "src/parser/grammar.y"
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
#line 295 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo { yystack_[0].value.as < std::string > (), ExprList {}, true };
      }
#line 1247 "src/parser/parser.cpp"
    break;

  case 17: // protocol_extends_opt: %empty
#line 299 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo {};
      }
#line 1255 "src/parser/parser.cpp"
    break;

  case 18: // protocol_member_list: %empty
#line 306 "src/parser/grammar.y"
      {
          yylhs.value.as < ProtocolMethodList > () = hulk::parser::ProtocolMethodList {};
      }
#line 1263 "src/parser/parser.cpp"
    break;

  case 19: // protocol_member_list: protocol_member_list protocol_member
#line 310 "src/parser/grammar.y"
      {
          yystack_[1].value.as < ProtocolMethodList > ().push_back(std::move(yystack_[0].value.as < Hulk::ProtocolMethodSig > ()));
          yylhs.value.as < ProtocolMethodList > () = std::move(yystack_[1].value.as < ProtocolMethodList > ());
      }
#line 1272 "src/parser/parser.cpp"
    break;

  case 20: // protocol_member: IDENTIFIER LPAREN params_opt RPAREN return_ann_opt SEMICOLON
#line 318 "src/parser/grammar.y"
      {
          yylhs.value.as < Hulk::ProtocolMethodSig > () = Hulk::ProtocolMethodSig(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), yystack_[1].value.as < std::string > ());
      }
#line 1280 "src/parser/parser.cpp"
    break;

  case 21: // ctor_params_opt: LPAREN params_opt RPAREN
#line 325 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = std::move(yystack_[1].value.as < ParamList > ());
      }
#line 1288 "src/parser/parser.cpp"
    break;

  case 22: // ctor_params_opt: %empty
#line 329 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = ParamList {};
      }
#line 1296 "src/parser/parser.cpp"
    break;

  case 23: // inherits_opt: INHERITS IDENTIFIER parent_args_opt
#line 336 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo { yystack_[1].value.as < std::string > (), std::move(yystack_[0].value.as < ExprList > ()), true };
      }
#line 1304 "src/parser/parser.cpp"
    break;

  case 24: // inherits_opt: %empty
#line 340 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo {};
      }
#line 1312 "src/parser/parser.cpp"
    break;

  case 25: // parent_args_opt: LPAREN args_opt RPAREN
#line 347 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[1].value.as < ExprList > ());
      }
#line 1320 "src/parser/parser.cpp"
    break;

  case 26: // parent_args_opt: %empty
#line 351 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 1328 "src/parser/parser.cpp"
    break;

  case 27: // type_member_list: %empty
#line 358 "src/parser/grammar.y"
      {
          yylhs.value.as < TypeMemberList > () = TypeMemberList {};
      }
#line 1336 "src/parser/parser.cpp"
    break;

  case 28: // type_member_list: type_member_list type_member
#line 362 "src/parser/grammar.y"
      {
          yystack_[1].value.as < TypeMemberList > ().push_back(std::move(yystack_[0].value.as < Hulk::TypeMember > ()));
          yylhs.value.as < TypeMemberList > () = std::move(yystack_[1].value.as < TypeMemberList > ());
      }
#line 1345 "src/parser/parser.cpp"
    break;

  case 29: // type_member: IDENTIFIER type_ann_opt ASSIGN expr SEMICOLON
#line 370 "src/parser/grammar.y"
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
#line 385 "src/parser/grammar.y"
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
#line 400 "src/parser/grammar.y"
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
#line 418 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = std::move(yystack_[0].value.as < ParamList > ());
      }
#line 1410 "src/parser/parser.cpp"
    break;

  case 33: // params_opt: %empty
#line 422 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = ParamList {};
      }
#line 1418 "src/parser/parser.cpp"
    break;

  case 34: // param_list: param
#line 429 "src/parser/grammar.y"
      {
          ParamList params;
          params.push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(params);
      }
#line 1428 "src/parser/parser.cpp"
    break;

  case 35: // param_list: param_list COMMA param
#line 435 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ParamList > ().push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(yystack_[2].value.as < ParamList > ());
      }
#line 1437 "src/parser/parser.cpp"
    break;

  case 36: // param: IDENTIFIER type_ann_opt
#line 443 "src/parser/grammar.y"
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
#line 454 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1457 "src/parser/parser.cpp"
    break;

  case 38: // return_ann_opt: %empty
#line 458 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "";
      }
#line 1465 "src/parser/parser.cpp"
    break;

  case 39: // type_ann_opt: COLON type_expr
#line 465 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1473 "src/parser/parser.cpp"
    break;

  case 40: // type_ann_opt: %empty
#line 469 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "";
      }
#line 1481 "src/parser/parser.cpp"
    break;

  case 41: // type_expr: IDENTIFIER
#line 476 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1489 "src/parser/parser.cpp"
    break;

  case 42: // type_expr: type_expr STAR
#line 480 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[1].value.as < std::string > ()) + "*";
      }
#line 1497 "src/parser/parser.cpp"
    break;

  case 43: // expr: lambda_expr
#line 487 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1505 "src/parser/parser.cpp"
    break;

  case 44: // expr: let_expr
#line 491 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1513 "src/parser/parser.cpp"
    break;

  case 45: // expr: if_expr
#line 495 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1521 "src/parser/parser.cpp"
    break;

  case 46: // expr: while_expr
#line 499 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1529 "src/parser/parser.cpp"
    break;

  case 47: // expr: for_expr
#line 503 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1537 "src/parser/parser.cpp"
    break;

  case 48: // expr: assign_expr
#line 507 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1545 "src/parser/parser.cpp"
    break;

  case 49: // lambda_expr: LPAREN lambda_param_list RPAREN return_ann_opt FATARROW expr
#line 514 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < std::string > ().empty()) {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Lambda>(std::move(yystack_[4].value.as < ParamList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Lambda>(std::move(yystack_[4].value.as < ParamList > ()), yystack_[2].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1558 "src/parser/parser.cpp"
    break;

  case 50: // lambda_param_list: lambda_param
#line 526 "src/parser/grammar.y"
      {
          ParamList params;
          params.push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(params);
      }
#line 1568 "src/parser/parser.cpp"
    break;

  case 51: // lambda_param_list: lambda_param_list COMMA lambda_param
#line 532 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ParamList > ().push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(yystack_[2].value.as < ParamList > ());
      }
#line 1577 "src/parser/parser.cpp"
    break;

  case 52: // lambda_param: IDENTIFIER COLON type_expr
#line 540 "src/parser/grammar.y"
      {
          yylhs.value.as < Hulk::Param > () = Hulk::Param(yystack_[2].value.as < std::string > (), yystack_[0].value.as < std::string > ());
      }
#line 1585 "src/parser/parser.cpp"
    break;

  case 53: // let_expr: LET binding_list IN expr
#line 547 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LetIn>(std::move(yystack_[2].value.as < BindingList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1594 "src/parser/parser.cpp"
    break;

  case 54: // binding_list: binding
#line 555 "src/parser/grammar.y"
      {
          BindingList bindings;
          bindings.push_back(std::move(yystack_[0].value.as < BindingPtr > ()));
          yylhs.value.as < BindingList > () = std::move(bindings);
      }
#line 1604 "src/parser/parser.cpp"
    break;

  case 55: // binding_list: binding_list COMMA binding
#line 561 "src/parser/grammar.y"
      {
          yystack_[2].value.as < BindingList > ().push_back(std::move(yystack_[0].value.as < BindingPtr > ()));
          yylhs.value.as < BindingList > () = std::move(yystack_[2].value.as < BindingList > ());
      }
#line 1613 "src/parser/parser.cpp"
    break;

  case 56: // binding: IDENTIFIER type_ann_opt ASSIGN expr
#line 569 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < std::string > ().empty()) {
              yylhs.value.as < BindingPtr > () = std::make_unique<Hulk::VariableBinding>(yystack_[3].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < BindingPtr > () = std::make_unique<Hulk::VariableBinding>(yystack_[3].value.as < std::string > (), yystack_[2].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < BindingPtr > ()->span = to_span(yylhs.location);
      }
#line 1626 "src/parser/parser.cpp"
    break;

  case 57: // if_expr: IF LPAREN expr RPAREN expr elif_clauses ELSE expr
#line 581 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::IfStmt>(std::move(yystack_[5].value.as < ExprPtr > ()), std::move(yystack_[3].value.as < ExprPtr > ()), std::move(yystack_[2].value.as < ElifList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1635 "src/parser/parser.cpp"
    break;

  case 58: // elif_clauses: %empty
#line 589 "src/parser/grammar.y"
      {
          yylhs.value.as < ElifList > () = ElifList {};
      }
#line 1643 "src/parser/parser.cpp"
    break;

  case 59: // elif_clauses: elif_clauses ELIF LPAREN expr RPAREN expr
#line 593 "src/parser/grammar.y"
      {
          yystack_[5].value.as < ElifList > ().emplace_back(std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ElifList > () = std::move(yystack_[5].value.as < ElifList > ());
      }
#line 1652 "src/parser/parser.cpp"
    break;

  case 60: // while_expr: WHILE LPAREN expr RPAREN expr
#line 601 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::WhileStmt>(std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1661 "src/parser/parser.cpp"
    break;

  case 61: // for_expr: FOR LPAREN IDENTIFIER IN expr RPAREN expr
#line 609 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::For>(yystack_[4].value.as < std::string > (), std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1670 "src/parser/parser.cpp"
    break;

  case 62: // assign_expr: lvalue DESTRUCTIVE_ASSIGN expr
#line 617 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < hulk::parser::LValueTarget > ().isMember) {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::DestructiveAssignMember>(std::move(yystack_[2].value.as < hulk::parser::LValueTarget > ().object), yystack_[2].value.as < hulk::parser::LValueTarget > ().name, std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::DestructiveAssign>(yystack_[2].value.as < hulk::parser::LValueTarget > ().name, std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1683 "src/parser/parser.cpp"
    break;

  case 63: // assign_expr: logic_or
#line 626 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1691 "src/parser/parser.cpp"
    break;

  case 64: // lvalue: IDENTIFIER
#line 633 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { nullptr, yystack_[0].value.as < std::string > (), false };
      }
#line 1699 "src/parser/parser.cpp"
    break;

  case 65: // lvalue: postfix DOT IDENTIFIER
#line 637 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > (), true };
      }
#line 1707 "src/parser/parser.cpp"
    break;

  case 66: // logic_or: logic_or OR logic_and
#line 644 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Or, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1718 "src/parser/parser.cpp"
    break;

  case 67: // logic_or: logic_and
#line 651 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1726 "src/parser/parser.cpp"
    break;

  case 68: // logic_and: logic_and AND equality
#line 658 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::And, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1737 "src/parser/parser.cpp"
    break;

  case 69: // logic_and: equality
#line 665 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1745 "src/parser/parser.cpp"
    break;

  case 70: // equality: equality EQUAL_EQUAL relation
#line 672 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Equal, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1756 "src/parser/parser.cpp"
    break;

  case 71: // equality: equality NOT_EQUAL relation
#line 679 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::NotEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1767 "src/parser/parser.cpp"
    break;

  case 72: // equality: relation
#line 686 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1775 "src/parser/parser.cpp"
    break;

  case 73: // relation: relation LESS type_test_expr
#line 693 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Less, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1786 "src/parser/parser.cpp"
    break;

  case 74: // relation: relation LESS_EQUAL type_test_expr
#line 700 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::LessEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1797 "src/parser/parser.cpp"
    break;

  case 75: // relation: relation GREATER type_test_expr
#line 707 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Greater, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1808 "src/parser/parser.cpp"
    break;

  case 76: // relation: relation GREATER_EQUAL type_test_expr
#line 714 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::GreaterEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1819 "src/parser/parser.cpp"
    break;

  case 77: // relation: type_test_expr
#line 721 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1827 "src/parser/parser.cpp"
    break;

  case 78: // type_test_expr: concat
#line 728 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1835 "src/parser/parser.cpp"
    break;

  case 79: // type_test_expr: concat IS type_expr
#line 732 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::IsExpr>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1844 "src/parser/parser.cpp"
    break;

  case 80: // type_test_expr: concat AS type_expr
#line 737 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::AsExpr>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1853 "src/parser/parser.cpp"
    break;

  case 81: // concat: concat CONCAT additive
#line 745 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::StringBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::StringOp::Concat, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1864 "src/parser/parser.cpp"
    break;

  case 82: // concat: concat DOUBLECONCAT additive
#line 752 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::StringBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::StringOp::SpaceConcat, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1875 "src/parser/parser.cpp"
    break;

  case 83: // concat: additive
#line 759 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1883 "src/parser/parser.cpp"
    break;

  case 84: // additive: additive PLUS multiplicative
#line 766 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Plus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1894 "src/parser/parser.cpp"
    break;

  case 85: // additive: additive MINUS multiplicative
#line 773 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Minus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1905 "src/parser/parser.cpp"
    break;

  case 86: // additive: multiplicative
#line 780 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1913 "src/parser/parser.cpp"
    break;

  case 87: // multiplicative: multiplicative STAR power
#line 787 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Mult, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1924 "src/parser/parser.cpp"
    break;

  case 88: // multiplicative: multiplicative SLASH power
#line 794 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Div, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1935 "src/parser/parser.cpp"
    break;

  case 89: // multiplicative: multiplicative PERCENT power
#line 801 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Mod, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1946 "src/parser/parser.cpp"
    break;

  case 90: // multiplicative: power
#line 808 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1954 "src/parser/parser.cpp"
    break;

  case 91: // power: unary CARET power
#line 815 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Pow, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1965 "src/parser/parser.cpp"
    break;

  case 92: // power: unary
#line 822 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1973 "src/parser/parser.cpp"
    break;

  case 93: // unary: MINUS unary
#line 829 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticUnaryOp>(
              Hulk::ArithUnaryType::Minus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1984 "src/parser/parser.cpp"
    break;

  case 94: // unary: NOT unary
#line 836 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicUnaryOp>(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1993 "src/parser/parser.cpp"
    break;

  case 95: // unary: postfix
#line 841 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 2001 "src/parser/parser.cpp"
    break;

  case 96: // postfix: primary
#line 848 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 2009 "src/parser/parser.cpp"
    break;

  case 97: // postfix: postfix LPAREN args_opt RPAREN
#line 852 "src/parser/grammar.y"
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
#line 2027 "src/parser/parser.cpp"
    break;

  case 98: // postfix: postfix DOT IDENTIFIER
#line 866 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::MemberAccess>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2036 "src/parser/parser.cpp"
    break;

  case 99: // primary: NUMBER_LITERAL
#line 874 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(yystack_[0].value.as < double > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2045 "src/parser/parser.cpp"
    break;

  case 100: // primary: STRING_LITERAL
#line 879 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::String>(yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2054 "src/parser/parser.cpp"
    break;

  case 101: // primary: TRUE
#line 884 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Boolean>(true);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2063 "src/parser/parser.cpp"
    break;

  case 102: // primary: FALSE
#line 889 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Boolean>(false);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2072 "src/parser/parser.cpp"
    break;

  case 103: // primary: IDENTIFIER
#line 894 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::VariableReference>(yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2081 "src/parser/parser.cpp"
    break;

  case 104: // primary: NEW IDENTIFIER LPAREN args_opt RPAREN
#line 899 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::NewExpr>(yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2090 "src/parser/parser.cpp"
    break;

  case 105: // primary: LPAREN expr RPAREN
#line 904 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[1].value.as < ExprPtr > ());
      }
#line 2098 "src/parser/parser.cpp"
    break;

  case 106: // primary: block
#line 908 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 2106 "src/parser/parser.cpp"
    break;

  case 107: // primary: PRINT LPAREN expr RPAREN
#line 912 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Print>(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2115 "src/parser/parser.cpp"
    break;

  case 108: // primary: SQRT LPAREN expr RPAREN
#line 917 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Sqrt, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2126 "src/parser/parser.cpp"
    break;

  case 109: // primary: SIN LPAREN expr RPAREN
#line 924 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Sin, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2137 "src/parser/parser.cpp"
    break;

  case 110: // primary: COS LPAREN expr RPAREN
#line 931 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Cos, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2148 "src/parser/parser.cpp"
    break;

  case 111: // primary: RAND LPAREN RPAREN
#line 938 "src/parser/grammar.y"
      {
          ExprList args;
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Rand, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2158 "src/parser/parser.cpp"
    break;

  case 112: // primary: EXP LPAREN expr RPAREN
#line 944 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Exp, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2169 "src/parser/parser.cpp"
    break;

  case 113: // primary: LOG LPAREN expr COMMA expr RPAREN
#line 951 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[3].value.as < ExprPtr > ()));
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Log, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2181 "src/parser/parser.cpp"
    break;

  case 114: // primary: PI_CONST
#line 959 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(3.14159265358979323846);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2190 "src/parser/parser.cpp"
    break;

  case 115: // primary: E_CONST
#line 964 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(2.71828182845904523536);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2199 "src/parser/parser.cpp"
    break;

  case 116: // args_opt: arg_list
#line 972 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[0].value.as < ExprList > ());
      }
#line 2207 "src/parser/parser.cpp"
    break;

  case 117: // args_opt: %empty
#line 976 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 2215 "src/parser/parser.cpp"
    break;

  case 118: // arg_list: expr
#line 983 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(args);
      }
#line 2225 "src/parser/parser.cpp"
    break;

  case 119: // arg_list: arg_list COMMA expr
#line 989 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ExprList > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(yystack_[2].value.as < ExprList > ());
      }
#line 2234 "src/parser/parser.cpp"
    break;

  case 120: // block: LBRACE block_body_opt RBRACE
#line 997 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ExprBlock>(std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2243 "src/parser/parser.cpp"
    break;

  case 121: // block_body_opt: %empty
#line 1005 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 2251 "src/parser/parser.cpp"
    break;

  case 122: // block_body_opt: expr_list opt_semi
#line 1009 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[1].value.as < ExprList > ());
      }
#line 2259 "src/parser/parser.cpp"
    break;

  case 123: // expr_list: expr
#line 1016 "src/parser/grammar.y"
      {
          ExprList nodes;
          nodes.push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(nodes);
      }
#line 2269 "src/parser/parser.cpp"
    break;

  case 124: // expr_list: expr_list SEMICOLON expr
#line 1022 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ExprList > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(yystack_[2].value.as < ExprList > ());
      }
#line 2278 "src/parser/parser.cpp"
    break;


#line 2282 "src/parser/parser.cpp"

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
  "EXTENDS", "INHERITS", "NEW", "IS", "AS", "PLUS", "MINUS", "STAR",
  "SLASH", "PERCENT", "CARET", "ASSIGN", "DESTRUCTIVE_ASSIGN",
  "EQUAL_EQUAL", "NOT_EQUAL", "LESS", "LESS_EQUAL", "GREATER",
  "GREATER_EQUAL", "AND", "OR", "NOT", "CONCAT", "DOUBLECONCAT",
  "FATARROW", "LPAREN", "RPAREN", "LBRACE", "RBRACE", "COMMA", "SEMICOLON",
  "COLON", "DOT", "UMINUS", "$accept", "program", "top_level_items",
  "top_level_item", "opt_semi", "decl", "function_decl", "type_decl",
  "protocol_decl", "protocol_extends_opt", "protocol_member_list",
  "protocol_member", "ctor_params_opt", "inherits_opt", "parent_args_opt",
  "type_member_list", "type_member", "params_opt", "param_list", "param",
  "return_ann_opt", "type_ann_opt", "type_expr", "expr", "lambda_expr",
  "lambda_param_list", "lambda_param", "let_expr", "binding_list",
  "binding", "if_expr", "elif_clauses", "while_expr", "for_expr",
  "assign_expr", "lvalue", "logic_or", "logic_and", "equality", "relation",
  "type_test_expr", "concat", "additive", "multiplicative", "power",
  "unary", "postfix", "primary", "args_opt", "arg_list", "block",
  "block_body_opt", "expr_list", YY_NULLPTR
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


  const short Parser::yypact_ninf_ = -209;

  const signed char Parser::yytable_ninf_ = -66;

  const short
  Parser::yypact_[] =
  {
     172,   -33,  -209,  -209,  -209,  -209,    -6,    44,    46,    54,
      55,    57,    58,  -209,  -209,    26,    59,    60,    62,    42,
     113,   116,   117,    68,    68,   248,   301,   122,   172,    66,
    -209,  -209,  -209,  -209,  -209,  -209,  -209,  -209,  -209,  -209,
    -209,    85,    78,    89,    -2,    -8,  -209,   -19,    20,    33,
    -209,   107,   -43,  -209,  -209,   301,   301,   301,   301,   301,
     301,    92,    88,   -14,  -209,   301,   301,   145,    96,    97,
     123,    99,  -209,   301,  -209,   -38,  -209,   -32,   100,   -24,
    -209,  -209,   101,    95,  -209,    66,  -209,  -209,   301,    68,
      68,    68,    68,    68,    68,    68,    68,   152,   152,    68,
      68,    68,    68,    68,    68,    68,    68,   301,   153,   104,
     105,   106,   108,   109,   111,  -209,   152,   125,   301,    26,
     112,   137,   142,   162,   162,   141,   168,   138,   301,   174,
     152,  -209,   144,   197,  -209,   301,  -209,  -209,  -209,    89,
      -2,    -8,    -8,  -209,  -209,  -209,  -209,  -209,   166,   166,
      20,    20,    33,    33,  -209,  -209,  -209,  -209,  -209,   150,
     148,   167,  -209,  -209,  -209,  -209,  -209,   301,   166,   301,
    -209,  -209,   301,   301,   301,    88,   154,   155,  -209,   156,
     208,   159,  -209,  -209,   161,  -209,   166,   152,   164,   158,
    -209,  -209,  -209,  -209,   301,   165,  -209,  -209,  -209,   169,
    -209,   144,   162,  -209,   175,  -209,     8,  -209,   166,   301,
    -209,  -209,    65,   301,   -36,  -209,   301,  -209,    11,   176,
    -209,  -209,  -209,   177,   301,  -209,   301,  -209,   178,   -35,
    -209,  -209,   162,   301,  -209,   160,  -209,   162,   181,   180,
     182,  -209,   183,   301,   144,   301,   144,   173,   184,  -209,
     -11,  -209,  -209,   301,  -209,   186,  -209
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,   103,   100,    99,   101,   102,     0,     0,     0,     0,
       0,     0,     0,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   121,     0,     2,     8,
       5,     9,    10,    11,     6,    43,    44,    45,    46,    47,
      48,     0,    63,    67,    69,    72,    77,    78,    83,    86,
      90,    92,    95,    96,   106,     0,     0,     0,     0,     0,
       0,     0,    40,     0,    54,     0,     0,     0,     0,    22,
      17,     0,   103,     0,    93,    95,    94,   103,     0,     0,
      50,   123,     0,     8,     1,     8,     7,     3,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   117,     0,     0,
       0,     0,     0,     0,     0,   111,     0,     0,     0,     0,
       0,     0,     0,    33,    33,    24,     0,     0,   117,     0,
       0,   105,    38,     0,   120,     7,   122,     4,    62,    66,
      68,    70,    71,    73,    74,    75,    76,    41,    79,    80,
      81,    82,    84,    85,    87,    88,    89,    91,   118,     0,
     116,    98,   107,   108,   109,   110,   112,     0,    39,     0,
      53,    55,     0,     0,     0,    40,     0,    32,    34,     0,
       0,     0,    16,    18,     0,    98,    52,     0,     0,     0,
      51,   124,    42,    97,     0,     0,    56,    58,    60,     0,
      36,    38,     0,    21,    26,    27,     0,   104,    37,     0,
     119,   113,     0,     0,     0,    35,   117,    23,     0,     0,
      15,    19,    49,     0,     0,    61,     0,    13,     0,    40,
      14,    28,    33,     0,    57,     0,    25,    33,     0,     0,
       0,    12,     0,     0,    38,     0,    38,     0,     0,    59,
       0,    29,    20,     0,    31,     0,    30
  };

  const short
  Parser::yypgoto_[] =
  {
    -209,  -209,  -209,   194,   -62,  -209,  -209,  -209,  -209,  -209,
    -209,  -209,  -209,  -209,  -209,  -209,  -209,  -123,  -209,    36,
    -198,  -166,   -96,     0,  -209,  -209,   102,  -209,  -209,   120,
    -209,  -209,  -209,  -209,  -209,  -209,  -209,   151,   157,     9,
     -44,  -209,     4,   -12,   -10,    82,    38,  -209,  -124,  -209,
    -208,  -209,  -209
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,    27,    28,    29,    87,    30,    31,    32,    33,   127,
     206,   221,   125,   181,   217,   218,   231,   176,   177,   178,
     188,   117,   148,   158,    35,    79,    80,    36,    63,    64,
      37,   212,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,   159,   160,
      54,    82,    83
  };

  const short
  Parser::yytable_[] =
  {
      34,   179,   149,   214,   184,   118,   227,   -64,   -64,   200,
     107,   219,    97,    98,   229,   107,   226,   108,   237,    26,
     168,   136,   129,   137,   116,    78,    81,   130,    34,    62,
     132,    99,   100,   133,   186,    93,    94,    95,    96,    91,
      92,   253,   254,   119,    26,    68,   248,    55,   250,   143,
     144,   145,   146,   101,   102,   109,   110,   111,   112,   113,
     114,    75,    75,   238,   220,   120,   121,   230,   103,   104,
     105,    72,     2,    78,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,   223,   224,   138,   152,
     153,   208,   228,   154,   155,   156,   157,    56,    22,    57,
     141,   142,    23,   150,   151,    74,    76,    58,    59,   239,
      60,    61,    65,    66,   242,    67,    69,    24,   170,    70,
      71,    73,    84,    26,    86,    88,    89,    75,    75,    75,
      75,    75,    75,    75,    75,   191,    90,    75,    75,    75,
      75,    75,    75,    75,    75,   106,   115,   116,   122,   123,
     124,   126,   128,   135,   131,   147,   161,   134,   162,   163,
     164,   174,   165,   166,   169,   175,   172,   195,   167,   196,
     180,   182,   197,   198,   199,     1,     2,   185,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,   173,    16,   183,   210,    17,    18,    19,    20,    21,
     189,   192,    22,   187,   193,   194,    23,   -65,   201,   222,
     203,   204,   202,   225,   205,   207,   209,   130,   241,   211,
     243,    24,    85,   213,   234,    25,   235,    26,   216,   232,
     233,   251,   236,   240,   244,   190,   245,   246,   215,   171,
     139,     0,   252,   247,   256,   249,     0,   140,     0,     0,
       0,    77,     2,   255,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    16,     0,
       0,    17,    18,     0,     0,     0,     0,     0,    22,     0,
       0,     0,    23,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,     0,     0,
       0,    25,     0,    26,     1,     2,     0,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
       0,    16,     0,     0,    17,    18,     0,     0,     0,     0,
       0,    22,     0,     0,     0,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,     0,     0,     0,    25,     0,    26
  };

  const short
  Parser::yycheck_[] =
  {
       0,   124,    98,   201,   128,    19,   214,    40,    40,   175,
      53,     3,    31,    32,     3,    53,    52,    60,    53,    55,
     116,    83,    60,    85,    59,    25,    26,    59,    28,     3,
      54,    50,    51,    57,   130,    43,    44,    45,    46,    41,
      42,    52,   250,    57,    55,     3,   244,    53,   246,    93,
      94,    95,    96,    33,    34,    55,    56,    57,    58,    59,
      60,    23,    24,   229,    56,    65,    66,    56,    35,    36,
      37,     3,     4,    73,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    21,    22,    88,   101,
     102,   187,   216,   103,   104,   105,   106,    53,    30,    53,
      91,    92,    34,    99,   100,    23,    24,    53,    53,   232,
      53,    53,    53,    53,   237,    53,     3,    49,   118,     3,
       3,    53,     0,    55,    58,    40,    48,    89,    90,    91,
      92,    93,    94,    95,    96,   135,    47,    99,   100,   101,
     102,   103,   104,   105,   106,    38,    54,    59,     3,    53,
      53,    28,    53,    58,    54,     3,     3,    56,    54,    54,
      54,    19,    54,    54,    39,     3,    54,   167,    57,   169,
      29,     3,   172,   173,   174,     3,     4,     3,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    54,    20,    55,   194,    23,    24,    25,    26,    27,
       3,    35,    30,    59,    54,    57,    34,    40,    54,   209,
      54,     3,    57,   213,    55,    54,    52,    59,    58,    54,
      39,    49,    28,    54,   224,    53,   226,    55,    53,    53,
      53,    58,    54,   233,    54,   133,    54,    54,   202,   119,
      89,    -1,    58,   243,    58,   245,    -1,    90,    -1,    -1,
      -1,     3,     4,   253,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    -1,    20,    -1,
      -1,    23,    24,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    -1,
      -1,    53,    -1,    55,     3,     4,    -1,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      -1,    20,    -1,    -1,    23,    24,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    53,    -1,    55
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     3,     4,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    20,    23,    24,    25,
      26,    27,    30,    34,    49,    53,    55,    63,    64,    65,
      67,    68,    69,    70,    85,    86,    89,    92,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   112,    53,    53,    53,    53,    53,
      53,    53,     3,    90,    91,    53,    53,    53,     3,     3,
       3,     3,     3,    53,   107,   108,   107,     3,    85,    87,
      88,    85,   113,   114,     0,    65,    58,    66,    40,    48,
      47,    41,    42,    43,    44,    45,    46,    31,    32,    50,
      51,    33,    34,    35,    36,    37,    38,    53,    60,    85,
      85,    85,    85,    85,    85,    54,    59,    83,    19,    57,
      85,    85,     3,    53,    53,    74,    28,    71,    53,    60,
      59,    54,    54,    57,    56,    58,    66,    66,    85,    99,
     100,   101,   101,   102,   102,   102,   102,     3,    84,    84,
     104,   104,   105,   105,   106,   106,   106,   106,    85,   110,
     111,     3,    54,    54,    54,    54,    54,    57,    84,    39,
      85,    91,    54,    54,    19,     3,    79,    80,    81,    79,
      29,    75,     3,    55,   110,     3,    84,    59,    82,     3,
      88,    85,    35,    54,    57,    85,    85,    85,    85,    85,
      83,    54,    57,    54,     3,    55,    72,    54,    84,    52,
      85,    54,    93,    54,    82,    81,    53,    76,    77,     3,
      56,    73,    85,    21,    22,    85,    52,   112,   110,     3,
      56,    78,    53,    53,    85,    85,    54,    53,    83,    79,
      85,    58,    79,    39,    54,    54,    54,    85,    82,    85,
      82,    58,    58,    52,   112,    85,    58
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    62,    63,    64,    64,    65,    65,    66,    66,    67,
      67,    67,    68,    68,    69,    70,    71,    71,    72,    72,
      73,    74,    74,    75,    75,    76,    76,    77,    77,    78,
      78,    78,    79,    79,    80,    80,    81,    82,    82,    83,
      83,    84,    84,    85,    85,    85,    85,    85,    85,    86,
      87,    87,    88,    89,    90,    90,    91,    92,    93,    93,
      94,    95,    96,    96,    97,    97,    98,    98,    99,    99,
     100,   100,   100,   101,   101,   101,   101,   101,   102,   102,
     102,   103,   103,   103,   104,   104,   104,   105,   105,   105,
     105,   106,   106,   107,   107,   107,   108,   108,   108,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   110,   110,   111,   111,
     112,   113,   113,   114,   114
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     1,     2,     3,     1,     1,     1,     0,     1,
       1,     1,     9,     7,     7,     6,     2,     0,     0,     2,
       6,     3,     0,     3,     0,     3,     0,     0,     2,     5,
       8,     6,     1,     0,     1,     3,     2,     2,     0,     2,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     6,
       1,     3,     3,     4,     1,     3,     4,     8,     0,     6,
       5,     7,     3,     1,     1,     3,     3,     1,     3,     1,
       3,     3,     1,     3,     3,     3,     3,     1,     1,     3,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       1,     3,     1,     2,     2,     1,     1,     4,     3,     1,
       1,     1,     1,     1,     5,     3,     1,     4,     4,     4,
       4,     3,     4,     6,     1,     1,     1,     0,     1,     3,
       3,     0,     2,     1,     3
  };




#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   165,   165,   184,   188,   205,   210,   219,   220,   224,
     228,   232,   239,   248,   260,   282,   294,   299,   306,   309,
     317,   324,   329,   335,   340,   346,   351,   358,   361,   369,
     384,   399,   417,   422,   428,   434,   442,   453,   458,   464,
     469,   475,   479,   486,   490,   494,   498,   502,   506,   513,
     525,   531,   539,   546,   554,   560,   568,   580,   589,   592,
     600,   608,   616,   625,   632,   636,   643,   650,   657,   664,
     671,   678,   685,   692,   699,   706,   713,   720,   727,   731,
     736,   744,   751,   758,   765,   772,   779,   786,   793,   800,
     807,   814,   821,   828,   835,   840,   847,   851,   865,   873,
     878,   883,   888,   893,   898,   903,   907,   911,   916,   923,
     930,   937,   943,   950,   958,   963,   971,   976,   982,   988,
     996,  1005,  1008,  1015,  1021
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
#line 2907 "src/parser/parser.cpp"

#line 1028 "src/parser/grammar.y"


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
