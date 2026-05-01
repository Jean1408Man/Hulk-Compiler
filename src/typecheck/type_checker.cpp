#include "type_checker.h"
#include "../ast/others/program.h"
#include "../ast/literales/number.h"
#include "../ast/literales/string.h"
#include "../ast/literales/boolean.h"
#include "../ast/binOps/arithmeticBinOp.h"
#include "../ast/binOps/logicBinOp.h"
#include "../ast/binOps/stringBinOp.h"
#include "../ast/unaryOps/arithmeticUnaryOp.h"
#include "../ast/unaryOps/logicUnaryOp.h"
#include "../ast/variables/variableReference.h"
#include "../ast/variables/variableBinding.h"
#include "../ast/variables/letIn.h"
#include "../ast/assignments/desctructiveAssign.h"
#include "../ast/assignments/destructiveAssignMember.h"
#include "../ast/conditionals/ifStmt.h"
#include "../ast/loops/while.h"
#include "../ast/loops/for.h"
#include "../ast/functions/functionCall.h"
#include "../ast/functions/functionDecl.h"
#include "../ast/functions/lambda.h"
#include "../ast/domainFunctions/print.h"
#include "../ast/domainFunctions/builtinCall.h"
#include "../ast/others/exprBlock.h"
#include "../ast/others/group.h"
#include "../ast/others/selfRef.h"
#include "../ast/others/baseCall.h"
#include "../ast/types/newExpr.h"
#include "../ast/types/memberAccess.h"
#include "../ast/types/methodCall.h"
#include "../ast/types/isExpr.h"
#include "../ast/types/asExpr.h"
#include "../ast/vectors/vectorLiteral.h"
#include "../ast/vectors/vectorIndex.h"
#include "../ast/vectors/vectorGenerator.h"
#include "../ast/types/typeDecl.h"
#include "../ast/types/typeMemberAttribute.h"
#include "../ast/types/typeMemberMethod.h"

namespace Hulk {

    TypeChecker::TypeChecker(const SemanticTables& tables,
                             const std::unordered_map<Expr*, HulkType>& type_map,
                             const std::unordered_map<Expr*, ResolutionResult>& resolution_map,
                             hulk::common::DiagnosticEngine& engine)
        : tables_(tables), type_map_(type_map), resolution_map_(resolution_map), engine_(engine) {}

    void TypeChecker::check(Program& program) {
        // Chequear declaraciones (funciones y tipos)
        for (auto& decl : program.GetDeclarations()) {
            if (auto* fd = dynamic_cast<FunctionDecl*>(decl.get())) {
                fd->GetBody()->accept(*this);
                HulkType ret_type = from_string_type(fd->GetReturnTypeAnnotation());
                if (!ret_type.is_unknown()) {
                    check_conforms(fd->GetBody(), ret_type, "retorno de función '" + fd->GetName() + "'");
                }
            } else if (auto* td = dynamic_cast<TypeDecl*>(decl.get())) {
                for (auto& member : td->GetMembers()) {
                    if (member.kind == TypeMember::Kind::Attribute) {
                        auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
                        attr->GetInitializer()->accept(*this);
                        HulkType attr_type = from_string_type(attr->GetTypeAnnotation());
                        if (!attr_type.is_unknown()) {
                            check_conforms(attr->GetInitializer(), attr_type, "inicialización de atributo '" + attr->GetName() + "'");
                        }
                    } else {
                        auto* method = static_cast<TypeMemberMethod*>(member.node.get());
                        method->GetBody()->accept(*this);
                        HulkType ret_type = from_string_type(method->GetReturnTypeAnnotation());
                        if (!ret_type.is_unknown()) {
                            check_conforms(method->GetBody(), ret_type, "retorno de método '" + method->GetName() + "'");
                        }
                    }
                }
            }
        }

        // Chequear expresión global
        if (program.GetGlobalExpr()) {
            program.GetGlobalExpr()->accept(*this);
        }
    }

    // Helpers
    HulkType TypeChecker::get_type(Expr* node) {
        auto it = type_map_.find(node);
        if (it != type_map_.end()) return it->second;
        return HulkType::make_unknown();
    }

    void TypeChecker::report_error(const hulk::common::Span& span, const std::string& msg) {
        engine_.report_raw(hulk::common::DiagnosticLevel::Semantic,
                           hulk::common::Severity::Error, span, msg);
    }

    void TypeChecker::check_conforms(Expr* node, const HulkType& expected, const std::string& context) {
        HulkType found = get_type(node);
        if (!found.conforms_to(expected, tables_)) {
            report_error(node->span, "Error de tipos en " + context + ": se esperaba '" + 
                         expected.to_string() + "' pero se encontró '" + found.to_string() + "'.");
        }
    }

    HulkType TypeChecker::from_string_type(const std::string& name) {
        if (name.empty()) return HulkType::make_unknown();
        if (name == "Number") return HulkType::make_number();
        if (name == "String") return HulkType::make_string();
        if (name == "Boolean") return HulkType::make_boolean();
        if (name == "Void") return HulkType::make_void();
        return HulkType::make_object(name);
    }

    // Literales
    void TypeChecker::visit(Number&) {}
    void TypeChecker::visit(String&) {}
    void TypeChecker::visit(Boolean&) {}

    // Operaciones
    void TypeChecker::visit(ArithmeticBinOp& node) {
        node.GetLeft()->accept(*this);
        node.GetRight()->accept(*this);
        check_conforms(node.GetLeft(), HulkType::make_number(), "operando izquierdo de operador aritmético");
        check_conforms(node.GetRight(), HulkType::make_number(), "operando derecho de operador aritmético");
    }

    void TypeChecker::visit(LogicBinOp& node) {
        node.GetLeft()->accept(*this);
        node.GetRight()->accept(*this);
        
        LogicOp op = node.GetOperator();
        if (op == LogicOp::And || op == LogicOp::Or) {
            check_conforms(node.GetLeft(), HulkType::make_boolean(), "operando izquierdo de operador lógico");
            check_conforms(node.GetRight(), HulkType::make_boolean(), "operando derecho de operador lógico");
        } else {
            // Comparaciones: <, >, <=, >=, ==, !=
            // En HULK, <, >, <=, >= operan sobre Number.
            // == y != pueden operar sobre cualquier cosa, pero por ahora pedimos Number o que coincidan.
            if (op != LogicOp::Equal && op != LogicOp::NotEqual) {
                check_conforms(node.GetLeft(), HulkType::make_number(), "operando izquierdo de comparación");
                check_conforms(node.GetRight(), HulkType::make_number(), "operando derecho de comparación");
            }
        }
    }

    void TypeChecker::visit(StringBinOp& node) {
        node.GetLeft()->accept(*this);
        node.GetRight()->accept(*this);
    }

    void TypeChecker::visit(ArithmeticUnaryOp& node) {
        node.GetOperand()->accept(*this);
        check_conforms(node.GetOperand(), HulkType::make_number(), "operando de operador aritmético unario");
    }

    void TypeChecker::visit(LogicUnaryOp& node) {
        node.GetOperand()->accept(*this);
        check_conforms(node.GetOperand(), HulkType::make_boolean(), "operando de operador lógico unario");
    }

    // Variables
    void TypeChecker::visit(VariableReference&) {}

    void TypeChecker::visit(VariableBinding& node) {
        node.GetInitializer()->accept(*this);
        HulkType ann = from_string_type(node.GetTypeAnnotation());
        if (!ann.is_unknown()) {
            check_conforms(node.GetInitializer(), ann, "inicialización de variable '" + node.GetName() + "'");
        }
    }

    void TypeChecker::visit(LetIn& node) {
        for (auto& b : node.GetBindings()) b->accept(*this);
        node.GetBody()->accept(*this);
    }

    // Asignaciones
    void TypeChecker::visit(DestructiveAssign& node) {
        node.GetValue()->accept(*this);
    }

    void TypeChecker::visit(DestructiveAssignMember& node) {
        node.GetObject()->accept(*this);
        node.GetValue()->accept(*this);
    }

    // Control de flujo
    void TypeChecker::visit(IfStmt& node) {
        node.GetCondition()->accept(*this);
        check_conforms(node.GetCondition(), HulkType::make_boolean(), "condición de 'if'");
        node.GetThenBranch()->accept(*this);
        for (auto& elif : node.GetElifBranches()) {
            elif.condition->accept(*this);
            check_conforms(elif.condition.get(), HulkType::make_boolean(), "condición de 'elif'");
            elif.body->accept(*this);
        }
        if (node.GetElseBranch()) node.GetElseBranch()->accept(*this);
    }

    void TypeChecker::visit(WhileStmt& node) {
        node.GetCondition()->accept(*this);
        check_conforms(node.GetCondition(), HulkType::make_boolean(), "condición de 'while'");
        node.GetBody()->accept(*this);
    }

    void TypeChecker::visit(For& node) {
        node.GetIterable()->accept(*this);
        node.GetBody()->accept(*this);
    }

    // Llamadas
    void TypeChecker::visit(FunctionCall& node) {
        for (auto& arg : node.GetArgs()) arg->accept(*this);
        
        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end() && it->second.kind == ResolutionKind::Function) {
            const FunctionDecl* decl = it->second.func_decl;
            if (decl && decl->GetParams().size() == node.GetArgs().size()) {
                for (size_t i = 0; i < decl->GetParams().size(); ++i) {
                    HulkType ann = from_string_type(decl->GetParams()[i].typeAnnotation);
                    if (!ann.is_unknown()) {
                        check_conforms(node.GetArgs()[i].get(), ann, "argumento " + std::to_string(i+1) + " de función '" + node.GetName() + "'");
                    }
                }
            }
        }
    }

    void TypeChecker::visit(Lambda& node) {
        node.GetBody()->accept(*this);
    }

    void TypeChecker::visit(Print& node) {
        node.GetExpr()->accept(*this);
    }

    void TypeChecker::visit(BuiltinCall& node) {
        for (auto& arg : node.GetArgs()) arg->accept(*this);
        if (node.GetFunc() == BuiltinFunc::Sqrt || node.GetFunc() == BuiltinFunc::Sin || 
            node.GetFunc() == BuiltinFunc::Cos || node.GetFunc() == BuiltinFunc::Exp || 
            node.GetFunc() == BuiltinFunc::Log) {
            for (auto& arg : node.GetArgs()) {
                check_conforms(arg.get(), HulkType::make_number(), "argumento de función builtin");
            }
        }
    }

    void TypeChecker::visit(ExprBlock& node) {
        for (auto& e : node.GetExprs()) e->accept(*this);
    }

    void TypeChecker::visit(Group& node) {
        node.GetExpr()->accept(*this);
    }

    void TypeChecker::visit(SelfRef&) {}

    void TypeChecker::visit(BaseCall& node) {
        for (auto& arg : node.GetArgs()) arg->accept(*this);
    }

    void TypeChecker::visit(NewExpr& node) {
        for (auto& arg : node.GetArgs()) arg->accept(*this);
        const SemanticTypeInfo* info = tables_.lookup_type(node.GetTypeName());
        if (info && info->ctor_params.size() == node.GetArgs().size()) {
            for (size_t i = 0; i < info->ctor_params.size(); ++i) {
                HulkType ann = from_string_type(info->ctor_params[i].typeAnnotation);
                if (!ann.is_unknown()) {
                    check_conforms(node.GetArgs()[i].get(), ann, "argumento " + std::to_string(i+1) + " de constructor de '" + node.GetTypeName() + "'");
                }
            }
        }
    }

    void TypeChecker::visit(MemberAccess& node) {
        node.GetObject()->accept(*this);
    }

    void TypeChecker::visit(MethodCall& node) {
        node.GetObject()->accept(*this);
        for (auto& arg : node.GetArgs()) arg->accept(*this);

        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end() && it->second.kind == ResolutionKind::Method) {
            const SemanticMethodInfo* info = it->second.method_info;
            if (info && info->params.size() == node.GetArgs().size()) {
                for (size_t i = 0; i < info->params.size(); ++i) {
                    HulkType ann = from_string_type(info->params[i].typeAnnotation);
                    if (!ann.is_unknown()) {
                        check_conforms(node.GetArgs()[i].get(), ann, "argumento " + std::to_string(i+1) + " de método '" + node.GetMethodName() + "'");
                    }
                }
            }
        }
    }

    void TypeChecker::visit(IsExpr& node) {
        node.GetExpr()->accept(*this);
    }

    void TypeChecker::visit(AsExpr& node) {
        node.GetExpr()->accept(*this);
    }

    void TypeChecker::visit(VectorLiteral& node) {
        for (auto& e : node.GetElements()) e->accept(*this);
    }

    void TypeChecker::visit(VectorIndex& node) {
        node.GetVector()->accept(*this);
        node.GetIndex()->accept(*this);
        check_conforms(node.GetIndex(), HulkType::make_number(), "índice de vector");
    }

    void TypeChecker::visit(VectorGenerator& node) {
        node.GetIterable()->accept(*this);
        node.GetBody()->accept(*this);
    }

}
