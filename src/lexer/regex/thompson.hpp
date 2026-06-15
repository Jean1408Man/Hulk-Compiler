#pragma once

#include <vector>
#include "../token_kind.hpp"
#include "nfa.hpp"
#include "regex_ast.hpp"

namespace hulk::lexer::regex {
// Construcción de Thompson: compila una lista ordenada de reglas (TokenKind, regex)
// en un único AFN combinado
struct Rule {
    TokenKind kind;
    RegexPtr  regex;
};

Nfa build_nfa(const std::vector<Rule>& rules);

} 