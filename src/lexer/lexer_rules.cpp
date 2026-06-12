#include "lexer_rules.hpp"
#include <vector>

#include "regex/regex_ast.hpp"
#include "regex/thompson.hpp"

namespace hulk::lexer {
namespace {
using namespace hulk::lexer::regex;

RegexPtr alpha() { return alt(range('a', 'z'), range('A', 'Z')); }
RegexPtr digit() { return range('0', '9'); }
RegexPtr idchar() {
    return alt(alt(range('a', 'z'), range('A', 'Z')),
               alt(range('0', '9'), lit('_')));
}

std::vector<Rule> build_rules() {
    // Orden de declaración = prioridad
    return {
        {TokenKind::Number,
         seq({plus(digit()), opt(seq({lit('.'), plus(digit())}))})},

        {TokenKind::Identifier, concat(alpha(), star(idchar()))},

        {TokenKind::DestructiveAssign, str(":=")},
        {TokenKind::EqualEqual,        str("==")},
        {TokenKind::FatArrow,          str("=>")},
        {TokenKind::NotEqual,          str("!=")},
        {TokenKind::LessEqual,         str("<=")},
        {TokenKind::GreaterEqual,      str(">=")},
        {TokenKind::DoubleConcat,      str("@@")},

        {TokenKind::Plus,    lit('+')},
        {TokenKind::Minus,   lit('-')},
        {TokenKind::Star,    lit('*')},
        {TokenKind::Slash,   lit('/')},
        {TokenKind::Percent, lit('%')},
        {TokenKind::Caret,   lit('^')},
        {TokenKind::Assign,  lit('=')},
        {TokenKind::Less,    lit('<')},
        {TokenKind::Greater, lit('>')},
        {TokenKind::And,     lit('&')},
        {TokenKind::Or,      lit('|')},
        {TokenKind::Not,     lit('!')},
        {TokenKind::Concat,  lit('@')},

        {TokenKind::LParen,     lit('(')},
        {TokenKind::RParen,     lit(')')},
        {TokenKind::LBrace,     lit('{')},
        {TokenKind::RBrace,     lit('}')},
        {TokenKind::Comma,      lit(',')},
        {TokenKind::Semicolon,  lit(';')},
        {TokenKind::Colon,      lit(':')},
        {TokenKind::Dot,        lit('.')},
        {TokenKind::Underscore, lit('_')},
    };
}

}

const regex::Nfa& lexer_nfa() {
    static const regex::Nfa nfa = build_nfa(build_rules());
    return nfa;
}

}