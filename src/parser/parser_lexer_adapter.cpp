#include "parser.hpp"

#include <cmath>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "../common/span.hpp"
#include "../lexer/token_kind.hpp"
#include "parser_driver.hpp"
#include "parser_tables.hpp"

namespace {

using SymbolId = hulk::parser::parser_tables::symbol_id;

hulk::parser::Symbol make_symbol(int symbol, const hulk::common::Span& span) {
    return hulk::parser::Symbol { symbol, hulk::parser::ParserValue {}, span };
}

hulk::parser::Symbol make_symbol(int symbol,
                                 std::string value,
                                 const hulk::common::Span& span) {
    return hulk::parser::Symbol { symbol, std::move(value), span };
}

hulk::parser::Symbol make_symbol(int symbol,
                                 double value,
                                 const hulk::common::Span& span) {
    return hulk::parser::Symbol { symbol, value, span };
}

std::optional<double> parse_number_lexeme(const std::string& lexeme,
                                          hulk::parser::ParserDriver& driver,
                                          const hulk::common::Span& span) {
    try {
        std::size_t consumed = 0;
        const double value = std::stod(lexeme, &consumed);
        if (consumed != lexeme.size() || !std::isfinite(value)) {
            driver.report_syntax_error("Literal numerico invalido", span);
            return std::nullopt;
        }
        return value;
    } catch (const std::out_of_range&) {
        driver.report_syntax_error("Literal numerico fuera de rango", span);
        return std::nullopt;
    } catch (const std::exception&) {
        driver.report_syntax_error("Literal numerico invalido", span);
        return std::nullopt;
    }
}

std::optional<std::string> decode_string_lexeme(const std::string& lexeme,
                                                hulk::parser::ParserDriver& driver,
                                                const hulk::common::Span& span) {
    if (lexeme.size() < 2 || lexeme.front() != '"' || lexeme.back() != '"') {
        driver.report_syntax_error("Literal de string invalido", span);
        return std::nullopt;
    }

    std::string decoded;
    for (std::size_t i = 1; i + 1 < lexeme.size(); ++i) {
        const char c = lexeme[i];
        if (c != '\\') {
            decoded += c;
            continue;
        }

        if (i + 2 >= lexeme.size()) {
            driver.report_syntax_error("Escape de string incompleto", span);
            return std::nullopt;
        }

        const char escaped = lexeme[++i];
        switch (escaped) {
            case 'n': decoded += '\n'; break;
            case 'r': decoded += '\r'; break;
            case 't': decoded += '\t'; break;
            case '"': decoded += '"'; break;
            case '\\': decoded += '\\'; break;
            default: {
                std::string message = "Escape de string no soportado: \\";
                message += escaped;
                driver.report_syntax_error(message, span);
                return std::nullopt;
            }
        }
    }

    return decoded;
}

} 

namespace hulk::parser {

Parser::symbol_type yylex(ParserDriver& driver) {
    using TK = hulk::lexer::TokenKind;

    const auto token = driver.next_token();

    switch (token.kind) {
        case TK::EndOfFile: return make_symbol(SymbolId::END, token.span);
        case TK::Error: return make_symbol(SymbolId::ERROR_TOKEN, token.lexeme, token.span);

        case TK::Identifier: return make_symbol(SymbolId::IDENTIFIER, token.lexeme, token.span);
        case TK::Number: {
            const auto value = parse_number_lexeme(token.lexeme, driver, token.span);
            if (!value.has_value()) return Symbol::invalid(token.span);
            return make_symbol(SymbolId::NUMBER_LITERAL, *value, token.span);
        }
        case TK::String: {
            const auto value = decode_string_lexeme(token.lexeme, driver, token.span);
            if (!value.has_value()) return Symbol::invalid(token.span);
            return make_symbol(SymbolId::STRING_LITERAL, *value, token.span);
        }

        case TK::True: return make_symbol(SymbolId::TRUE, token.span);
        case TK::False: return make_symbol(SymbolId::FALSE, token.span);

        case TK::Print: return make_symbol(SymbolId::PRINT, token.span);
        case TK::Sqrt: return make_symbol(SymbolId::SQRT, token.span);
        case TK::Sin: return make_symbol(SymbolId::SIN, token.span);
        case TK::Cos: return make_symbol(SymbolId::COS, token.span);
        case TK::Exp: return make_symbol(SymbolId::EXP, token.span);
        case TK::Log: return make_symbol(SymbolId::LOG, token.span);
        case TK::Rand: return make_symbol(SymbolId::RAND, token.span);
        case TK::Pi: return make_symbol(SymbolId::PI_CONST, token.span);
        case TK::E: return make_symbol(SymbolId::E_CONST, token.span);

        case TK::Let: return make_symbol(SymbolId::LET, token.span);
        case TK::In: return make_symbol(SymbolId::IN, token.span);
        case TK::If: return make_symbol(SymbolId::IF, token.span);
        case TK::Elif: return make_symbol(SymbolId::ELIF, token.span);
        case TK::Else: return make_symbol(SymbolId::ELSE, token.span);
        case TK::While: return make_symbol(SymbolId::WHILE, token.span);
        case TK::For: return make_symbol(SymbolId::FOR, token.span);
        case TK::Function: return make_symbol(SymbolId::FUNCTION, token.span);
        case TK::Type: return make_symbol(SymbolId::TYPE, token.span);
        case TK::Extends: return make_symbol(SymbolId::EXTENDS, token.span);
        case TK::Inherits: return make_symbol(SymbolId::INHERITS, token.span);
        case TK::New: return make_symbol(SymbolId::NEW, token.span);
        case TK::Is: return make_symbol(SymbolId::IS, token.span);
        case TK::As: return make_symbol(SymbolId::AS, token.span);
        case TK::Self: return make_symbol(SymbolId::SELF, token.span);
        case TK::Base: return make_symbol(SymbolId::BASE, token.span);

        case TK::Plus: return make_symbol(SymbolId::PLUS, token.span);
        case TK::Minus: return make_symbol(SymbolId::MINUS, token.span);
        case TK::Star: return make_symbol(SymbolId::STAR, token.span);
        case TK::Slash: return make_symbol(SymbolId::SLASH, token.span);
        case TK::Percent: return make_symbol(SymbolId::PERCENT, token.span);
        case TK::Caret: return make_symbol(SymbolId::CARET, token.span);
        case TK::Assign: return make_symbol(SymbolId::ASSIGN, token.span);
        case TK::DestructiveAssign: return make_symbol(SymbolId::DESTRUCTIVE_ASSIGN, token.span);
        case TK::EqualEqual: return make_symbol(SymbolId::EQUAL_EQUAL, token.span);
        case TK::NotEqual: return make_symbol(SymbolId::NOT_EQUAL, token.span);
        case TK::Less: return make_symbol(SymbolId::LESS, token.span);
        case TK::LessEqual: return make_symbol(SymbolId::LESS_EQUAL, token.span);
        case TK::Greater: return make_symbol(SymbolId::GREATER, token.span);
        case TK::GreaterEqual: return make_symbol(SymbolId::GREATER_EQUAL, token.span);
        case TK::And: return make_symbol(SymbolId::AND, token.span);
        case TK::Or: return make_symbol(SymbolId::OR, token.span);
        case TK::Not: return make_symbol(SymbolId::NOT, token.span);
        case TK::Concat: return make_symbol(SymbolId::CONCAT, token.span);
        case TK::DoubleConcat: return make_symbol(SymbolId::DOUBLECONCAT, token.span);
        case TK::FatArrow: return make_symbol(SymbolId::FATARROW, token.span);

        case TK::LParen: return make_symbol(SymbolId::LPAREN, token.span);
        case TK::RParen: return make_symbol(SymbolId::RPAREN, token.span);
        case TK::LBrace: return make_symbol(SymbolId::LBRACE, token.span);
        case TK::RBrace: return make_symbol(SymbolId::RBRACE, token.span);
        case TK::Comma: return make_symbol(SymbolId::COMMA, token.span);
        case TK::Semicolon: return make_symbol(SymbolId::SEMICOLON, token.span);
        case TK::Colon: return make_symbol(SymbolId::COLON, token.span);
        case TK::Dot: return make_symbol(SymbolId::DOT, token.span);
        case TK::Protocol: return make_symbol(SymbolId::PROTOCOL, token.span);

        case TK::Underscore: return make_symbol(SymbolId::UNDERSCORE, token.span);
        case TK::Auto:       return make_symbol(SymbolId::AUTO, token.span);
    }

    return make_symbol(SymbolId::ERROR_TOKEN, "<unknown>", token.span);
}

}
