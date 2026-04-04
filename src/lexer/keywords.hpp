#pragma once

#include <string_view>
#include <unordered_map>
#include "token_kind.hpp"

namespace hulk::lexer {

inline const std::unordered_map<std::string_view, TokenKind> kKeywords = {
    // booleanos
    {"true", TokenKind::True},
    {"false", TokenKind::False},

    // builtins y constantes
    {"print", TokenKind::Print},
    {"sqrt", TokenKind::Sqrt},
    {"sin", TokenKind::Sin},
    {"cos", TokenKind::Cos},
    {"exp", TokenKind::Exp},
    {"log", TokenKind::Log},
    {"rand", TokenKind::Rand},
    {"PI", TokenKind::Pi},
    {"E", TokenKind::E},

    // lenguaje base
    {"let", TokenKind::Let},
    {"in", TokenKind::In},
    {"if", TokenKind::If},
    {"elif", TokenKind::Elif},
    {"else", TokenKind::Else},
    {"while", TokenKind::While},
    {"for", TokenKind::For},
    {"function", TokenKind::Function},
    {"type", TokenKind::Type},
    {"inherits", TokenKind::Inherits},
    {"new", TokenKind::New},
    {"is", TokenKind::Is},
    {"as", TokenKind::As},
};

}