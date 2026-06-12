#pragma once

#include <string>

#include "parser_driver.hpp"
#include "symbol.hpp"

namespace hulk::parser {

class Parser {
public:
    using symbol_type = Symbol;

    explicit Parser(ParserDriver& driver);

    int parse();

private:
    ParserDriver& driver_;

    std::string syntax_error_message(int state, const Symbol& lookahead) const;
    void report_syntax_error(int state, const Symbol& lookahead) const;
};

Parser::symbol_type yylex(ParserDriver& driver);

} // namespace hulk::parser
