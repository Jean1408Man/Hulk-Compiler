#include "type_inferencer.h"
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
#include "../ast/vectors/vectorLiteral.h"
#include "../ast/vectors/vectorIndex.h"
#include "../ast/vectors/vectorGenerator.h"

namespace Hulk {

    TypeInferencer::TypeInferencer(const SemanticTables& tables,
                                   const std::unordered_map<Expr*, ResolutionResult>& resolution_map,
                                   hulk::common::DiagnosticEngine& engine)
        : tables_(tables), resolution_map_(resolution_map), engine_(engine), current_type_(HulkType::make_unknown()) {}

    void TypeInferencer::infer(Program& program) {
        int max_iterations = 10;
        int iteration = 0;
        changed_ = true;

        while (changed_ && iteration < max_iterations) {
            changed_ = false;
            iteration++;

            for (auto& decl : program.GetDeclarations()) {
                if (auto* func_decl = dynamic_cast<FunctionDecl*>(decl.get())) {
                    current_func_decl_ = func_decl->GetName();
                    current_type_decl_ = "";
                    
                    HulkType body_type = infer_expr(*func_decl->GetBody());
                    
                    if (iteration == max_iterations || !changed_) {
                        if (!func_decl->HasReturnTypeAnnotation()) {
                            if (body_type.is_error() || body_type.is_unknown()) {
                                engine_.report_raw(hulk::common::DiagnosticLevel::Semantic, hulk::common::Severity::Error, func_decl->span, 
                                    "Cannot infer return type for function '" + func_decl->GetName() + "'. Explicit type annotation required.");
                            }
                        }
                    }
                } else if (auto* type_decl = dynamic_cast<TypeDecl*>(decl.get())) {
                    current_type_decl_ = type_decl->GetName();
                    current_func_decl_ = "";
                    
                    // Visitar argumentos de herencia
                    if (type_decl->HasParent()) {
                        for (auto& arg : type_decl->GetParentArgs()) {
                            infer_expr(*arg);
                        }
                    }

                    for (auto& member : type_decl->GetMembers()) {

                        if (member.kind == TypeMember::Kind::Attribute) {
                            auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
                            infer_expr(*attr->GetInitializer());
                        } else if (member.kind == TypeMember::Kind::Method) {
                            auto* method = static_cast<TypeMemberMethod*>(member.node.get());
                            current_func_decl_ = method->GetName();
                            HulkType body_type = infer_expr(*method->GetBody());
                            current_func_decl_ = "";
                            if (iteration == max_iterations || !changed_) {
                                if (!method->HasReturnTypeAnnotation()) {
                                    if (body_type.is_error() || body_type.is_unknown()) {
                                        engine_.report_raw(hulk::common::DiagnosticLevel::Semantic, hulk::common::Severity::Error, method->span, 
                                            "Cannot infer return type for method '" + method->GetName() + "'. Explicit type annotation required.");
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (program.GetGlobalExpr()) {
                infer_expr(*program.GetGlobalExpr());
            }
        }
    }

    HulkType TypeInferencer::infer_expr(Expr& node) {
        node.accept(*this);
        return current_type_;
    }

    void TypeInferencer::set_type(Expr& node, HulkType type) {
        auto it = type_map_.find(&node);
        if (it != type_map_.end()) {
            if (it->second != type) {
                if (it->second.is_unknown() && !type.is_unknown()) {
                    it->second = type;
                    changed_ = true;
                } else if (!it->second.is_unknown() && !type.is_unknown() && it->second != type) {
                    HulkType lca = get_lca(it->second, type);
                    if (lca != it->second) {
                        it->second = lca;
                        changed_ = true;
                    }
                }
            }
        } else {
            type_map_[&node] = type;
            changed_ = true;
        }
        current_type_ = type;
    }

    void TypeInferencer::refine_type(Expr& node, const HulkType& type) {
        if (type.is_unknown() || type.is_error()) return;
        
        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end()) {
            const ResolutionResult& res = it->second;
            if (res.kind == ResolutionKind::Variable) {
                if (res.binding->HasTypeAnnotation()) return;
                auto& current = binding_types_[res.binding];
                if (current.is_unknown()) {
                    current = type;
                    changed_ = true;
                }
            } else if (res.kind == ResolutionKind::Param) {
                if (res.param->HasTypeAnnotation()) return;
                auto& current = param_types_[res.param];
                if (current.is_unknown()) {
                    current = type;
                    changed_ = true;
                }
            } else if (res.kind == ResolutionKind::Synthetic) {
                if (!res.synthetic->type_name.empty()) return;
                auto& current = synthetic_types_[res.synthetic];
                if (current.is_unknown() || (current.kind() == HulkType::Kind::Object && current.name() == "Object" && type.kind() == HulkType::Kind::Object && type.name() != "Object")) {
                    current = type;
                    changed_ = true;
                }
            }
        }

        // Propagación recursiva para MemberAccess
        if (auto* mem = dynamic_cast<MemberAccess*>(&node)) {
            HulkType obj_type = type_map_[mem->GetObject()];
            if (obj_type.kind() == HulkType::Kind::Object) {
                if (const SemanticAttrInfo* attr = tables_.find_attribute(obj_type.name(), mem->GetMemberName())) {
                    if (attr->initializer && attr->type_annotation.empty()) {
                        refine_type(*attr->initializer, type);
                    }
                }
            }
        }
    }
    void TypeInferencer::refine_param_type(const Param* param, const HulkType& type) {
        if (type.is_unknown() || type.is_error()) return;
        if (param->HasTypeAnnotation()) return;
        
        auto& current = param_types_[param];
        if (current.is_unknown()) {
            current = type;
            changed_ = true;
        } else if (!current.is_unknown() && !type.is_unknown() && current != type) {
            HulkType lca = get_lca(current, type);
            if (lca != current) {
                current = lca;
                changed_ = true;
            }
        }
    }


    HulkType TypeInferencer::from_string_type(const std::string& type_name) {
        if (type_name.empty() || type_name == "auto" || type_name == "_") return HulkType::make_unknown();
        if (type_name == "Number") return HulkType::make_number();
        if (type_name == "String") return HulkType::make_string();
        if (type_name == "Boolean") return HulkType::make_boolean();
        if (type_name == "Void") return HulkType::make_void();
        return HulkType::make_object(type_name);
    }

    HulkType TypeInferencer::get_lca(const HulkType& a, const HulkType& b) {
        if (a.is_error() || b.is_error()) return HulkType::make_error();
        if (a.is_unknown()) return b;
        if (b.is_unknown()) return a;
        if (a == b) return a;
        
        if (a.kind() == HulkType::Kind::Object && b.kind() == HulkType::Kind::Object) {
            std::string lca_name = tables_.find_lca(a.name(), b.name());
            return HulkType::make_object(lca_name);
        }
        
        return HulkType::make_object("Object");
    }

    void TypeInferencer::visit(Number& node) {
        set_type(node, HulkType::make_number());
    }

    void TypeInferencer::visit(String& node) {
        set_type(node, HulkType::make_string());
    }

    void TypeInferencer::visit(Boolean& node) {
        set_type(node, HulkType::make_boolean());
    }

    void TypeInferencer::visit(ArithmeticBinOp& node) {
        infer_expr(*node.GetLeft());
        refine_type(*node.GetLeft(), HulkType::make_number());
        infer_expr(*node.GetRight());
        refine_type(*node.GetRight(), HulkType::make_number());
        set_type(node, HulkType::make_number());
    }

    void TypeInferencer::visit(LogicBinOp& node) {
        infer_expr(*node.GetLeft());
        infer_expr(*node.GetRight());
        
        LogicOp op = node.GetOperator();
        if (op == LogicOp::And || op == LogicOp::Or) {
            refine_type(*node.GetLeft(), HulkType::make_boolean());
            refine_type(*node.GetRight(), HulkType::make_boolean());
        } else if (op != LogicOp::Equal && op != LogicOp::NotEqual) {
            refine_type(*node.GetLeft(), HulkType::make_number());
            refine_type(*node.GetRight(), HulkType::make_number());
        }
        
        set_type(node, HulkType::make_boolean());
    }

    void TypeInferencer::visit(StringBinOp& node) {
        HulkType left = infer_expr(*node.GetLeft());
        HulkType right = infer_expr(*node.GetRight());
        if (left.is_error() || right.is_error()) set_type(node, HulkType::make_error());
        else set_type(node, HulkType::make_string());
    }

    void TypeInferencer::visit(ArithmeticUnaryOp& node) {
        infer_expr(*node.GetOperand());
        refine_type(*node.GetOperand(), HulkType::make_number());
        set_type(node, HulkType::make_number());
    }

    void TypeInferencer::visit(LogicUnaryOp& node) {
        infer_expr(*node.GetOperand());
        refine_type(*node.GetOperand(), HulkType::make_boolean());
        set_type(node, HulkType::make_boolean());
    }

    void TypeInferencer::visit(VariableReference& node) {
        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end()) {
            const ResolutionResult& res = it->second;
            if (res.kind == ResolutionKind::Variable) {
                VariableBinding* binding = res.binding;
                if (binding->HasTypeAnnotation()) {
                    set_type(node, from_string_type(binding->GetTypeAnnotation()));
                    return;
                }
                auto it_b = binding_types_.find(binding);
                if (it_b != binding_types_.end() && !it_b->second.is_unknown()) {
                    set_type(node, it_b->second);
                    return;
                }
                auto type_it = type_map_.find(binding->GetInitializer());
                if (type_it != type_map_.end()) {
                    set_type(node, type_it->second);
                    return;
                }
            } else if (res.kind == ResolutionKind::Param) {
                if (res.param->HasTypeAnnotation()) {
                    set_type(node, from_string_type(res.param->typeAnnotation));
                    return;
                }
                auto it_p = param_types_.find(res.param);
                if (it_p != param_types_.end() && !it_p->second.is_unknown()) {
                    set_type(node, it_p->second);
                    return;
                }
            } else if (res.kind == ResolutionKind::Synthetic) {
                if (res.synthetic->kind == SyntheticKind::Self) {
                    set_type(node, HulkType::make_object(res.synthetic->type_name));
                    return;
                }
                auto it_s = synthetic_types_.find(res.synthetic);
                if (it_s != synthetic_types_.end() && !it_s->second.is_unknown()) {
                    set_type(node, it_s->second);
                    return;
                }
                if (!res.synthetic->type_name.empty()) {
                    set_type(node, from_string_type(res.synthetic->type_name));
                    return;
                }
            } else if (res.kind == ResolutionKind::BuiltinConstant) {
                set_type(node, from_string_type(res.builtin_const->type));
                return;
            }
        }
        set_type(node, HulkType::make_unknown());
    }

    void TypeInferencer::visit(VariableBinding& node) {
        HulkType init_type = infer_expr(*node.GetInitializer());
        set_type(node, init_type);
        
        // Propagar al mapa de bindings para que las referencias lo vean
        if (!node.HasTypeAnnotation()) {
            auto& current = binding_types_[&node];
            if (current.is_unknown() && !init_type.is_unknown()) {
                current = init_type;
                changed_ = true;
            } else if (!current.is_unknown() && !init_type.is_unknown() && current != init_type) {
                HulkType lca = get_lca(current, init_type);
                if (lca != current) {
                    current = lca;
                    changed_ = true;
                }
            }
        }
    }

    void TypeInferencer::visit(LetIn& node) {
        for (auto& binding : node.GetBindings()) {
            infer_expr(*binding);
        }
        HulkType body_type = infer_expr(*node.GetBody());
        set_type(node, body_type);
    }

    void TypeInferencer::visit(DestructiveAssign& node) {
        HulkType val_type = infer_expr(*node.GetValue());
        set_type(node, val_type);
    }

    void TypeInferencer::visit(DestructiveAssignMember& node) {
        HulkType val_type = infer_expr(*node.GetValue());
        set_type(node, val_type);
    }

    void TypeInferencer::visit(IfStmt& node) {
        infer_expr(*node.GetCondition());
        refine_type(*node.GetCondition(), HulkType::make_boolean()); // condición debe ser Boolean
        HulkType lca = infer_expr(*node.GetThenBranch());
        
        for (auto& elif : node.GetElifBranches()) {
            infer_expr(*elif.condition);
            refine_type(*elif.condition, HulkType::make_boolean());
            HulkType elif_type = infer_expr(*elif.body);
            lca = get_lca(lca, elif_type);
        }
        
        if (node.GetElseBranch()) {
            HulkType else_type = infer_expr(*node.GetElseBranch());
            lca = get_lca(lca, else_type);
        } else {
            lca = get_lca(lca, HulkType::make_void());
        }
        
        set_type(node, lca);
    }

    void TypeInferencer::visit(WhileStmt& node) {
        infer_expr(*node.GetCondition());
        refine_type(*node.GetCondition(), HulkType::make_boolean()); // condición debe ser Boolean
        HulkType body_type = infer_expr(*node.GetBody());
        set_type(node, body_type);
    }

    void TypeInferencer::visit(For& node) {
        infer_expr(*node.GetIterable());
        HulkType body_type = infer_expr(*node.GetBody());
        set_type(node, body_type);
    }

    void TypeInferencer::visit(FunctionCall& node) {
        for (auto& arg : node.GetArgs()) {
            infer_expr(*arg);
        }
        
        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end()) {
            const ResolutionResult& res = it->second;
            if (res.kind == ResolutionKind::Function) {
                // Propagación hacia parámetros
                const FunctionDecl* decl = res.func_decl;
                for (size_t i = 0; i < node.GetArgs().size() && i < decl->GetParams().size(); ++i) {
                    HulkType arg_type = type_map_[node.GetArgs()[i].get()];
                    if (!arg_type.is_unknown() && !arg_type.is_error()) {
                        refine_param_type(&decl->GetParams()[i], arg_type);
                    }
                    
                    HulkType param_type = from_string_type(decl->GetParams()[i].typeAnnotation);
                    if (param_type.is_unknown()) {
                        auto p_it = param_types_.find(&decl->GetParams()[i]);
                        if (p_it != param_types_.end()) param_type = p_it->second;
                    }
                    if (!param_type.is_unknown()) {
                        refine_type(*node.GetArgs()[i], param_type);
                    }
                }

                if (res.func_decl->HasReturnTypeAnnotation()) {
                    set_type(node, from_string_type(res.func_decl->GetReturnTypeAnnotation()));
                    return;
                }
                auto body_it = type_map_.find(res.func_decl->GetBody());
                if (body_it != type_map_.end()) {
                    set_type(node, body_it->second);
                    return;
                }
            } else if (res.kind == ResolutionKind::Method) {
                const SemanticMethodInfo* info = res.method_info;
                // Propagación hacia parámetros
                for (size_t i = 0; i < node.GetArgs().size() && i < info->ast_params.size(); ++i) {
                    HulkType arg_type = type_map_[node.GetArgs()[i].get()];
                    if (!arg_type.is_unknown() && !arg_type.is_error()) {
                        refine_param_type(info->ast_params[i], arg_type);
                    }
                    
                    HulkType param_type = from_string_type(info->params[i].typeAnnotation);
                    if (param_type.is_unknown()) {
                        auto p_it = param_types_.find(info->ast_params[i]);
                        if (p_it != param_types_.end()) param_type = p_it->second;
                    }
                    if (!param_type.is_unknown()) {
                        refine_type(*node.GetArgs()[i], param_type);
                    }
                }

                if (!info->return_type_annotation.empty()) {
                    set_type(node, from_string_type(info->return_type_annotation));
                    return;
                }
                if (info->body) {
                    auto body_it = type_map_.find(info->body);
                    if (body_it != type_map_.end()) {
                        set_type(node, body_it->second);
                        return;
                    }
                }
            } else if (res.kind == ResolutionKind::BuiltinFunction) {

                if (res.builtin_func->name == "print") {
                    if (!node.GetArgs().empty()) {
                        auto arg_it = type_map_.find(node.GetArgs()[0].get());
                        set_type(node, (arg_it != type_map_.end()) ? arg_it->second : HulkType::make_object("Object"));
                    } else {
                        set_type(node, HulkType::make_object("Object"));
                    }
                }
                else if (res.builtin_func->name == "range") set_type(node, HulkType::make_object("Iterable"));
                else set_type(node, HulkType::make_number());
                return;
            }
        }
        set_type(node, HulkType::make_unknown());
    }


    void TypeInferencer::visit(Lambda& node) {
        // Lambdas are out of scope for core inference
        // but we visit the body just in case
        infer_expr(*node.GetBody());
        set_type(node, HulkType::make_unknown());
    }

    void TypeInferencer::visit(Print& node) {
        HulkType arg_type = infer_expr(*node.GetExpr());
        set_type(node, arg_type); // Retorno identidad
    }

    void TypeInferencer::visit(BuiltinCall& node) {
        for (auto& arg : node.GetArgs()) {
            infer_expr(*arg);
        }
        // Todas las funciones matemáticas (sin, cos, sqrt) retornan Number. 
        if (node.GetFunc() == BuiltinFunc::Sqrt || node.GetFunc() == BuiltinFunc::Sin || node.GetFunc() == BuiltinFunc::Cos || node.GetFunc() == BuiltinFunc::Exp || node.GetFunc() == BuiltinFunc::Log || node.GetFunc() == BuiltinFunc::Rand) {
            set_type(node, HulkType::make_number());
        } else {
            set_type(node, HulkType::make_unknown());
        }
    }

    void TypeInferencer::visit(ExprBlock& node) {
        HulkType last_type = HulkType::make_void();
        for (auto& expr : node.GetExprs()) {
            last_type = infer_expr(*expr);
        }
        set_type(node, last_type);
    }

    void TypeInferencer::visit(Group& node) {
        HulkType inner_type = infer_expr(*node.GetExpr());
        set_type(node, inner_type);
    }

    void TypeInferencer::visit(SelfRef& node) {
        if (!current_type_decl_.empty()) {
            set_type(node, HulkType::make_object(current_type_decl_));
        } else {
            engine_.report_raw(hulk::common::DiagnosticLevel::Semantic, hulk::common::Severity::Error, node.span, "Cannot use 'self' outside a class");
            set_type(node, HulkType::make_error());
        }
    }

    void TypeInferencer::visit(BaseCall& node) {
        for (auto& arg : node.GetArgs()) {
            infer_expr(*arg);
        }
        
        auto it = resolution_map_.find(&node);
        if (it != resolution_map_.end() && it->second.kind == ResolutionKind::Method) {
            const SemanticMethodInfo* parent_method = it->second.method_info;
            if (!parent_method->return_type_annotation.empty()) {
                set_type(node, from_string_type(parent_method->return_type_annotation));
                return;
            }
            if (parent_method->body) {
                auto body_it = type_map_.find(parent_method->body);
                if (body_it != type_map_.end() && !body_it->second.is_unknown()) {
                    set_type(node, body_it->second);
                    return;
                }
            }
            // Retorno aún no inferido — dejar Unknown para siguiente iteración
            set_type(node, HulkType::make_unknown());
            return;
        }
        set_type(node, HulkType::make_error());
    }

    void TypeInferencer::visit(NewExpr& node) {
        for (auto& arg : node.GetArgs()) {
            infer_expr(*arg);
        }
        
        const SemanticTypeInfo* info = tables_.lookup_type(node.GetTypeName());
        if (info && info->decl) {
            const auto& ast_params = info->decl->GetCtorParams();
            for (size_t i = 0; i < node.GetArgs().size() && i < ast_params.size(); ++i) {
                HulkType arg_type = type_map_[node.GetArgs()[i].get()];
                if (!arg_type.is_unknown() && !arg_type.is_error()) {
                    refine_param_type(&ast_params[i], arg_type);
                }
            }
        }
        
        set_type(node, HulkType::make_object(node.GetTypeName()));
    }


    void TypeInferencer::visit(MemberAccess& node) {
        HulkType obj_type = infer_expr(*node.GetObject());
        if (obj_type.kind() == HulkType::Kind::Object) {
            if (const SemanticAttrInfo* attr = tables_.find_attribute(obj_type.name(), node.GetMemberName())) {
                if (!attr->type_annotation.empty()) {
                    set_type(node, from_string_type(attr->type_annotation));
                    return;
                }
                if (attr->initializer) {
                    auto it = type_map_.find(attr->initializer);
                    if (it != type_map_.end()) {
                        set_type(node, it->second);
                        return;
                    }
                }
            }
        }
        set_type(node, HulkType::make_unknown());
    }

    void TypeInferencer::visit(MethodCall& node) {
        HulkType obj_type = infer_expr(*node.GetObject());
        for (auto& arg : node.GetArgs()) {
            infer_expr(*arg);
        }
        
        auto it = resolution_map_.find(&node);
        const SemanticMethodInfo* info = nullptr;
        if (it != resolution_map_.end()) {
            const ResolutionResult& res = it->second;
            if (res.kind == ResolutionKind::Method) {
                info = res.method_info;
            }
        } else if (obj_type.kind() == HulkType::Kind::Object) {
            info = tables_.find_method(obj_type.name(), node.GetMethodName());
        }

        if (info) {
            // Propagación hacia parámetros
            for (size_t i = 0; i < node.GetArgs().size() && i < info->ast_params.size(); ++i) {
                HulkType arg_type = type_map_[node.GetArgs()[i].get()];
                if (!arg_type.is_unknown() && !arg_type.is_error()) {
                    refine_param_type(info->ast_params[i], arg_type);
                }
                
                HulkType param_type = from_string_type(info->params[i].typeAnnotation);
                if (param_type.is_unknown()) {
                    auto p_it = param_types_.find(info->ast_params[i]);
                    if (p_it != param_types_.end()) param_type = p_it->second;
                }
                if (!param_type.is_unknown()) {
                    refine_type(*node.GetArgs()[i], param_type);
                }
            }

            if (!info->return_type_annotation.empty()) {
                set_type(node, from_string_type(info->return_type_annotation));
                return;
            }
            if (info->body) {
                auto body_it = type_map_.find(info->body);
                if (body_it != type_map_.end()) {
                    set_type(node, body_it->second);
                    return;
                }
            }
        }
        set_type(node, HulkType::make_unknown());
    }


    void TypeInferencer::visit(IsExpr& node) {
        infer_expr(*node.GetExpr());
        set_type(node, HulkType::make_boolean());
    }

    void TypeInferencer::visit(AsExpr& node) {
        infer_expr(*node.GetExpr());
        set_type(node, from_string_type(node.GetTypeName()));
    }

    void TypeInferencer::visit(VectorLiteral& node) {
        for (auto& elem : node.GetElements()) {
            infer_expr(*elem);
        }
        set_type(node, HulkType::make_object("Iterable"));
    }

    void TypeInferencer::visit(VectorIndex& node) {
        infer_expr(*node.GetVector());
        infer_expr(*node.GetIndex());
        set_type(node, HulkType::make_unknown());
    }

    void TypeInferencer::visit(VectorGenerator& node) {
        infer_expr(*node.GetIterable());
        infer_expr(*node.GetBody());
        set_type(node, HulkType::make_object("Iterable"));
    }

} // namespace Hulk
