#pragma once
#include <string>
#include "span.hpp"

namespace hulk::common {

enum class DiagnosticLevel {
    Error,
    Warning,
};

struct Diagnostic {
    DiagnosticLevel level = DiagnosticLevel::Error;
    std::string message;
    Span span {};
};

}