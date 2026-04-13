#pragma once
namespace hulk::lexer {

enum class TokenKind {
    // especiales
    EndOfFile,
    Error,

    // identificadores y literales
    Identifier,
    Number,
    String,

    // booleanos
    True,
    False,

    // builtins y constantes globales
    Print,
    Sqrt,
    Sin,
    Cos,
    Exp,
    Log,
    Rand,
    Pi,
    E,

    // keywords del lenguaje base
    Let,
    In,
    If,
    Elif,
    Else,
    While,
    For,
    Function,
    Type,
    Inherits,
    New,
    Is,
    As,

    // operadores
    Plus,              // +
    Minus,             // -
    Star,              // *
    Slash,             // /
    Percent,           // %
    Caret,             // ^
    Assign,            // =
    DestructiveAssign, // :=
    EqualEqual,        // ==
    NotEqual,          // !=
    Less,              // <
    LessEqual,         // <=
    Greater,           // >
    GreaterEqual,      // >=
    And,               // &
    Or,                // |
    Not,               // !
    DoubleConcat,   // @@
    Concat,            // @
    FatArrow,          // =>

    // delimitadores y puntuación
    LParen,            // (
    RParen,            // )
    LBrace,            // {
    RBrace,            // }
    Comma,             // ,
    Semicolon,         // ;
    Colon,             // :
    Dot,               // .
};

} // namespace hulk::lexer