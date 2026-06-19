#include "lr_engine.hpp"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "parser_tables.hpp"

namespace hulk::parser {

Parser::Parser(ParserDriver& driver)
    : driver_(driver) {}

std::string Parser::syntax_error_message(int state, const Symbol& lookahead) const {
    std::ostringstream out;
    out << "error de sintaxis";

    if (lookahead.sym >= 0 && lookahead.sym < parser_tables::terminal_count) {
        out << ", no se esperaba " << parser_tables::symbol_name(lookahead.sym);
    } else {
        out << ", token inválido inesperado";
    }

    std::vector<std::string> expected;
    if (state >= 0 && state < parser_tables::state_count) {
        for (int token = 0; token < parser_tables::terminal_count; ++token) {
            if (parser_tables::action(state, token) != parser_tables::error_action) {
                expected.push_back(parser_tables::symbol_name(token));
                if (expected.size() == 5) {
                    break;
                }
            }
        }
    }

    if (!expected.empty()) {
        out << ", se esperaba ";
        for (std::size_t i = 0; i < expected.size(); ++i) {
            if (i > 0) {
                out << (i + 1 == expected.size() ? " o " : ", ");
            }
            out << expected[i];
        }
    }

    return out.str();
}

void Parser::report_syntax_error(int state, const Symbol& lookahead) const {
    driver_.report_syntax_error(syntax_error_message(state, lookahead), lookahead.span);
}

int Parser::parse() {
    std::vector<StackEntry> stack;
    stack.push_back(StackEntry {});

    Symbol lookahead;
    bool has_lookahead = false;

    while (true) {
        const int state = stack.back().state;

        if (!has_lookahead) {
            lookahead = yylex(driver_);
            has_lookahead = true;
        }

        if (lookahead.sym < 0 || lookahead.sym >= parser_tables::terminal_count) {
            report_syntax_error(state, lookahead);
            return 1;
        }

        const int action = parser_tables::action(state, lookahead.sym);

        if (action == parser_tables::accept_action) {
            return 0;
        }

        if (action > 0) {
            stack.push_back(StackEntry {
                .state = action - 1,
                .value = std::move(lookahead.value),
                .span = lookahead.span,
            });
            has_lookahead = false;
            continue;
        }

        if (action < 0) {
            const int rule = -action;
            const int rhs_len = parser_tables::rule_len(rule);
            if (rhs_len < 0 || static_cast<std::size_t>(rhs_len) >= stack.size() + 1) {
                report_syntax_error(state, lookahead);
                return 1;
            }

            const std::size_t base = stack.size() - static_cast<std::size_t>(rhs_len);
            hulk::common::Span result_span =
                parser_tables::merged_span(stack, base, rhs_len);
            ParserValue result_value {};

            parser_tables::run_semantic_action(
                rule, stack, base, result_span, result_value, driver_);

            for (int i = 0; i < rhs_len; ++i) {
                stack.pop_back();
            }

            const int lhs = parser_tables::rule_lhs(rule);
            const int goto_state = parser_tables::go_to(stack.back().state, lhs);
            if (goto_state < 0) {
                report_syntax_error(state, lookahead);
                return 1;
            }

            stack.push_back(StackEntry {
                .state = goto_state,
                .value = std::move(result_value),
                .span = result_span,
            });
            continue;
        }

        report_syntax_error(state, lookahead);
        return 1;
    }
}

}
