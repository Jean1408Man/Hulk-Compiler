#include "analyzer.h"

#include "../ast/assignments/desctructiveAssign.h"
#include "../ast/assignments/destructiveAssignMember.h"
#include "../ast/binOps/arithmeticBinOp.h"
#include "../ast/binOps/logicBinOp.h"
#include "../ast/binOps/stringBinOp.h"
#include "../ast/conditionals/ifStmt.h"
#include "../ast/domainFunctions/builtinCall.h"
#include "../ast/domainFunctions/print.h"
#include "../ast/functions/functionCall.h"
#include "../ast/functions/functionDecl.h"
#include "../ast/literales/boolean.h"
#include "../ast/literales/number.h"
#include "../ast/literales/string.h"
#include "../ast/loops/for.h"
#include "../ast/loops/while.h"
#include "../ast/others/baseCall.h"
#include "../ast/others/exprBlock.h"
#include "../ast/others/group.h"
#include "../ast/others/program.h"
#include "../ast/others/selfRef.h"
#include "../ast/protocols/protocolDecl.h"
#include "../ast/types/asExpr.h"
#include "../ast/types/isExpr.h"
#include "../ast/types/memberAccess.h"
#include "../ast/types/methodCall.h"
#include "../ast/types/newExpr.h"
#include "../ast/types/typeDecl.h"
#include "../ast/types/typeMemberAttribute.h"
#include "../ast/types/typeMemberMethod.h"
#include "../ast/unaryOps/arithmeticUnaryOp.h"
#include "../ast/unaryOps/logicUnaryOp.h"
#include "../ast/variables/letIn.h"
#include "../ast/variables/variableBinding.h"
#include "../ast/variables/variableReference.h"
#include "../inference/type_inferencer.h"
#include "../typecheck/type_checker.h"

namespace Hulk {

namespace {

constexpr const char* kRestrictedInferenceMessage =
    "Inferencia implicita no permitida en modo restringido. "
    "Use ': _', ': auto' o escriba un tipo concreto.";

class SemanticPolicyChecker : public ExprVisitor, public DeclVisitor {
public:
    SemanticPolicyChecker(hulk::common::DiagnosticEngine& engine,
                          bool restricted_inference)
        : engine_(engine),
          restricted_inference_(restricted_inference) {}

    void check(Program& program) {
        for (const auto& decl : program.GetDeclarations()) {
            if (decl) decl->accept(*this);
        }
        visit_expr(program.GetGlobalExpr());
    }

    [[nodiscard]] bool has_errors() const { return has_errors_; }

private:
    hulk::common::DiagnosticEngine& engine_;
    bool restricted_inference_ = false;
    bool has_errors_ = false;

    void report_restricted(const hulk::common::Span& span) {
        engine_.report_raw(hulk::common::DiagnosticLevel::Semantic,
                           hulk::common::Severity::Error,
                           span,
                           kRestrictedInferenceMessage);
        has_errors_ = true;
    }

    // Reports a restricted-inference error only when the position has neither
    // a concrete type annotation nor an explicit type-hole request (_ / auto).
    void require_annotation(bool has_concrete_annotation, bool is_type_hole,
                            const hulk::common::Span& span) {
        if (restricted_inference_ && !has_concrete_annotation && !is_type_hole)
            report_restricted(span);
    }

    void require_param_annotation(const Param& param,
                                  const hulk::common::Span& owner_span) {
        require_annotation(param.HasTypeAnnotation(), param.IsTypeHole(), owner_span);
    }

    void visit_expr(Expr* expr) {
        if (expr) expr->accept(*this);
    }

    void visit_args(const std::vector<std::unique_ptr<Expr>>& args) {
        for (const auto& arg : args) visit_expr(arg.get());
    }

    void visit_binary(BinOp& node) {
        visit_expr(node.GetLeft());
        visit_expr(node.GetRight());
    }

    void visit_unary(UnaryOp& node) {
        visit_expr(node.GetOperand());
    }

    void visit(Number&) override {}
    void visit(String&) override {}
    void visit(Boolean&) override {}
    void visit(VariableReference&) override {}
    void visit(SelfRef&) override {}

    void visit(ArithmeticBinOp& node) override { visit_binary(node); }
    void visit(LogicBinOp& node) override { visit_binary(node); }
    void visit(StringBinOp& node) override { visit_binary(node); }
    void visit(ArithmeticUnaryOp& node) override { visit_unary(node); }
    void visit(LogicUnaryOp& node) override { visit_unary(node); }

    void visit(VariableBinding& node) override {
        require_annotation(node.HasTypeAnnotation(), node.IsTypeHole(), node.span);
        visit_expr(node.GetInitializer());
    }

    void visit(LetIn& node) override {
        for (const auto& binding : node.GetBindings()) {
            if (binding) binding->accept(*this);
        }
        visit_expr(node.GetBody());
    }

    void visit(DestructiveAssign& node) override {
        visit_expr(node.GetValue());
    }

    void visit(DestructiveAssignMember& node) override {
        visit_expr(node.GetObject());
        visit_expr(node.GetValue());
    }

    void visit(IfStmt& node) override {
        visit_expr(node.GetCondition());
        visit_expr(node.GetThenBranch());
        for (const auto& branch : node.GetElifBranches()) {
            visit_expr(branch.condition.get());
            visit_expr(branch.body.get());
        }
        visit_expr(node.GetElseBranch());
    }

    void visit(WhileStmt& node) override {
        visit_expr(node.GetCondition());
        visit_expr(node.GetBody());
    }

    void visit(For& node) override {
        visit_expr(node.GetIterable());
        visit_expr(node.GetBody());
    }

    void visit(FunctionCall& node) override {
        visit_args(node.GetArgs());
    }

    void visit(Print& node) override { visit_expr(node.GetExpr()); }
    void visit(BuiltinCall& node) override {
        visit_args(node.GetArgs());
    }

    void visit(ExprBlock& node) override {
        for (const auto& expr : node.GetExprs()) visit_expr(expr.get());
    }

    void visit(Group& node) override { visit_expr(node.GetExpr()); }
    void visit(BaseCall& node) override { visit_args(node.GetArgs()); }

    void visit(NewExpr& node) override { visit_args(node.GetArgs()); }

    void visit(MemberAccess& node) override { visit_expr(node.GetObject()); }

    void visit(MethodCall& node) override {
        visit_expr(node.GetObject());
        visit_args(node.GetArgs());
    }

    void visit(IsExpr& node) override { visit_expr(node.GetExpr()); }
    void visit(AsExpr& node) override { visit_expr(node.GetExpr()); }

    void visit(FunctionDecl& node) override {
        for (const auto& param : node.GetParams()) require_param_annotation(param, node.span);
        require_annotation(node.HasReturnTypeAnnotation(), node.IsReturnTypeHole(), node.span);
        visit_expr(node.GetBody());
    }

    void visit(TypeDecl& node) override {
        for (const auto& param : node.GetCtorParams()) require_param_annotation(param, node.span);
        for (const auto& arg : node.GetParentArgs()) visit_expr(arg.get());
        for (const auto& member : node.GetMembers()) {
            if (member.node) member.node->accept(*this);
        }
    }

    void visit(TypeMemberAttribute& node) override {
        require_annotation(node.HasTypeAnnotation(), node.IsTypeHole(), node.span);
        visit_expr(node.GetInitializer());
    }

    void visit(TypeMemberMethod& node) override {
        for (const auto& param : node.GetParams()) require_param_annotation(param, node.span);
        require_annotation(node.HasReturnTypeAnnotation(), node.IsReturnTypeHole(), node.span);
        visit_expr(node.GetBody());
    }

    void visit(ProtocolDecl& node) override {
        (void)node;
    }
};

}

SemanticAnalyzer::SemanticAnalyzer(hulk::common::DiagnosticEngine& engine,
                                   SemanticOptions options)
    : engine_(engine),
      options_(options)
{}

SemanticAnalyzer::~SemanticAnalyzer() = default;

bool SemanticAnalyzer::analyze(Program& program) {
    SemanticPolicyChecker policy_checker(engine_, options_.restricted_inference);
    policy_checker.check(program);
    if (policy_checker.has_errors()) has_errors_ = true;

    resolver_ = std::make_unique<SymbolResolver>(tables_, engine_);
    resolver_->run(program);
    // Continuamos a pesar de errores en el resolver para capturar errores de tipos
    // a menos que el resolver haya fallado catastróficamente (sin tablas consistentes)

    inferencer_ = std::make_unique<TypeInferencer>(tables_, resolver_->resolution_map(), engine_);
    inferencer_->infer(program);

    type_checker_ = std::make_unique<TypeChecker>(
        tables_,
        inferencer_->type_map(),
        resolver_->resolution_map(),
        inferencer_->param_types(),
        inferencer_->binding_types(),
        inferencer_->synthetic_types(),
        engine_
    );
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
