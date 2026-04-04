#pragma once
#include <string>
#include "token_kind.hpp"
#include "../common/span.hpp"

namespace hulk::lexer {

struct Token {
    TokenKind kind = TokenKind::Error;
    std::string lexeme;
    hulk::common::Span span {};
};

}