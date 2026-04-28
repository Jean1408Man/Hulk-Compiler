#include "analyzer.h"
#include "../ast/others/program.h"

namespace Hulk {

SemanticAnalyzer::SemanticAnalyzer(hulk::common::DiagnosticEngine& engine)
    : engine_(engine)
{}

bool SemanticAnalyzer::analyze(Program& program) {
    resolver_ = std::make_unique<SymbolResolver>(tables_, engine_);
    const bool ok = resolver_->run(program);
    has_errors_ = !ok;
    return ok;
}

const std::unordered_map<Expr*, ResolutionResult>& SemanticAnalyzer::resolution_map() const {
    // Si analyze() nunca fue llamado, resolver_ es null — devolver un mapa vacío estático.
    static const std::unordered_map<Expr*, ResolutionResult> empty;
    if (!resolver_) return empty;
    return resolver_->resolution_map();
}

}
