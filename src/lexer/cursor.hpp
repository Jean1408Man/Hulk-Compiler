#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include "../common/position.hpp"

namespace hulk::lexer {

class Cursor {
public:
    explicit Cursor(std::string source)
        : source_(std::move(source)) {}

    [[nodiscard]] bool eof() const {
        return pos_.index >= source_.size();
    }

    [[nodiscard]] char peek(std::size_t offset = 0) const {
        const std::size_t i = pos_.index + offset;
        if (i >= source_.size()) {
            return '\0';
        }
        return source_[i];
    }

    char advance() {
        if (eof()) {
            return '\0';
        }

        const char c = source_[pos_.index++];
        if (c == '\n') {
            ++pos_.line;
            pos_.column = 1;
        } else {
            ++pos_.column;
        }
        return c;
    }

    bool match(char expected) {
        if (peek() != expected) {
            return false;
        }
        advance();
        return true;
    }

    bool match(std::string_view expected) {
        for (std::size_t i = 0; i < expected.size(); ++i) {
            if (peek(i) != expected[i]) {
                return false;
            }
        }
        for (std::size_t i = 0; i < expected.size(); ++i) {
            advance();
        }
        return true;
    }

    [[nodiscard]] hulk::common::Position position() const {
        return pos_;
    }

private:
    std::string source_;
    hulk::common::Position pos_ {};
};

}