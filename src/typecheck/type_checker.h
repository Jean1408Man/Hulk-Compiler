#ifndef HULK_TYPE_CHECKER_H
#define HULK_TYPE_CHECKER_H

#include "../eval/visitor.h"
#include "../semantic/semantic_tables.h"
#include "../binding/symbol_resolver.h"
#include "../inference/hulk_type.h"
#include "../common/diagnosticEngine.hpp"
#include <unordered_map>

namespace Hulk {

    class TypeChecker : public ExprVisitor {
    public:
        TypeChecker(const SemanticTables& tables,
                    const std::unordered_map<Expr*, HulkType>& type_map,
                    const std::unordered_map<Expr*, ResolutionResult>& resolution_map,
                    const std::unordered_map<const Param*, HulkType>& param_types,
                    const std::unordered_map<VariableBinding*, HulkType>& binding_types,
                    const std::unordered_map<const SyntheticSymbol*, HulkType>& synthetic_types,
                    hulk::common::DiagnosticEngine& engine);

        void check(Program& program);

        // Visitor methods
        void visit(Number& node) override;
        void visit(String& node) override;
        void visit(Boolean& node) override;

        void visit(ArithmeticBinOp& node) override;
        void visit(LogicBinOp& node) override;
        void visit(StringBinOp& node) override;
        void visit(ArithmeticUnaryOp& node) override;
        void visit(LogicUnaryOp& node) override;

        void visit(VariableReference& node) override;
        void visit(VariableBinding& node) override;
        void visit(LetIn& node) override;

        void visit(DestructiveAssign& node) override;
        void visit(DestructiveAssignMember& node) override;

        void visit(IfStmt& node) override;
        void visit(WhileStmt& node) override;
        void visit(For& node) override;

        void visit(FunctionCall& node) override;

        void visit(Print& node) override;
        void visit(BuiltinCall& node) override;

        void visit(ExprBlock& node) override;
        void visit(Group& node) override;
        void visit(SelfRef& node) override;
        void visit(BaseCall& node) override;

        void visit(NewExpr& node) override;
        void visit(MemberAccess& node) override;
        void visit(MethodCall& node) override;
        void visit(IsExpr& node) override;
        void visit(AsExpr& node) override;

    private:
        const SemanticTables& tables_;
        const std::unordered_map<Expr*, HulkType>& type_map_;
        const std::unordered_map<Expr*, ResolutionResult>& resolution_map_;
        const std::unordered_map<const Param*, HulkType>&           param_types_;
        const std::unordered_map<VariableBinding*, HulkType>&       binding_types_;
        const std::unordered_map<const SyntheticSymbol*, HulkType>& synthetic_types_;
        hulk::common::DiagnosticEngine& engine_;

        // Helpers
        HulkType get_type(Expr* node);
        void report_error(const hulk::common::Span& span, const std::string& msg);
        void check_conforms(Expr* node, const HulkType& expected, const std::string& context);
        bool type_conforms_with_inference(const HulkType& found, const HulkType& expected) const;
        bool type_conforms_to_protocol_inferred(const std::string& type_name,
                                                const std::string& protocol_name,
                                                int depth = 0) const;
        bool method_satisfies_protocol_inferred(const SemanticMethodInfo& actual,
                                                const SemanticProtocolMethodInfo& required) const;
        HulkType resolve_method_param_type(const SemanticMethodInfo& method, std::size_t index) const;
        HulkType resolve_method_return_type(const SemanticMethodInfo& method) const;
        
        HulkType from_string_type(const std::string& name) const;
    };

}

#endif
