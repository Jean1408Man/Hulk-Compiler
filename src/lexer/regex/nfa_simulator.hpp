#pragma once

#include <cstddef>
#include <functional>
#include "../token_kind.hpp"
#include "nfa.hpp"

namespace hulk::lexer::regex {
// Simulación de conjunto de estados del AFN, con cierre-ε.
// Implementa maximal munch (lexema más largo) y desempate por orden de declaración
struct MatchResult {
    bool        matched = false;
    std::size_t length  = 0;  
    TokenKind   kind    = TokenKind::Error; 
};

MatchResult longest_match(const Nfa& nfa,
                          const std::function<int(std::size_t)>& peek);

} 