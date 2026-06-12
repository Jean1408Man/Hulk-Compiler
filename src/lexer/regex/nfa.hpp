#pragma once

#include <vector>
#include "../token_kind.hpp"
#include "regex_ast.hpp"

namespace hulk::lexer::regex {

struct Transition {
    CharSet set;
    int     target;
};

struct State {
    std::vector<Transition> trans;  // transiciones por carácter/clase
    std::vector<int>        eps;    // ε-transiciones

    bool      accepting = false;
    int       priority  = -1;    
    TokenKind kind      = TokenKind::Error;
};

struct Nfa {
    std::vector<State> states;
    int                start = 0;

    int add_state() {
        states.emplace_back();
        return static_cast<int>(states.size()) - 1;
    }
};

} 