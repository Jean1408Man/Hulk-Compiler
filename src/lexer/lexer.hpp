#pragma once

#include <string>
#include <vector>
#include "cursor.hpp"
#include "token.hpp"
#include "../common/diagnostic.hpp"

namespace hulk::lexer {

class Lexer {
public:
    explicit Lexer(std::string source);

    Token next_token();
    std::vector<Token> tokenize();

    [[nodiscard]] const std::vector<hulk::common::Diagnostic>& diagnostics() const {
        return diagnostics_;
    }

private:
    Cursor cursor_;
    std::vector<hulk::common::Diagnostic> diagnostics_;

    void skip_whitespace_and_comments();

    Token scan_identifier_or_keyword();
    Token scan_number();
    Token scan_string();
    Token scan_operator_or_delimiter();

    Token make_token(TokenKind kind, const std::string& lexeme,
                     hulk::common::Position start,
                     hulk::common::Position end);

    Token error_token(const std::string& message,
                      hulk::common::Position start,
                      hulk::common::Position end,
                      const std::string& lexeme);
};

}