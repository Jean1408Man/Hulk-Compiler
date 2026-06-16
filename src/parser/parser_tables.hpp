#pragma once

#include <vector>

#include "../common/span.hpp"
#include "parser_driver.hpp"
#include "symbol.hpp"

namespace hulk::parser::parser_tables {

struct symbol_id {
    static constexpr int END = 0;
    static constexpr int IDENTIFIER = 1;
    static constexpr int STRING_LITERAL = 2;
    static constexpr int ERROR_TOKEN = 3;
    static constexpr int NUMBER_LITERAL = 4;
    static constexpr int TRUE = 5;
    static constexpr int FALSE = 6;
    static constexpr int PRINT = 7;
    static constexpr int SQRT = 8;
    static constexpr int SIN = 9;
    static constexpr int COS = 10;
    static constexpr int EXP = 11;
    static constexpr int LOG = 12;
    static constexpr int RAND = 13;
    static constexpr int PI_CONST = 14;
    static constexpr int E_CONST = 15;
    static constexpr int LET = 16;
    static constexpr int IN = 17;
    static constexpr int IF = 18;
    static constexpr int ELIF = 19;
    static constexpr int ELSE = 20;
    static constexpr int WHILE = 21;
    static constexpr int FOR = 22;
    static constexpr int FUNCTION = 23;
    static constexpr int TYPE = 24;
    static constexpr int PROTOCOL = 25;
    static constexpr int EXTENDS = 26;
    static constexpr int INHERITS = 27;
    static constexpr int NEW = 28;
    static constexpr int IS = 29;
    static constexpr int AS = 30;
    static constexpr int SELF = 31;
    static constexpr int BASE = 32;
    static constexpr int PLUS = 33;
    static constexpr int MINUS = 34;
    static constexpr int STAR = 35;
    static constexpr int SLASH = 36;
    static constexpr int PERCENT = 37;
    static constexpr int CARET = 38;
    static constexpr int ASSIGN = 39;
    static constexpr int DESTRUCTIVE_ASSIGN = 40;
    static constexpr int EQUAL_EQUAL = 41;
    static constexpr int NOT_EQUAL = 42;
    static constexpr int LESS = 43;
    static constexpr int LESS_EQUAL = 44;
    static constexpr int GREATER = 45;
    static constexpr int GREATER_EQUAL = 46;
    static constexpr int AND = 47;
    static constexpr int OR = 48;
    static constexpr int NOT = 49;
    static constexpr int CONCAT = 50;
    static constexpr int DOUBLECONCAT = 51;
    static constexpr int FATARROW = 52;
    static constexpr int LPAREN = 53;
    static constexpr int RPAREN = 54;
    static constexpr int LBRACE = 55;
    static constexpr int RBRACE = 56;
    static constexpr int COMMA = 57;
    static constexpr int SEMICOLON = 58;
    static constexpr int COLON = 59;
    static constexpr int DOT = 60;
    static constexpr int UNDERSCORE = 61;
    static constexpr int AUTO = 62;
    static constexpr int UMINUS = 63;
    static constexpr int program = 64;
    static constexpr int expr = 65;
    static constexpr int let_expr = 66;
    static constexpr int if_expr = 67;
    static constexpr int while_expr = 68;
    static constexpr int for_expr = 69;
    static constexpr int assign_expr = 70;
    static constexpr int logic_or = 71;
    static constexpr int logic_and = 72;
    static constexpr int equality = 73;
    static constexpr int relation = 74;
    static constexpr int type_test_expr = 75;
    static constexpr int concat = 76;
    static constexpr int additive = 77;
    static constexpr int multiplicative = 78;
    static constexpr int power = 79;
    static constexpr int unary = 80;
    static constexpr int postfix = 81;
    static constexpr int primary = 82;
    static constexpr int block = 83;
    static constexpr int decl = 84;
    static constexpr int function_decl = 85;
    static constexpr int type_decl = 86;
    static constexpr int protocol_decl = 87;
    static constexpr int binding_list = 88;
    static constexpr int binding = 89;
    static constexpr int expr_list = 90;
    static constexpr int args_opt = 91;
    static constexpr int arg_list = 92;
    static constexpr int block_body_opt = 93;
    static constexpr int parent_args_opt = 94;
    static constexpr int params_opt = 95;
    static constexpr int param_list = 96;
    static constexpr int ctor_params_opt = 97;
    static constexpr int protocol_member_list = 98;
    static constexpr int protocol_member = 99;
    static constexpr int param = 100;
    static constexpr int type_expr = 101;
    static constexpr int type_ann_opt = 102;
    static constexpr int return_ann_opt = 103;
    static constexpr int elif_clauses = 104;
    static constexpr int inherits_opt = 105;
    static constexpr int protocol_extends_opt = 106;
    static constexpr int type_member = 107;
    static constexpr int type_member_list = 108;
    static constexpr int lvalue = 109;
    static constexpr int top_level_items = 110;
    static constexpr int top_level_item = 111;
    static constexpr int opt_semi = 112;
};

inline constexpr int terminal_count = 64;
inline constexpr int nonterminal_count = 49;
inline constexpr int symbol_count = 114;
inline constexpr int state_count = 253;
inline constexpr int rule_count = 126;
inline constexpr int error_action = 0;
inline constexpr int accept_action = 2147483647;

int action(int state, int terminal);
int go_to(int state, int lhs_symbol);
int rule_lhs(int rule);
int rule_len(int rule);
const char* symbol_name(int symbol);
hulk::common::Span merged_span(const std::vector<StackEntry>& stack,
                              std::size_t base,
                              int rhs_len);
void run_semantic_action(int rule,
                         std::vector<StackEntry>& stack,
                         std::size_t base,
                         hulk::common::Span& result_span,
                         ParserValue& result_value,
                         ParserDriver& driver);

}
