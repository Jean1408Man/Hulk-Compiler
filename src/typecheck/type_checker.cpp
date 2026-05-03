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
#include "../ast/types/typeDecl.h"
#include "../ast/types/typeMemberAttribute.h"
#include "../ast/types/typeMemberMethod.h"

namespace Hulk {

    TypeChecker::TypeChecker(const SemanticTables& tables,
                             const std::unordered_map<Expr*, HulkType>& type_map,
                             const std::unordered_map<Expr*, ResolutionResult>& resolution_map,
                             const std::unordered_map<const Param*, HulkType>& param_types,
                             const std::unordered_map<VariableBinding*, HulkType>& binding_types,
                             const std::unordered_map<const SyntheticSymbol*, HulkType>& synthetic_types,
                             hulk::common::DiagnosticEngine& engine)
        : tables_(tables), type_map_(type_map), resolution_map_(resolution_map), 
          param_types_(param_types), binding_types_(binding_types), synthetic_types_(synthetic_types),
          engine_(engine) {}

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
                if (td->HasParent()) {
                    std::vector<Param> parent_params = tables_.get_effective_constructor(td->GetParentName());
                    if (parent_params.size() == td->GetParentArgs().size()) {
                        for (size_t i = 0; i < parent_params.size(); ++i) {
                            td->GetParentArgs()[i]->accept(*this);
                            HulkType ann = from_string_type(parent_params[i].typeAnnotation);
                            if (!ann.is_unknown()) {
                                check_conforms(td->GetParentArgs()[i].get(), ann, "argumento " + std::to_string(i+1) + " de herencia de '" + td->GetParentName() + "'");
                            }
                        }
                    } else {
                        for (auto& arg : td->GetParentArgs()) arg->accept(*this);
                    }
                }
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
                        
                        // FN06: Chequear override post-inferencia
                        if (td->HasParent()) {
                            if (const SemanticMethodInfo* parent_method = tables_.find_method(td->GetParentName(), method->GetName())) {
                                HulkType parent_ret = from_string_type(parent_method->return_type_annotation);
                                HulkType child_ret = ret_type.is_unknown() ? get_type(method->GetBody()) : ret_type;
                                if (!parent_ret.is_unknown() && !child_ret.is_unknown() && !child_ret.is_error()) {
                                    if (!child_ret.conforms_to(parent_ret, tables_)) {
                                        report_error(method->span, "Método '" + method->GetName() + "' redefine el retorno con tipo incompatible ('" +
                                            child_ret.to_string() + "' no conforma a '" + parent_ret.to_string() + "').");
                                    }
                                }
                            }
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
        if (found.is_error()) return;
        
        // Si el tipo encontrado es Unknown (type hole), permite que conforme a cualquier tipo esperado
        // Esto ocurre cuando se usa `_` o `auto` en una expresión
        if (found.is_unknown()) {
            return;
        }
        
        // Si el tipo esperado es "auto" o "_", aceptar cualquier tipo (son type holes)
        if (expected.is_unknown()) {
            return;
        }
        
        if (!found.conforms_to(expected, tables_)) {
            report_error(node->span, "Error de tipos en " + context + ": se esperaba '" + 
                         expected.to_string() + "' pero se encontró '" + found.to_string() + "'.");
        }
    }

    HulkType TypeChecker::from_string_type(const std::string& name) {
        if (name.empty() || name == "auto" || name == "_") return HulkType::make_unknown();
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
        
        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end()) {
            const ResolutionResult& res = it->second;
            HulkType target_type = HulkType::make_unknown();
            
            if (res.kind == ResolutionKind::Variable) {
                target_type = from_string_type(res.binding->GetTypeAnnotation());
                if (target_type.is_unknown()) {
                    auto b_it = binding_types_.find(res.binding);
                    if (b_it != binding_types_.end()) target_type = b_it->second;
                }
            } else if (res.kind == ResolutionKind::Param) {
                target_type = from_string_type(res.param->typeAnnotation);
                if (target_type.is_unknown()) {
                    auto p_it = param_types_.find(res.param);
                    if (p_it != param_types_.end()) target_type = p_it->second;
                }
            }
            
            if (!target_type.is_unknown()) {
                check_conforms(node.GetValue(), target_type, "asignación a '" + node.GetName() + "'");
            }
        }
    }

    void TypeChecker::visit(DestructiveAssignMember& node) {
        node.GetObject()->accept(*this);
        node.GetValue()->accept(*this);
        
        HulkType obj_type = get_type(node.GetObject());
        if (obj_type.kind() == HulkType::Kind::Object) {
            const SemanticAttrInfo* attr = tables_.find_attribute(obj_type.name(), node.GetMemberName());
            if (attr) {
                HulkType attr_type = from_string_type(attr->type_annotation);
                if (!attr_type.is_unknown()) {
                    check_conforms(node.GetValue(), attr_type, "asignación a atributo '" + node.GetMemberName() + "'");
                }
            } else if (!obj_type.is_unknown() && !obj_type.is_error()) {
                report_error(node.span, "Tipo '" + obj_type.name() + "' no tiene atributo '" + node.GetMemberName() + "'.");
            }
        }
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
        if (it != resolution_map_.end()) {
            const ResolutionResult& res = it->second;
            if (res.kind == ResolutionKind::Function) {
                const FunctionDecl* decl = res.func_decl;
                if (decl && decl->GetParams().size() == node.GetArgs().size()) {
                    for (size_t i = 0; i < decl->GetParams().size(); ++i) {
                        const Param& p = decl->GetParams()[i];
                        HulkType ann = from_string_type(p.typeAnnotation);
                        if (ann.is_unknown()) {
                            auto p_it = param_types_.find(&p);
                            if (p_it != param_types_.end()) ann = p_it->second;
                        }
                        
                        if (!ann.is_unknown()) {
                            check_conforms(node.GetArgs()[i].get(), ann, "argumento " + std::to_string(i+1) + " de función '" + node.GetName() + "'");
                        }
                    }
                }
            } else if (res.kind == ResolutionKind::Method) {
                // Caso base() o resolución especial de función como método
                const SemanticMethodInfo* info = res.method_info;
                if (info && info->params.size() == node.GetArgs().size()) {
                    for (size_t i = 0; i < info->params.size(); ++i) {
                        const Param& p = info->params[i];
                        HulkType ann = from_string_type(p.typeAnnotation);
                        if (ann.is_unknown()) {
                            auto p_it = param_types_.find(&p);
                            if (p_it != param_types_.end()) ann = p_it->second;
                        }
                        
                        if (!ann.is_unknown()) {
                            check_conforms(node.GetArgs()[i].get(), ann, "argumento " + std::to_string(i+1) + " de llamada especial '" + node.GetName() + "'");
                        }
                    }
                }
            } else if (res.kind == ResolutionKind::BuiltinFunction) {
                const BuiltinFuncInfo* bfi = res.builtin_func;
                if (bfi && bfi->param_types.size() == node.GetArgs().size()) {
                    for (size_t i = 0; i < bfi->param_types.size(); ++i) {
                        HulkType ann = from_string_type(bfi->param_types[i]);
                        if (!ann.is_unknown()) {
                            check_conforms(node.GetArgs()[i].get(), ann, "argumento " + std::to_string(i+1) + " de función builtin '" + node.GetName() + "'");
                        }
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

        // Validar tipos de args contra la firma del método padre
        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end() && it->second.kind == ResolutionKind::Method) {
            const SemanticMethodInfo* info = it->second.method_info;
            if (info && info->params.size() == node.GetArgs().size()) {
                for (size_t i = 0; i < info->params.size(); ++i) {
                    HulkType ann = from_string_type(info->params[i].typeAnnotation);
                    if (!ann.is_unknown()) {
                        check_conforms(node.GetArgs()[i].get(), ann,
                            "argumento " + std::to_string(i+1) + " de base()");
                    }
                }
            } else if (info && info->params.size() != node.GetArgs().size()) {
                report_error(node.span, "Aridad incorrecta en base(): se esperaban " +
                    std::to_string(info->params.size()) + " argumento(s), se recibieron " +
                    std::to_string(node.GetArgs().size()) + ".");
            }
        }
    }

    void TypeChecker::visit(NewExpr& node) {
        for (auto& arg : node.GetArgs()) arg->accept(*this);
        
        std::vector<Param> params = tables_.get_effective_constructor(node.GetTypeName());
        if (params.size() != node.GetArgs().size()) {
            report_error(node.span, "Constructor de '" + node.GetTypeName() + "' espera " + 
                         std::to_string(params.size()) + " argumento(s) pero recibió " + 
                         std::to_string(node.GetArgs().size()) + ".");
        } else {
            for (size_t i = 0; i < params.size(); ++i) {
                const Param& p = params[i];
                HulkType ann = from_string_type(p.typeAnnotation);
                if (ann.is_unknown()) {
                    auto p_it = param_types_.find(&p);
                    if (p_it != param_types_.end()) ann = p_it->second;
                }
                
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

        HulkType obj_type = get_type(node.GetObject());
        const SemanticMethodInfo* info = nullptr;

        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end() && it->second.kind == ResolutionKind::Method) {
            info = it->second.method_info;
        } else if (obj_type.kind() == HulkType::Kind::Object) {
            // Resolución dinámica basada en el tipo inferido (Corte 9)
            info = tables_.find_method(obj_type.name(), node.GetMethodName());
            // Si el tipo es Error o Unknown, no reportamos error de método no encontrado (evitar cascada)
            if (!info && !obj_type.is_unknown() && !obj_type.is_error()) {
                report_error(node.span, "Tipo '" + obj_type.name() + "' no tiene un método '" + node.GetMethodName() + "'.");
            }
        }

        if (info) {
            if (info->params.size() != node.GetArgs().size()) {
                report_error(node.span, "Método '" + node.GetMethodName() + "' espera " + 
                             std::to_string(info->params.size()) + " argumento(s) pero recibió " + 
                             std::to_string(node.GetArgs().size()) + ".");
            } else {
                for (size_t i = 0; i < info->params.size(); ++i) {
                    // Usar anotación de la copia (para el texto del tipo)
                    HulkType ann = from_string_type(info->params[i].typeAnnotation);
                    if (ann.is_unknown() && i < info->ast_params.size()) {
                        // Buscar con el puntero al AST original (mismo que usó el inferencer)
                        auto p_it = param_types_.find(info->ast_params[i]);
                        if (p_it != param_types_.end()) ann = p_it->second;
                    }
                    if (!ann.is_unknown()) {
                        check_conforms(node.GetArgs()[i].get(), ann,
                            "argumento " + std::to_string(i+1) + " de método '" + node.GetMethodName() + "'");
                    }
                }
            }
        }
    }

    void TypeChecker::visit(IsExpr& node) {
        node.GetExpr()->accept(*this);
        HulkType expr_type = get_type(node.GetExpr());
        HulkType target_type = from_string_type(node.GetTypeName());
        
        if (!expr_type.is_unknown() && !target_type.is_unknown() && !expr_type.is_error() && !target_type.is_error()) {
            if (!expr_type.conforms_to(target_type, tables_) && 
                !target_type.conforms_to(expr_type, tables_)) {
                report_error(node.span, "Operación 'is' no plausible: tipo '" + expr_type.to_string() + 
                             "' y '" + target_type.to_string() + "' no están relacionados.");
            }
        }
    }

    void TypeChecker::visit(AsExpr& node) {
        node.GetExpr()->accept(*this);
        HulkType expr_type = get_type(node.GetExpr());
        HulkType target_type = from_string_type(node.GetTypeName());
        
        if (!expr_type.is_unknown() && !target_type.is_unknown() && !expr_type.is_error() && !target_type.is_error()) {
            if (!expr_type.conforms_to(target_type, tables_) && 
                !target_type.conforms_to(expr_type, tables_)) {
                report_error(node.span, "Operación 'as' no plausible: tipo '" + expr_type.to_string() + 
                             "' y '" + target_type.to_string() + "' no están relacionados.");
            }
        }
    }

}
