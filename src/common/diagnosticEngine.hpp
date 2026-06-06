#pragma once
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "diagnostic.hpp"
#include "diagnosticRepository.hpp"

namespace hulk::common {

class DiagnosticEngine {
private:
    const DiagnosticRepository& repo_;
    std::vector<Diagnostic> diagnostics_;

public:
    explicit DiagnosticEngine(const DiagnosticRepository& repo)
        : repo_(repo) {}

    template <typename... Args>
    void report(const std::string& error_id,
                DiagnosticLevel level,
                Severity severity,
                Span span,
                Args&&... args) {
        const std::string tmpl = repo_.get_template(error_id);
        std::string final_msg;
        
        if constexpr (sizeof...(args) > 0) {
            if (tmpl.find("{}") != std::string::npos) {
                final_msg = std::vformat(tmpl, std::make_format_args(args...));
            } else {
                final_msg = tmpl;
            }
        } else {
            final_msg = tmpl;
        }
        diagnostics_.push_back(Diagnostic {
            .level    = level,
            .severity = severity,
            .message  = std::move(final_msg),
            .span     = span,
        });
    }

    void report_raw(DiagnosticLevel level,
                    Severity severity,
                    Span span,
                    std::string message) {
        diagnostics_.push_back(Diagnostic {
            .level    = level,
            .severity = severity,
            .message  = std::move(message),
            .span     = span,
        });
    }

    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const {
        return diagnostics_;
    }

    [[nodiscard]] bool has_errors() const {
        for (const auto& d : diagnostics_) {
            if (d.severity == Severity::Error) return true;
        }
        return false;
    }

    [[nodiscard]] bool has_blocking_errors() const {
        return has_errors();
    }

    void clear() { diagnostics_.clear(); }

    static const char* level_string(DiagnosticLevel level) {
        switch (level) {
            case DiagnosticLevel::Lexical:   return "LEXICAL";
            case DiagnosticLevel::Syntactic: return "SYNTACTIC";
            case DiagnosticLevel::Semantic:  return "SEMANTIC";
        }
        return "SEMANTIC";
    }

    void print_all() const {
        for (const auto& d : diagnostics_) {
            const int line = (d.span.start.line > 0) ? d.span.start.line : 0;
            const int col  = (d.span.start.column > 0) ? d.span.start.column : 0;
            std::cerr
                << "(" << line << "," << col << ") "
                << level_string(d.level)
                << ": "
                << d.message
                << "\n";
        }
    }

    void print(Severity filter) const {
        for (const auto& d : diagnostics_) {
            if (d.severity != filter) continue;
            const int line = (d.span.start.line > 0) ? d.span.start.line : 0;
            const int col  = (d.span.start.column > 0) ? d.span.start.column : 0;
            std::cerr
                << "(" << line << "," << col << ") "
                << level_string(d.level)
                << ": "
                << d.message
                << "\n";
        }
    }

    // (Lexical > Syntactic > Semantic).
    [[nodiscard]] std::optional<DiagnosticLevel> most_fundamental_error_level() const {
        bool has_lex = false, has_syn = false, has_sem = false;
        for (const auto& d : diagnostics_) {
            if (d.severity != Severity::Error) continue;
            switch (d.level) {
                case DiagnosticLevel::Lexical:   has_lex = true; break;
                case DiagnosticLevel::Syntactic: has_syn = true; break;
                case DiagnosticLevel::Semantic:  has_sem = true; break;
            }
        }
        if (has_lex) return DiagnosticLevel::Lexical;
        if (has_syn) return DiagnosticLevel::Syntactic;
        if (has_sem) return DiagnosticLevel::Semantic;
        return std::nullopt;
    }

    [[nodiscard]] int exit_code_for_contract() const {
        const auto level = most_fundamental_error_level();
        if (!level) return 0;
        switch (*level) {
            case DiagnosticLevel::Lexical:   return 1;
            case DiagnosticLevel::Syntactic: return 2;
            case DiagnosticLevel::Semantic:  return 3;
        }
        return 3;
    }
};

} 
