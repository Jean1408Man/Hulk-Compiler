#pragma once

#include <string>
#include <variant>

#include "../common/span.hpp"
#include "parser_semantic_types.hpp"

namespace hulk::parser {

using ParserValue = std::variant<
    std::monostate,
    double,
    std::string,
    ExprPtr,
    ExprList,
    DeclPtr,
    DeclList,
    ProgramPtr,
    BindingPtr,
    BindingList,
    ParamList,
    Hulk::Param,
    ProtocolMethodList,
    Hulk::ProtocolMethodSig,
    ElifList,
    Hulk::TypeMember,
    TypeMemberList,
    InheritsInfo,
    LValueTarget,
    TopLevelItems>;

struct Symbol {
    int sym = -1;
    ParserValue value {};
    hulk::common::Span span {};

    static Symbol invalid(const hulk::common::Span& token_span) {
        return Symbol { -1, ParserValue {}, token_span };
    }
};

struct StackEntry {
    int state = 0;
    ParserValue value {};
    hulk::common::Span span {};
};

}
