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
#line 97 "src/parser/grammar.y"

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
#line 168 "src/parser/grammar.y"
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
#line 1093 "src/parser/parser.cpp"
    break;

  case 3: // top_level_items: top_level_item opt_semi
#line 187 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = std::move(yystack_[1].value.as < hulk::parser::TopLevelItems > ());
      }
#line 1101 "src/parser/parser.cpp"
    break;

  case 4: // top_level_items: top_level_items top_level_item opt_semi
#line 191 "src/parser/grammar.y"
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
#line 1119 "src/parser/parser.cpp"
    break;

  case 5: // top_level_item: decl
#line 208 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = hulk::parser::TopLevelItems {};
          yylhs.value.as < hulk::parser::TopLevelItems > ().decls.push_back(std::move(yystack_[0].value.as < DeclPtr > ()));
      }
#line 1128 "src/parser/parser.cpp"
    break;

  case 6: // top_level_item: expr
#line 213 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::TopLevelItems > () = hulk::parser::TopLevelItems {};
          yylhs.value.as < hulk::parser::TopLevelItems > ().globalExpr = std::move(yystack_[0].value.as < ExprPtr > ());
          yylhs.value.as < hulk::parser::TopLevelItems > ().hasGlobalExpr = true;
      }
#line 1138 "src/parser/parser.cpp"
    break;

  case 9: // decl: function_decl
#line 227 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1146 "src/parser/parser.cpp"
    break;

  case 10: // decl: type_decl
#line 231 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1154 "src/parser/parser.cpp"
    break;

  case 11: // decl: protocol_decl
#line 235 "src/parser/grammar.y"
      {
          yylhs.value.as < DeclPtr > () = std::move(yystack_[0].value.as < DeclPtr > ());
      }
#line 1162 "src/parser/parser.cpp"
    break;

  case 12: // function_decl: FUNCTION IDENTIFIER LPAREN params_opt RPAREN return_ann_opt FATARROW expr SEMICOLON
#line 242 "src/parser/grammar.y"
      {
          if (yystack_[3].value.as < std::string > ().empty()) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[7].value.as < std::string > (), std::move(yystack_[5].value.as < ParamList > ()), std::move(yystack_[1].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[7].value.as < std::string > (), std::move(yystack_[5].value.as < ParamList > ()), yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < ExprPtr > ()));
          }
          yylhs.value.as < DeclPtr > ()->span = to_span(yylhs.location);
      }
#line 1175 "src/parser/parser.cpp"
    break;

  case 13: // function_decl: FUNCTION IDENTIFIER LPAREN params_opt RPAREN return_ann_opt block
#line 251 "src/parser/grammar.y"
      {
          if (yystack_[1].value.as < std::string > ().empty()) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::FunctionDecl>(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), yystack_[1].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < DeclPtr > ()->span = to_span(yylhs.location);
      }
#line 1188 "src/parser/parser.cpp"
    break;

  case 14: // type_decl: TYPE IDENTIFIER ctor_params_opt inherits_opt LBRACE type_member_list RBRACE
#line 263 "src/parser/grammar.y"
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
#line 1211 "src/parser/parser.cpp"
    break;

  case 15: // protocol_decl: PROTOCOL IDENTIFIER protocol_extends_opt LBRACE protocol_member_list RBRACE
#line 285 "src/parser/grammar.y"
      {
          if (yystack_[3].value.as < hulk::parser::InheritsInfo > ().hasParent) {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::ProtocolDecl>(yystack_[4].value.as < std::string > (), yystack_[3].value.as < hulk::parser::InheritsInfo > ().parentName, std::move(yystack_[1].value.as < ProtocolMethodList > ()));
          } else {
              yylhs.value.as < DeclPtr > () = std::make_unique<Hulk::ProtocolDecl>(yystack_[4].value.as < std::string > (), std::move(yystack_[1].value.as < ProtocolMethodList > ()));
          }
          yylhs.value.as < DeclPtr > ()->span = to_span(yylhs.location);
      }
#line 1224 "src/parser/parser.cpp"
    break;

  case 16: // protocol_extends_opt: EXTENDS IDENTIFIER
#line 297 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo { yystack_[0].value.as < std::string > (), ExprList {}, true };
      }
#line 1232 "src/parser/parser.cpp"
    break;

  case 17: // protocol_extends_opt: %empty
#line 301 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo {};
      }
#line 1240 "src/parser/parser.cpp"
    break;

  case 18: // protocol_member_list: %empty
#line 308 "src/parser/grammar.y"
      {
          yylhs.value.as < ProtocolMethodList > () = hulk::parser::ProtocolMethodList {};
      }
#line 1248 "src/parser/parser.cpp"
    break;

  case 19: // protocol_member_list: protocol_member_list protocol_member
#line 312 "src/parser/grammar.y"
      {
          yystack_[1].value.as < ProtocolMethodList > ().push_back(std::move(yystack_[0].value.as < Hulk::ProtocolMethodSig > ()));
          yylhs.value.as < ProtocolMethodList > () = std::move(yystack_[1].value.as < ProtocolMethodList > ());
      }
#line 1257 "src/parser/parser.cpp"
    break;

  case 20: // protocol_member: IDENTIFIER LPAREN params_opt RPAREN return_ann_opt SEMICOLON
#line 320 "src/parser/grammar.y"
      {
          yylhs.value.as < Hulk::ProtocolMethodSig > () = Hulk::ProtocolMethodSig(yystack_[5].value.as < std::string > (), std::move(yystack_[3].value.as < ParamList > ()), yystack_[1].value.as < std::string > ());
      }
#line 1265 "src/parser/parser.cpp"
    break;

  case 21: // ctor_params_opt: LPAREN params_opt RPAREN
#line 327 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = std::move(yystack_[1].value.as < ParamList > ());
      }
#line 1273 "src/parser/parser.cpp"
    break;

  case 22: // ctor_params_opt: %empty
#line 331 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = ParamList {};
      }
#line 1281 "src/parser/parser.cpp"
    break;

  case 23: // inherits_opt: INHERITS IDENTIFIER parent_args_opt
#line 338 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo { yystack_[1].value.as < std::string > (), std::move(yystack_[0].value.as < ExprList > ()), true };
      }
#line 1289 "src/parser/parser.cpp"
    break;

  case 24: // inherits_opt: %empty
#line 342 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::InheritsInfo > () = hulk::parser::InheritsInfo {};
      }
#line 1297 "src/parser/parser.cpp"
    break;

  case 25: // parent_args_opt: LPAREN args_opt RPAREN
#line 349 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[1].value.as < ExprList > ());
      }
#line 1305 "src/parser/parser.cpp"
    break;

  case 26: // parent_args_opt: %empty
#line 353 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 1313 "src/parser/parser.cpp"
    break;

  case 27: // type_member_list: %empty
#line 360 "src/parser/grammar.y"
      {
          yylhs.value.as < TypeMemberList > () = TypeMemberList {};
      }
#line 1321 "src/parser/parser.cpp"
    break;

  case 28: // type_member_list: type_member_list type_member
#line 364 "src/parser/grammar.y"
      {
          yystack_[1].value.as < TypeMemberList > ().push_back(std::move(yystack_[0].value.as < Hulk::TypeMember > ()));
          yylhs.value.as < TypeMemberList > () = std::move(yystack_[1].value.as < TypeMemberList > ());
      }
#line 1330 "src/parser/parser.cpp"
    break;

  case 29: // type_member: IDENTIFIER type_ann_opt ASSIGN expr SEMICOLON
#line 372 "src/parser/grammar.y"
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
#line 1349 "src/parser/parser.cpp"
    break;

  case 30: // type_member: IDENTIFIER LPAREN params_opt RPAREN return_ann_opt FATARROW expr SEMICOLON
#line 387 "src/parser/grammar.y"
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
#line 1368 "src/parser/parser.cpp"
    break;

  case 31: // type_member: IDENTIFIER LPAREN params_opt RPAREN return_ann_opt block
#line 402 "src/parser/grammar.y"
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
#line 1387 "src/parser/parser.cpp"
    break;

  case 32: // params_opt: param_list
#line 420 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = std::move(yystack_[0].value.as < ParamList > ());
      }
#line 1395 "src/parser/parser.cpp"
    break;

  case 33: // params_opt: %empty
#line 424 "src/parser/grammar.y"
      {
          yylhs.value.as < ParamList > () = ParamList {};
      }
#line 1403 "src/parser/parser.cpp"
    break;

  case 34: // param_list: param
#line 431 "src/parser/grammar.y"
      {
          ParamList params;
          params.push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(params);
      }
#line 1413 "src/parser/parser.cpp"
    break;

  case 35: // param_list: param_list COMMA param
#line 437 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ParamList > ().push_back(std::move(yystack_[0].value.as < Hulk::Param > ()));
          yylhs.value.as < ParamList > () = std::move(yystack_[2].value.as < ParamList > ());
      }
#line 1422 "src/parser/parser.cpp"
    break;

  case 36: // param: IDENTIFIER type_ann_opt
#line 445 "src/parser/grammar.y"
      {
          if (yystack_[0].value.as < std::string > ().empty()) {
              yylhs.value.as < Hulk::Param > () = Hulk::Param(yystack_[1].value.as < std::string > ());
          } else {
              yylhs.value.as < Hulk::Param > () = Hulk::Param(yystack_[1].value.as < std::string > (), yystack_[0].value.as < std::string > ());
          }
      }
#line 1434 "src/parser/parser.cpp"
    break;

  case 37: // return_ann_opt: COLON type_expr
#line 456 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1442 "src/parser/parser.cpp"
    break;

  case 38: // return_ann_opt: %empty
#line 460 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "";
      }
#line 1450 "src/parser/parser.cpp"
    break;

  case 39: // type_ann_opt: COLON type_expr
#line 467 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1458 "src/parser/parser.cpp"
    break;

  case 40: // type_ann_opt: %empty
#line 471 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "";
      }
#line 1466 "src/parser/parser.cpp"
    break;

  case 41: // type_expr: IDENTIFIER
#line 478 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[0].value.as < std::string > ());
      }
#line 1474 "src/parser/parser.cpp"
    break;

  case 42: // type_expr: type_expr STAR
#line 482 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = std::move(yystack_[1].value.as < std::string > ()) + "*";
      }
#line 1482 "src/parser/parser.cpp"
    break;

  case 43: // type_expr: UNDERSCORE
#line 486 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "_";
      }
#line 1490 "src/parser/parser.cpp"
    break;

  case 44: // type_expr: AUTO
#line 490 "src/parser/grammar.y"
      {
          yylhs.value.as < std::string > () = "auto";
      }
#line 1498 "src/parser/parser.cpp"
    break;

  case 45: // expr: let_expr
#line 497 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1506 "src/parser/parser.cpp"
    break;

  case 46: // expr: if_expr
#line 501 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1514 "src/parser/parser.cpp"
    break;

  case 47: // expr: while_expr
#line 505 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1522 "src/parser/parser.cpp"
    break;

  case 48: // expr: for_expr
#line 509 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1530 "src/parser/parser.cpp"
    break;

  case 49: // expr: assign_expr
#line 513 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1538 "src/parser/parser.cpp"
    break;

  case 50: // let_expr: LET binding_list IN expr
#line 520 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LetIn>(std::move(yystack_[2].value.as < BindingList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1547 "src/parser/parser.cpp"
    break;

  case 51: // binding_list: binding
#line 528 "src/parser/grammar.y"
      {
          BindingList bindings;
          bindings.push_back(std::move(yystack_[0].value.as < BindingPtr > ()));
          yylhs.value.as < BindingList > () = std::move(bindings);
      }
#line 1557 "src/parser/parser.cpp"
    break;

  case 52: // binding_list: binding_list COMMA binding
#line 534 "src/parser/grammar.y"
      {
          yystack_[2].value.as < BindingList > ().push_back(std::move(yystack_[0].value.as < BindingPtr > ()));
          yylhs.value.as < BindingList > () = std::move(yystack_[2].value.as < BindingList > ());
      }
#line 1566 "src/parser/parser.cpp"
    break;

  case 53: // binding: IDENTIFIER type_ann_opt ASSIGN expr
#line 542 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < std::string > ().empty()) {
              yylhs.value.as < BindingPtr > () = std::make_unique<Hulk::VariableBinding>(yystack_[3].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < BindingPtr > () = std::make_unique<Hulk::VariableBinding>(yystack_[3].value.as < std::string > (), yystack_[2].value.as < std::string > (), std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < BindingPtr > ()->span = to_span(yylhs.location);
      }
#line 1579 "src/parser/parser.cpp"
    break;

  case 54: // if_expr: IF LPAREN expr RPAREN expr elif_clauses ELSE expr
#line 554 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::IfStmt>(std::move(yystack_[5].value.as < ExprPtr > ()), std::move(yystack_[3].value.as < ExprPtr > ()), std::move(yystack_[2].value.as < ElifList > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1588 "src/parser/parser.cpp"
    break;

  case 55: // elif_clauses: %empty
#line 562 "src/parser/grammar.y"
      {
          yylhs.value.as < ElifList > () = ElifList {};
      }
#line 1596 "src/parser/parser.cpp"
    break;

  case 56: // elif_clauses: elif_clauses ELIF LPAREN expr RPAREN expr
#line 566 "src/parser/grammar.y"
      {
          yystack_[5].value.as < ElifList > ().emplace_back(std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ElifList > () = std::move(yystack_[5].value.as < ElifList > ());
      }
#line 1605 "src/parser/parser.cpp"
    break;

  case 57: // while_expr: WHILE LPAREN expr RPAREN expr
#line 574 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::WhileStmt>(std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1614 "src/parser/parser.cpp"
    break;

  case 58: // for_expr: FOR LPAREN IDENTIFIER IN expr RPAREN expr
#line 582 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::For>(yystack_[4].value.as < std::string > (), std::move(yystack_[2].value.as < ExprPtr > ()), std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1623 "src/parser/parser.cpp"
    break;

  case 59: // assign_expr: lvalue DESTRUCTIVE_ASSIGN expr
#line 590 "src/parser/grammar.y"
      {
          if (yystack_[2].value.as < hulk::parser::LValueTarget > ().isMember) {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::DestructiveAssignMember>(std::move(yystack_[2].value.as < hulk::parser::LValueTarget > ().object), yystack_[2].value.as < hulk::parser::LValueTarget > ().name, std::move(yystack_[0].value.as < ExprPtr > ()));
          } else {
              yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::DestructiveAssign>(yystack_[2].value.as < hulk::parser::LValueTarget > ().name, std::move(yystack_[0].value.as < ExprPtr > ()));
          }
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1636 "src/parser/parser.cpp"
    break;

  case 60: // assign_expr: logic_or
#line 599 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1644 "src/parser/parser.cpp"
    break;

  case 61: // lvalue: IDENTIFIER
#line 606 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { nullptr, yystack_[0].value.as < std::string > (), false };
      }
#line 1652 "src/parser/parser.cpp"
    break;

  case 62: // lvalue: postfix DOT IDENTIFIER
#line 610 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > (), true };
      }
#line 1660 "src/parser/parser.cpp"
    break;

  case 63: // lvalue: SELF
#line 614 "src/parser/grammar.y"
      {
          yylhs.value.as < hulk::parser::LValueTarget > () = hulk::parser::LValueTarget { nullptr, "self", false };
      }
#line 1668 "src/parser/parser.cpp"
    break;

  case 64: // logic_or: logic_or OR logic_and
#line 621 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Or, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1679 "src/parser/parser.cpp"
    break;

  case 65: // logic_or: logic_and
#line 628 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1687 "src/parser/parser.cpp"
    break;

  case 66: // logic_and: logic_and AND equality
#line 635 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::And, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1698 "src/parser/parser.cpp"
    break;

  case 67: // logic_and: equality
#line 642 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1706 "src/parser/parser.cpp"
    break;

  case 68: // equality: equality EQUAL_EQUAL relation
#line 649 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Equal, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1717 "src/parser/parser.cpp"
    break;

  case 69: // equality: equality NOT_EQUAL relation
#line 656 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::NotEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1728 "src/parser/parser.cpp"
    break;

  case 70: // equality: relation
#line 663 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1736 "src/parser/parser.cpp"
    break;

  case 71: // relation: relation LESS type_test_expr
#line 670 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Less, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1747 "src/parser/parser.cpp"
    break;

  case 72: // relation: relation LESS_EQUAL type_test_expr
#line 677 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::LessEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1758 "src/parser/parser.cpp"
    break;

  case 73: // relation: relation GREATER type_test_expr
#line 684 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::Greater, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1769 "src/parser/parser.cpp"
    break;

  case 74: // relation: relation GREATER_EQUAL type_test_expr
#line 691 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::LogicOp::GreaterEqual, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1780 "src/parser/parser.cpp"
    break;

  case 75: // relation: type_test_expr
#line 698 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1788 "src/parser/parser.cpp"
    break;

  case 76: // type_test_expr: concat
#line 705 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1796 "src/parser/parser.cpp"
    break;

  case 77: // type_test_expr: concat IS type_expr
#line 709 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::IsExpr>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1805 "src/parser/parser.cpp"
    break;

  case 78: // type_test_expr: concat AS type_expr
#line 714 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::AsExpr>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1814 "src/parser/parser.cpp"
    break;

  case 79: // concat: concat CONCAT additive
#line 722 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::StringBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::StringOp::Concat, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1825 "src/parser/parser.cpp"
    break;

  case 80: // concat: concat DOUBLECONCAT additive
#line 729 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::StringBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::StringOp::SpaceConcat, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1836 "src/parser/parser.cpp"
    break;

  case 81: // concat: additive
#line 736 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1844 "src/parser/parser.cpp"
    break;

  case 82: // additive: additive PLUS multiplicative
#line 743 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Plus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1855 "src/parser/parser.cpp"
    break;

  case 83: // additive: additive MINUS multiplicative
#line 750 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Minus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1866 "src/parser/parser.cpp"
    break;

  case 84: // additive: multiplicative
#line 757 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1874 "src/parser/parser.cpp"
    break;

  case 85: // multiplicative: multiplicative STAR power
#line 764 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Mult, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1885 "src/parser/parser.cpp"
    break;

  case 86: // multiplicative: multiplicative SLASH power
#line 771 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Div, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1896 "src/parser/parser.cpp"
    break;

  case 87: // multiplicative: multiplicative PERCENT power
#line 778 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Mod, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1907 "src/parser/parser.cpp"
    break;

  case 88: // multiplicative: power
#line 785 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1915 "src/parser/parser.cpp"
    break;

  case 89: // power: unary CARET power
#line 792 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticBinOp>(
              std::move(yystack_[2].value.as < ExprPtr > ()), Hulk::ArithmeticOp::Pow, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1926 "src/parser/parser.cpp"
    break;

  case 90: // power: unary
#line 799 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1934 "src/parser/parser.cpp"
    break;

  case 91: // unary: MINUS unary
#line 806 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ArithmeticUnaryOp>(
              Hulk::ArithUnaryType::Minus, std::move(yystack_[0].value.as < ExprPtr > ())
          );
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1945 "src/parser/parser.cpp"
    break;

  case 92: // unary: NOT unary
#line 813 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::LogicUnaryOp>(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1954 "src/parser/parser.cpp"
    break;

  case 93: // unary: postfix
#line 818 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1962 "src/parser/parser.cpp"
    break;

  case 94: // postfix: primary
#line 825 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 1970 "src/parser/parser.cpp"
    break;

  case 95: // postfix: postfix LPAREN args_opt RPAREN
#line 829 "src/parser/grammar.y"
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
#line 1988 "src/parser/parser.cpp"
    break;

  case 96: // postfix: postfix DOT IDENTIFIER
#line 843 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::MemberAccess>(std::move(yystack_[2].value.as < ExprPtr > ()), yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 1997 "src/parser/parser.cpp"
    break;

  case 97: // primary: NUMBER_LITERAL
#line 851 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(yystack_[0].value.as < double > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2006 "src/parser/parser.cpp"
    break;

  case 98: // primary: STRING_LITERAL
#line 856 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::String>(yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2015 "src/parser/parser.cpp"
    break;

  case 99: // primary: TRUE
#line 861 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Boolean>(true);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2024 "src/parser/parser.cpp"
    break;

  case 100: // primary: FALSE
#line 866 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Boolean>(false);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2033 "src/parser/parser.cpp"
    break;

  case 101: // primary: IDENTIFIER
#line 871 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::VariableReference>(yystack_[0].value.as < std::string > ());
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2042 "src/parser/parser.cpp"
    break;

  case 102: // primary: SELF
#line 876 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::SelfRef>();
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2051 "src/parser/parser.cpp"
    break;

  case 103: // primary: BASE LPAREN args_opt RPAREN
#line 881 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BaseCall>(std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2060 "src/parser/parser.cpp"
    break;

  case 104: // primary: NEW IDENTIFIER LPAREN args_opt RPAREN
#line 886 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::NewExpr>(yystack_[3].value.as < std::string > (), std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2069 "src/parser/parser.cpp"
    break;

  case 105: // primary: LPAREN expr RPAREN
#line 891 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[1].value.as < ExprPtr > ());
      }
#line 2077 "src/parser/parser.cpp"
    break;

  case 106: // primary: block
#line 895 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::move(yystack_[0].value.as < ExprPtr > ());
      }
#line 2085 "src/parser/parser.cpp"
    break;

  case 107: // primary: PRINT LPAREN expr RPAREN
#line 899 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Print>(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2094 "src/parser/parser.cpp"
    break;

  case 108: // primary: SQRT LPAREN expr RPAREN
#line 904 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Sqrt, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2105 "src/parser/parser.cpp"
    break;

  case 109: // primary: SIN LPAREN expr RPAREN
#line 911 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Sin, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2116 "src/parser/parser.cpp"
    break;

  case 110: // primary: COS LPAREN expr RPAREN
#line 918 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Cos, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2127 "src/parser/parser.cpp"
    break;

  case 111: // primary: RAND LPAREN RPAREN
#line 925 "src/parser/grammar.y"
      {
          ExprList args;
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Rand, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2137 "src/parser/parser.cpp"
    break;

  case 112: // primary: EXP LPAREN expr RPAREN
#line 931 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Exp, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2148 "src/parser/parser.cpp"
    break;

  case 113: // primary: LOG LPAREN expr COMMA expr RPAREN
#line 938 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[3].value.as < ExprPtr > ()));
          args.push_back(std::move(yystack_[1].value.as < ExprPtr > ()));
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::BuiltinCall>(Hulk::BuiltinFunc::Log, std::move(args));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2160 "src/parser/parser.cpp"
    break;

  case 114: // primary: PI_CONST
#line 946 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(3.14159265358979323846);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2169 "src/parser/parser.cpp"
    break;

  case 115: // primary: E_CONST
#line 951 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::Number>(2.71828182845904523536);
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2178 "src/parser/parser.cpp"
    break;

  case 116: // args_opt: arg_list
#line 959 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[0].value.as < ExprList > ());
      }
#line 2186 "src/parser/parser.cpp"
    break;

  case 117: // args_opt: %empty
#line 963 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 2194 "src/parser/parser.cpp"
    break;

  case 118: // arg_list: expr
#line 970 "src/parser/grammar.y"
      {
          ExprList args;
          args.push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(args);
      }
#line 2204 "src/parser/parser.cpp"
    break;

  case 119: // arg_list: arg_list COMMA expr
#line 976 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ExprList > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(yystack_[2].value.as < ExprList > ());
      }
#line 2213 "src/parser/parser.cpp"
    break;

  case 120: // block: LBRACE block_body_opt RBRACE
#line 984 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprPtr > () = std::make_unique<Hulk::ExprBlock>(std::move(yystack_[1].value.as < ExprList > ()));
          yylhs.value.as < ExprPtr > ()->span = to_span(yylhs.location);
      }
#line 2222 "src/parser/parser.cpp"
    break;

  case 121: // block_body_opt: %empty
#line 992 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = ExprList {};
      }
#line 2230 "src/parser/parser.cpp"
    break;

  case 122: // block_body_opt: expr_list opt_semi
#line 996 "src/parser/grammar.y"
      {
          yylhs.value.as < ExprList > () = std::move(yystack_[1].value.as < ExprList > ());
      }
#line 2238 "src/parser/parser.cpp"
    break;

  case 123: // expr_list: expr
#line 1003 "src/parser/grammar.y"
      {
          ExprList nodes;
          nodes.push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(nodes);
      }
#line 2248 "src/parser/parser.cpp"
    break;

  case 124: // expr_list: expr_list SEMICOLON expr
#line 1009 "src/parser/grammar.y"
      {
          yystack_[2].value.as < ExprList > ().push_back(std::move(yystack_[0].value.as < ExprPtr > ()));
          yylhs.value.as < ExprList > () = std::move(yystack_[2].value.as < ExprList > ());
      }
#line 2257 "src/parser/parser.cpp"
    break;


#line 2261 "src/parser/parser.cpp"

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
  "type_expr", "expr", "let_expr", "binding_list", "binding", "if_expr",
  "elif_clauses", "while_expr", "for_expr", "assign_expr", "lvalue",
  "logic_or", "logic_and", "equality", "relation", "type_test_expr",
  "concat", "additive", "multiplicative", "power", "unary", "postfix",
  "primary", "args_opt", "arg_list", "block", "block_body_opt",
  "expr_list", YY_NULLPTR
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


  const short Parser::yypact_ninf_ = -199;

  const signed char Parser::yytable_ninf_ = -64;

  const short
  Parser::yypact_[] =
  {
     112,   -27,  -199,  -199,  -199,  -199,   -30,   -23,    37,    41,
      46,    47,    48,  -199,  -199,   101,    53,    54,    57,   103,
     110,   111,   128,    91,    85,   242,   242,   185,   185,   141,
     112,    83,  -199,  -199,  -199,  -199,  -199,  -199,  -199,  -199,
    -199,  -199,   102,    97,   100,    -8,    -7,  -199,   -19,    12,
      51,  -199,   113,   -44,  -199,  -199,   185,   185,   185,   185,
     185,   185,    94,    90,   -10,  -199,   185,   185,   149,    99,
     104,   127,   105,   185,  -199,  -199,  -199,   -41,  -199,   106,
    -199,    98,   114,  -199,    83,  -199,  -199,   185,   242,   242,
     242,   242,   242,   242,   242,   242,     1,     1,   242,   242,
     242,   242,   242,   242,   242,   242,   185,   154,   108,   109,
     119,   120,   121,   122,  -199,     1,   117,   185,   101,   123,
     124,   142,   167,   167,   153,   175,   126,   185,  -199,   130,
     125,   184,  -199,  -199,   185,  -199,  -199,  -199,   100,    -8,
      -7,    -7,  -199,  -199,  -199,  -199,  -199,  -199,  -199,   170,
     170,    12,    12,    51,    51,  -199,  -199,  -199,  -199,   134,
     162,  -199,  -199,  -199,  -199,  -199,   185,   170,   185,  -199,
    -199,   185,   185,   185,    90,   155,   151,  -199,   156,   210,
     157,  -199,  -199,   160,  -199,   185,  -199,  -199,  -199,  -199,
     166,  -199,  -199,  -199,   168,  -199,   164,   167,  -199,   171,
    -199,     4,  -199,  -199,  -199,    33,   185,     1,   -37,  -199,
     185,  -199,     5,   173,  -199,  -199,   174,   185,  -199,   170,
     185,  -199,   176,   -39,  -199,  -199,   167,   185,  -199,   163,
    -199,   167,   189,   177,   178,  -199,   179,   185,   164,   185,
     164,   181,   183,  -199,   -31,  -199,  -199,   185,  -199,   200,
    -199
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,   101,    98,    97,    99,   100,     0,     0,     0,     0,
       0,     0,     0,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,   102,     0,     0,     0,     0,   121,     0,
       2,     8,     5,     9,    10,    11,     6,    45,    46,    47,
      48,    49,     0,    60,    65,    67,    70,    75,    76,    81,
      84,    88,    90,    93,    94,   106,     0,     0,     0,     0,
       0,     0,     0,    40,     0,    51,     0,     0,     0,     0,
      22,    17,     0,   117,   101,   102,    91,    93,    92,     0,
     123,     0,     8,     1,     8,     7,     3,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   117,     0,     0,     0,
       0,     0,     0,     0,   111,     0,     0,     0,     0,     0,
       0,     0,    33,    33,    24,     0,     0,   117,   118,     0,
     116,     0,   105,   120,     7,   122,     4,    59,    64,    66,
      68,    69,    71,    72,    73,    74,    41,    43,    44,    77,
      78,    79,    80,    82,    83,    85,    86,    87,    89,     0,
      96,   107,   108,   109,   110,   112,     0,    39,     0,    50,
      52,     0,     0,     0,    40,     0,    32,    34,     0,     0,
       0,    16,    18,     0,   103,     0,    96,   124,    42,    95,
       0,    53,    55,    57,     0,    36,    38,     0,    21,    26,
      27,     0,   104,   119,   113,     0,     0,     0,     0,    35,
     117,    23,     0,     0,    15,    19,     0,     0,    58,    37,
       0,    13,     0,    40,    14,    28,    33,     0,    54,     0,
      25,    33,     0,     0,     0,    12,     0,     0,    38,     0,
      38,     0,     0,    56,     0,    29,    20,     0,    31,     0,
      30
  };

  const short
  Parser::yypgoto_[] =
  {
    -199,  -199,  -199,   201,   -53,  -199,  -199,  -199,  -199,  -199,
    -199,  -199,  -199,  -199,  -199,  -199,  -199,  -121,  -199,    64,
    -147,  -137,   -96,     0,  -199,  -199,   144,  -199,  -199,  -199,
    -199,  -199,  -199,  -199,   150,   180,   -14,   -50,  -199,    -4,
      -3,   -52,    74,   -20,  -199,  -103,  -199,  -198,  -199,  -199
  };

  const unsigned char
  Parser::yydefgoto_[] =
  {
       0,    29,    30,    31,    86,    32,    33,    34,    35,   126,
     201,   215,   124,   180,   211,   212,   225,   175,   176,   177,
     208,   116,   149,   128,    37,    64,    65,    38,   205,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,   129,   130,    55,    81,    82
  };

  const short
  Parser::yytable_[] =
  {
      36,   150,   178,   159,   146,    77,    77,   213,   223,   117,
     221,   106,    96,    97,   106,   -61,   231,   220,   107,   167,
      28,   131,   115,   247,   183,    56,    28,    79,    80,   135,
      36,   136,    57,    98,    99,    90,    91,   195,    92,    93,
      94,    95,   142,   143,   144,   145,   248,   100,   101,   118,
     155,   156,   157,   158,   216,   217,   108,   109,   110,   111,
     112,   113,   214,   224,   147,   148,   119,   120,    77,    77,
      77,    77,    77,    77,    77,    77,   140,   141,    77,    77,
      77,    77,    77,    77,    77,    77,   232,   137,   102,   103,
     104,   242,    58,   244,   151,   152,    59,   153,   154,    76,
      78,    60,    61,    62,    63,   233,    69,   222,    66,    67,
     236,   219,    68,    70,    71,     1,     2,   169,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    72,    16,   -63,   187,    17,    18,    19,    20,    21,
      73,    83,    22,    85,    87,    23,    24,    88,    25,    89,
     114,   115,   121,   105,   122,   125,   133,   160,   168,   123,
     127,   173,   132,    26,   161,   162,   190,    27,   191,    28,
     174,   192,   193,   194,   134,   163,   164,   165,   181,   171,
     172,   166,   179,   182,   185,   203,   184,   186,     1,     2,
     189,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,   -62,    16,   218,   188,    17,    18,
     197,   196,   198,   199,   200,    22,   202,   228,    23,    24,
     229,    25,   204,   235,   206,   207,   210,   234,   226,   227,
     237,    84,   230,   238,   239,   240,    26,   241,   138,   243,
      27,   245,    28,   246,     0,    74,     2,   249,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
     250,   209,   170,     0,     0,     0,     0,     0,     0,   139,
       0,     0,    22,     0,     0,    75,    24,     0,    25,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    26,     0,     0,     0,    27,     0,    28
  };

  const short
  Parser::yycheck_[] =
  {
       0,    97,   123,   106,     3,    25,    26,     3,     3,    19,
     208,    55,    31,    32,    55,    42,    55,    54,    62,   115,
      57,    62,    61,    54,   127,    55,    57,    27,    28,    82,
      30,    84,    55,    52,    53,    43,    44,   174,    45,    46,
      47,    48,    92,    93,    94,    95,   244,    35,    36,    59,
     102,   103,   104,   105,    21,    22,    56,    57,    58,    59,
      60,    61,    58,    58,    63,    64,    66,    67,    88,    89,
      90,    91,    92,    93,    94,    95,    90,    91,    98,    99,
     100,   101,   102,   103,   104,   105,   223,    87,    37,    38,
      39,   238,    55,   240,    98,    99,    55,   100,   101,    25,
      26,    55,    55,    55,     3,   226,     3,   210,    55,    55,
     231,   207,    55,     3,     3,     3,     4,   117,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,     3,    20,    42,   134,    23,    24,    25,    26,    27,
      55,     0,    30,    60,    42,    33,    34,    50,    36,    49,
      56,    61,     3,    40,    55,    28,    58,     3,    41,    55,
      55,    19,    56,    51,    56,    56,   166,    55,   168,    57,
       3,   171,   172,   173,    60,    56,    56,    56,     3,    56,
      56,    59,    29,    57,    59,   185,    56,     3,     3,     4,
      56,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    42,    20,   206,    37,    23,    24,
      59,    56,    56,     3,    57,    30,    56,   217,    33,    34,
     220,    36,    56,    60,    56,    61,    55,   227,    55,    55,
      41,    30,    56,    56,    56,    56,    51,   237,    88,   239,
      55,    60,    57,    60,    -1,     3,     4,   247,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      60,   197,   118,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      -1,    -1,    30,    -1,    -1,    33,    34,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    55,    -1,    57
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     3,     4,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    20,    23,    24,    25,
      26,    27,    30,    33,    34,    36,    51,    55,    57,    67,
      68,    69,    71,    72,    73,    74,    89,    90,    93,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   113,    55,    55,    55,    55,
      55,    55,    55,     3,    91,    92,    55,    55,    55,     3,
       3,     3,     3,    55,     3,    33,   108,   109,   108,    89,
      89,   114,   115,     0,    69,    60,    70,    42,    50,    49,
      43,    44,    45,    46,    47,    48,    31,    32,    52,    53,
      35,    36,    37,    38,    39,    40,    55,    62,    89,    89,
      89,    89,    89,    89,    56,    61,    87,    19,    59,    89,
      89,     3,    55,    55,    78,    28,    75,    55,    89,   111,
     112,    62,    56,    58,    60,    70,    70,    89,   100,   101,
     102,   102,   103,   103,   103,   103,     3,    63,    64,    88,
      88,   105,   105,   106,   106,   107,   107,   107,   107,   111,
       3,    56,    56,    56,    56,    56,    59,    88,    41,    89,
      92,    56,    56,    19,     3,    83,    84,    85,    83,    29,
      79,     3,    57,   111,    56,    59,     3,    89,    37,    56,
      89,    89,    89,    89,    89,    87,    56,    59,    56,     3,
      57,    76,    56,    89,    56,    94,    56,    61,    86,    85,
      55,    80,    81,     3,    58,    77,    21,    22,    89,    88,
      54,   113,   111,     3,    58,    82,    55,    55,    89,    89,
      56,    55,    87,    83,    89,    60,    83,    41,    56,    56,
      56,    89,    86,    89,    86,    60,    60,    54,   113,    89,
      60
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    66,    67,    68,    68,    69,    69,    70,    70,    71,
      71,    71,    72,    72,    73,    74,    75,    75,    76,    76,
      77,    78,    78,    79,    79,    80,    80,    81,    81,    82,
      82,    82,    83,    83,    84,    84,    85,    86,    86,    87,
      87,    88,    88,    88,    88,    89,    89,    89,    89,    89,
      90,    91,    91,    92,    93,    94,    94,    95,    96,    97,
      97,    98,    98,    98,    99,    99,   100,   100,   101,   101,
     101,   102,   102,   102,   102,   102,   103,   103,   103,   104,
     104,   104,   105,   105,   105,   106,   106,   106,   106,   107,
     107,   108,   108,   108,   109,   109,   109,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   111,   111,   112,   112,
     113,   114,   114,   115,   115
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     1,     2,     3,     1,     1,     1,     0,     1,
       1,     1,     9,     7,     7,     6,     2,     0,     0,     2,
       6,     3,     0,     3,     0,     3,     0,     0,     2,     5,
       8,     6,     1,     0,     1,     3,     2,     2,     0,     2,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     3,     4,     8,     0,     6,     5,     7,     3,
       1,     1,     3,     1,     3,     1,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     1,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     3,     1,     3,
       1,     2,     2,     1,     1,     4,     3,     1,     1,     1,
       1,     1,     1,     4,     5,     3,     1,     4,     4,     4,
       4,     3,     4,     6,     1,     1,     1,     0,     1,     3,
       3,     0,     2,     1,     3
  };




#if YYDEBUG
  const short
  Parser::yyrline_[] =
  {
       0,   167,   167,   186,   190,   207,   212,   221,   222,   226,
     230,   234,   241,   250,   262,   284,   296,   301,   308,   311,
     319,   326,   331,   337,   342,   348,   353,   360,   363,   371,
     386,   401,   419,   424,   430,   436,   444,   455,   460,   466,
     471,   477,   481,   485,   489,   496,   500,   504,   508,   512,
     519,   527,   533,   541,   553,   562,   565,   573,   581,   589,
     598,   605,   609,   613,   620,   627,   634,   641,   648,   655,
     662,   669,   676,   683,   690,   697,   704,   708,   713,   721,
     728,   735,   742,   749,   756,   763,   770,   777,   784,   791,
     798,   805,   812,   817,   824,   828,   842,   850,   855,   860,
     865,   870,   875,   880,   885,   890,   894,   898,   903,   910,
     917,   924,   930,   937,   945,   950,   958,   963,   969,   975,
     983,   992,   995,  1002,  1008
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
#line 2872 "src/parser/parser.cpp"

#line 1015 "src/parser/grammar.y"


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
