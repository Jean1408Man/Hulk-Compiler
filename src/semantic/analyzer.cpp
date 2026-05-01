#include "analyzer.h"
#include "../ast/others/program.h"
#include "../inference/type_inferencer.h"
#include "../typecheck/type_checker.h"

namespace Hulk {

SemanticAnalyzer::SemanticAnalyzer(hulk::common::DiagnosticEngine& engine)
    : engine_(engine)
{}

SemanticAnalyzer::~SemanticAnalyzer() = default;

bool SemanticAnalyzer::analyze(Program& program) {
    resolver_ = std::make_unique<SymbolResolver>(tables_, engine_);
    const bool ok = resolver_->run(program);
    has_errors_ = !ok;

    inferencer_ = std::make_unique<TypeInferencer>(tables_, resolver_->resolution_map(), engine_);
    inferencer_->infer(program);
    
    type_checker_ = std::make_unique<TypeChecker>(tables_, inferencer_->type_map(), resolver_->resolution_map(), engine_);
    type_checker_->check(program);

    if (engine_.has_errors()) {
        has_errors_ = true;
    }

    return !has_errors_;
}

const std::unordered_map<Expr*, ResolutionResult>& SemanticAnalyzer::resolution_map() const {
    // Si analyze() nunca fue llamado, resolver_ es null — devolver un mapa vacío estático.
    static const std::unordered_map<Expr*, ResolutionResult> empty;
    if (!resolver_) return empty;
    return resolver_->resolution_map();
}

const std::unordered_map<Expr*, HulkType>& SemanticAnalyzer::type_map() const {
    static const std::unordered_map<Expr*, HulkType> empty;
    if (!inferencer_) return empty;
    return inferencer_->type_map();
}

}
