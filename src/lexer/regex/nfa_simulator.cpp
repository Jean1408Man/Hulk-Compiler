#include "nfa_simulator.hpp"
#include <vector>

namespace hulk::lexer::regex {
namespace {

void add_with_closure(const Nfa& nfa, int s,
                      std::vector<char>& in_set,
                      std::vector<int>& set,
                      std::vector<int>& stack) {
    if (in_set[s]) {
        return;
    }
    in_set[s] = 1;
    set.push_back(s);
    stack.push_back(s);

    while (!stack.empty()) {
        const int cur = stack.back();
        stack.pop_back();
        for (const int next : nfa.states[cur].eps) {
            if (!in_set[next]) {
                in_set[next] = 1;
                set.push_back(next);
                stack.push_back(next);
            }
        }
    }
}

MatchResult best_accept(const Nfa& nfa, const std::vector<int>& set,
                        std::size_t length) {
    MatchResult result;
    int best_priority = -1;
    for (const int s : set) {
        const State& st = nfa.states[s];
        if (!st.accepting) {
            continue;
        }
        if (!result.matched || st.priority < best_priority) {
            result.matched = true;
            result.length = length;
            result.kind = st.kind;
            best_priority = st.priority;
        }
    }
    return result;
}

}

MatchResult longest_match(const Nfa& nfa,
                          const std::function<int(std::size_t)>& peek) {
    const std::size_t n = nfa.states.size();

    std::vector<char> in_current(n, 0);
    std::vector<int>  current;
    std::vector<int>  stack;
    add_with_closure(nfa, nfa.start, in_current, current, stack);

    MatchResult best;  // matched=false ⇒ no hay aceptación desde S0

    {
        const MatchResult acc = best_accept(nfa, current, 0);
        if (acc.matched) {
            best = acc;
        }
    }

    std::size_t pos = 0;
    while (!current.empty()) {
        const int c = peek(pos);
        if (c < 0 || c > 255) {
            break;
        }

        std::vector<char> in_next(n, 0);
        std::vector<int>  next;
        std::vector<int>  next_stack;
        for (const int s : current) {
            for (const Transition& t : nfa.states[s].trans) {
                if (t.set.test(static_cast<std::size_t>(c))) {
                    add_with_closure(nfa, t.target, in_next, next, next_stack);
                }
            }
        }

        if (next.empty()) {
            break;
        }

        ++pos;
        current.swap(next);
        in_current.swap(in_next);

        // Maximal munch: cada aceptación a mayor `pos` sobrescribe a la anterior.
        const MatchResult acc = best_accept(nfa, current, pos);
        if (acc.matched) {
            best = acc;
        }
    }

    return best;
}

}