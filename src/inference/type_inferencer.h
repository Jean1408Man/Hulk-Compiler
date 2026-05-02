#ifndef HULK_TYPE_INFERENCER_H
#define HULK_TYPE_INFERENCER_H

#include "../eval/visitor.h"
#include "../semantic/semantic_tables.h"
#include "../binding/symbol_resolver.h"
#include "../common/diagnosticEngine.hpp"
#include "hulk_type.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace Hulk {

    class Expr; // Forward declaration

    class TypeInferencer : public ExprVisitor {
    public:
        TypeInferencer(const SemanticTables& tables,
                       const std::unordered_map<Expr*, ResolutionResult>& resolution_map,
                       hulk::common::DiagnosticEngine& engine);

        // Entrada principal
        void infer(Program& program);

        // Mapa resultante de inferencia
        const std::unordered_map<Expr*, HulkType>& type_map() const { return type_map_; }
        
        const std::unordered_map<const Param*, HulkType>& param_types() const { return param_types_; }
        const std::unordered_map<VariableBinding*, HulkType>& binding_types() const { return binding_types_; }
        const std::unordered_map<const SyntheticSymbol*, HulkType>& synthetic_types() const { return synthetic_types_; }

        // Métodos del ExprVisitor
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
        void visit(Lambda& node) override;

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

        void visit(VectorLiteral& node) override;
        void visit(VectorIndex& node) override;
        void visit(VectorGenerator& node) override;

    private:
        const SemanticTables& tables_;
        const std::unordered_map<Expr*, ResolutionResult>& resolution_map_;
        hulk::common::DiagnosticEngine& engine_;

        std::unordered_map<Expr*, HulkType> type_map_;

        // Tipos inferidos para los símbolos
        std::unordered_map<const Param*, HulkType>           param_types_;
        std::unordered_map<VariableBinding*, HulkType>       binding_types_;
        std::unordered_map<const SyntheticSymbol*, HulkType> synthetic_types_;

        // Tipo de la última expresión visitada
        HulkType current_type_;

        bool changed_ = false;

        // Contexto actual para inferir retornos implícitos
        std::string current_type_decl_;
        std::string current_func_decl_;

        // Auxiliares
        void set_type(Expr& node, HulkType type);
        void refine_type(Expr& node, const HulkType& type);
        HulkType get_lca(const HulkType& a, const HulkType& b);
        HulkType infer_expr(Expr& node);

        // Mapeo desde notación textual a HulkType
        HulkType from_string_type(const std::string& type_name);
    };

} // namespace Hulk

#endif // HULK_TYPE_INFERENCER_H
