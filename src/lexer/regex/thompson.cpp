#include "thompson.hpp"

namespace hulk::lexer::regex {
namespace {

struct Fragment {
    int in;
    int out;
};

class Builder {
public:
    explicit Builder(Nfa& nfa) : nfa_(nfa) {}

    Fragment compile(const RegexPtr& node) {
        switch (node->kind) {
            case NodeKind::Char:   return compile_char(node);
            case NodeKind::Concat: return compile_concat(node);
            case NodeKind::Alt:    return compile_alt(node);
            case NodeKind::Star:   return compile_star(node);
            case NodeKind::Plus:   return compile_plus(node);
            case NodeKind::Opt:    return compile_opt(node);
        }
        return compile_char(node);
    }

private:
    Nfa& nfa_;

    void add_eps(int from, int to) { nfa_.states[from].eps.push_back(to); }

    void add_trans(int from, const CharSet& set, int to) {
        nfa_.states[from].trans.push_back(Transition{set, to});
    }

    // in --[set]--> out
    Fragment compile_char(const RegexPtr& node) {
        const int in = nfa_.add_state();
        const int out = nfa_.add_state();
        add_trans(in, node->set, out);
        return {in, out};
    }

    // out(a) --ε--> in(b)
    Fragment compile_concat(const RegexPtr& node) {
        const Fragment fa = compile(node->a);
        const Fragment fb = compile(node->b);
        add_eps(fa.out, fb.in);
        return {fa.in, fb.out};
    }

    // in --ε--> in(a), in --ε--> in(b); out(a) --ε--> out, out(b) --ε--> out
    Fragment compile_alt(const RegexPtr& node) {
        const int in = nfa_.add_state();
        const int out = nfa_.add_state();
        const Fragment fa = compile(node->a);
        const Fragment fb = compile(node->b);
        add_eps(in, fa.in);
        add_eps(in, fb.in);
        add_eps(fa.out, out);
        add_eps(fb.out, out);
        return {in, out};
    }

    // in --ε--> in(a), in --ε--> out; out(a) --ε--> in(a), out(a) --ε--> out
    Fragment compile_star(const RegexPtr& node) {
        const int in = nfa_.add_state();
        const int out = nfa_.add_state();
        const Fragment fa = compile(node->a);
        add_eps(in, fa.in);
        add_eps(in, out);
        add_eps(fa.out, fa.in);
        add_eps(fa.out, out);
        return {in, out};
    }

    // Requiere al menos una repetición: out(a) --ε--> in(a) (bucle) y --ε--> out.
    Fragment compile_plus(const RegexPtr& node) {
        const int out = nfa_.add_state();
        const Fragment fa = compile(node->a);
        add_eps(fa.out, fa.in);
        add_eps(fa.out, out);
        return {fa.in, out};
    }

    // in --ε--> in(a), in --ε--> out; out(a) --ε--> out
    Fragment compile_opt(const RegexPtr& node) {
        const int in = nfa_.add_state();
        const int out = nfa_.add_state();
        const Fragment fa = compile(node->a);
        add_eps(in, fa.in);
        add_eps(in, out);
        add_eps(fa.out, out);
        return {in, out};
    }
};

} 

Nfa build_nfa(const std::vector<Rule>& rules) {
    Nfa nfa;
    const int start = nfa.add_state();  // S0
    nfa.start = start;

    Builder builder(nfa);
    for (std::size_t i = 0; i < rules.size(); ++i) {
        const Fragment frag = builder.compile(rules[i].regex);
        nfa.states[start].eps.push_back(frag.in);

        State& accept = nfa.states[frag.out];
        accept.accepting = true;
        accept.kind = rules[i].kind;
        accept.priority = static_cast<int>(i);
    }
    return nfa;
}

} 