#include "lexer.hpp"
#include <string>
#include <string_view>
#include "keywords.hpp"
#include "lexer_rules.hpp"
#include "regex/nfa_simulator.hpp"

namespace hulk::lexer {
namespace {

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

}

Lexer::Lexer(std::string source, hulk::common::DiagnosticEngine& engine)
    : cursor_(std::move(source)), engine_(engine) {}

Token Lexer::next_token() {
    skip_whitespace_and_comments();

    const hulk::common::Position start = cursor_.position();

    if (cursor_.eof()) {
        return make_token(TokenKind::EndOfFile, "", start, start);
    }

    if (cursor_.peek() == '"') {
        return scan_string();
    }

    // simulación del AFN (Thompson + conjunto de estados)
    const regex::MatchResult m = regex::longest_match(
        lexer_nfa(),
        [this](std::size_t i) -> int {
            return static_cast<unsigned char>(cursor_.peek(i));
        });

    if (!m.matched) {
        // Ningún token empieza con este carácter: carácter inválido.
        const char bad = cursor_.advance();
        const hulk::common::Position end = cursor_.position();
        return error_token("INVALID_CHAR", start, end, std::string(1, bad));
    }

    std::string lexeme;
    lexeme.reserve(m.length);
    for (std::size_t i = 0; i < m.length; ++i) {
        lexeme += cursor_.advance();
    }
    const hulk::common::Position end = cursor_.position();

    TokenKind kind = m.kind;
    if (kind == TokenKind::Identifier) {
        const auto it = kKeywords.find(std::string_view(lexeme));
        if (it != kKeywords.end()) {
            kind = it->second;
        }
    }

    return make_token(kind, lexeme, start, end);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token token = next_token();
        tokens.push_back(token);

        if (token.kind == TokenKind::EndOfFile) {
            break;
        }
    }

    return tokens;
}

void Lexer::skip_whitespace_and_comments() {
    while (!cursor_.eof()) {
        const char c = cursor_.peek();

        if (is_whitespace(c)) {
            cursor_.advance();
            continue;
        }

        if (cursor_.peek() == '/' && cursor_.peek(1) == '/') {
            while (!cursor_.eof() && cursor_.peek() != '\n') {
                cursor_.advance();
            }
            continue;
        }

        break;
    }
}

Token Lexer::scan_string() {
    const hulk::common::Position start = cursor_.position();
    std::string lexeme;

    // Consume comilla inicial
    lexeme += cursor_.advance();

    while (!cursor_.eof()) {
        const char c = cursor_.peek();

        if (c == '\\') {
            lexeme += cursor_.advance();

            if (cursor_.eof()) {
                const hulk::common::Position end = cursor_.position();
                return error_token("INCOMPLETE_ESCAPE", start, end, lexeme);
            }

            lexeme += cursor_.advance();
            continue;
        }

        if (c == '"') {
            lexeme += cursor_.advance();
            const hulk::common::Position end = cursor_.position();
            return make_token(TokenKind::String, lexeme, start, end);
        }

        if (c == '\n') {
            const hulk::common::Position end = cursor_.position();
            return error_token("UNTERMINATED_STRING_EOL", start, end, lexeme);
        }

        lexeme += cursor_.advance();
    }

    const hulk::common::Position end = cursor_.position();
    return error_token("UNTERMINATED_STRING_EOF", start, end, lexeme);
}

Token Lexer::make_token(TokenKind kind,
                        const std::string& lexeme,
                        hulk::common::Position start,
                        hulk::common::Position end) {
    return Token {
        .kind = kind,
        .lexeme = lexeme,
        .span = hulk::common::Span {
            .start = start,
            .end = end,
        },
    };
}

Token Lexer::error_token(const std::string& error_id,
                         hulk::common::Position start,
                         hulk::common::Position end,
                         const std::string& lexeme) {
    const hulk::common::Span span { .start = start, .end = end };

    engine_.report(error_id,
                   hulk::common::DiagnosticLevel::Lexical,
                   hulk::common::Severity::Error,
                   span,
                   lexeme);

    return Token {
        .kind   = TokenKind::Error,
        .lexeme = lexeme,
        .span   = span,
    };
}

}