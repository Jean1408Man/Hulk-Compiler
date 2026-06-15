#pragma once

#include <bitset>
#include <initializer_list>
#include <memory>
#include <string_view>

namespace hulk::lexer::regex {

using CharSet = std::bitset<256>;

enum class NodeKind {
    Char,    
    Concat,  // a · b
    Alt,     // a | b
    Star,    // a*
    Plus,  
    Opt,     // a?
};

struct RegexNode;
using RegexPtr = std::shared_ptr<RegexNode>;

struct RegexNode {
    NodeKind kind;
    CharSet  set; 
    RegexPtr a;    
    RegexPtr b;  
};

inline RegexPtr chars(const CharSet& set) {
    auto n = std::make_shared<RegexNode>();
    n->kind = NodeKind::Char;
    n->set = set;
    return n;
}

inline RegexPtr lit(char c) {
    CharSet set;
    set.set(static_cast<unsigned char>(c));
    return chars(set);
}

inline RegexPtr range(unsigned char lo, unsigned char hi) {
    CharSet set;
    for (unsigned int c = lo; c <= hi; ++c) {
        set.set(c);
    }
    return chars(set);
}

inline RegexPtr any_of(std::string_view cs) {
    CharSet set;
    for (char c : cs) {
        set.set(static_cast<unsigned char>(c));
    }
    return chars(set);
}

inline RegexPtr concat(RegexPtr a, RegexPtr b) {
    auto n = std::make_shared<RegexNode>();
    n->kind = NodeKind::Concat;
    n->a = std::move(a);
    n->b = std::move(b);
    return n;
}

inline RegexPtr alt(RegexPtr a, RegexPtr b) {
    auto n = std::make_shared<RegexNode>();
    n->kind = NodeKind::Alt;
    n->a = std::move(a);
    n->b = std::move(b);
    return n;
}

inline RegexPtr star(RegexPtr a) {
    auto n = std::make_shared<RegexNode>();
    n->kind = NodeKind::Star;
    n->a = std::move(a);
    return n;
}

inline RegexPtr plus(RegexPtr a) {
    auto n = std::make_shared<RegexNode>();
    n->kind = NodeKind::Plus;
    n->a = std::move(a);
    return n;
}

inline RegexPtr opt(RegexPtr a) {
    auto n = std::make_shared<RegexNode>();
    n->kind = NodeKind::Opt;
    n->a = std::move(a);
    return n;
}

inline RegexPtr seq(std::initializer_list<RegexPtr> parts) {
    RegexPtr acc;
    for (const RegexPtr& p : parts) {
        acc = acc ? concat(acc, p) : p;
    }
    return acc;
}

inline RegexPtr str(std::string_view literal) {
    RegexPtr acc;
    for (char c : literal) {
        RegexPtr l = lit(c);
        acc = acc ? concat(acc, l) : l;
    }
    return acc;
}

} 