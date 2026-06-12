#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>

#include "lexer/lexer_rules.hpp"
#include "lexer/regex/nfa_simulator.hpp"
#include "lexer/token_kind.hpp"

using hulk::lexer::lexer_nfa;
using hulk::lexer::TokenKind;
using hulk::lexer::regex::longest_match;
using hulk::lexer::regex::MatchResult;

namespace {

MatchResult match(std::string_view input) {
    return longest_match(lexer_nfa(), [input](std::size_t i) -> int {
        if (i >= input.size()) {
            return -1;
        }
        return static_cast<unsigned char>(input[i]);
    });
}

void expect(std::string_view input, std::size_t length, TokenKind kind) {
    const MatchResult m = match(input);
    assert(m.matched);
    assert(m.length == length);
    assert(m.kind == kind);
}

void numbers() {
    expect("3.14", 4, TokenKind::Number);
    expect("42", 2, TokenKind::Number);
    // '5.' sin dígito tras el punto ⇒ sólo "5" (el '.' queda fuera).
    expect("5.", 1, TokenKind::Number);
    expect("5.x", 1, TokenKind::Number);
    expect("0", 1, TokenKind::Number);
}

void identifiers() {
    expect("abc", 3, TokenKind::Identifier);
    expect("a_1b", 4, TokenKind::Identifier);
    // El AFN sólo reconoce Identifier; la keyword se resuelve fuera (tabla hash).
    expect("let", 3, TokenKind::Identifier);
    expect("x+y", 1, TokenKind::Identifier);
}

void maximal_munch_operators() {
    expect(":=", 2, TokenKind::DestructiveAssign);
    expect(":", 1, TokenKind::Colon);
    expect("==", 2, TokenKind::EqualEqual);
    expect("=>", 2, TokenKind::FatArrow);
    expect("=", 1, TokenKind::Assign);
    expect("!=", 2, TokenKind::NotEqual);
    expect("!", 1, TokenKind::Not);
    expect("<=", 2, TokenKind::LessEqual);
    expect(">=", 2, TokenKind::GreaterEqual);
    expect("<", 1, TokenKind::Less);
    expect(">", 1, TokenKind::Greater);
    expect("@@", 2, TokenKind::DoubleConcat);
    expect("@", 1, TokenKind::Concat);
    // '@@@' ⇒ '@@' (longest) + resto.
    expect("@@@", 2, TokenKind::DoubleConcat);
}

void single_chars() {
    expect("+", 1, TokenKind::Plus);
    expect("-", 1, TokenKind::Minus);
    expect("*", 1, TokenKind::Star);
    expect("/", 1, TokenKind::Slash);
    expect("%", 1, TokenKind::Percent);
    expect("^", 1, TokenKind::Caret);
    expect("&", 1, TokenKind::And);
    expect("|", 1, TokenKind::Or);
    expect("(", 1, TokenKind::LParen);
    expect(")", 1, TokenKind::RParen);
    expect("{", 1, TokenKind::LBrace);
    expect("}", 1, TokenKind::RBrace);
    expect(",", 1, TokenKind::Comma);
    expect(";", 1, TokenKind::Semicolon);
    expect(".", 1, TokenKind::Dot);
    expect("_", 1, TokenKind::Underscore);
    // '_foo' ⇒ Underscore (len 1); el identificador empieza en 'foo'.
    expect("_foo", 1, TokenKind::Underscore);
}

void no_match() {
    // Caracteres que no inician ninguna regla ⇒ matched=false (camino INVALID_CHAR).
    assert(!match("#").matched);
    assert(!match("$").matched);
    assert(!match("?").matched);
    // Cadena vacía ⇒ sin match.
    assert(!match("").matched);
}

} 

int main() {
    numbers();
    identifiers();
    maximal_munch_operators();
    single_chars();
    no_match();
    return 0;
}
