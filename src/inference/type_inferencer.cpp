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
                    
                    // Solo reportar errores de inferencia en la última iteración
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
                    for (auto& member : type_decl->GetMembers()) {
                        if (member.kind == TypeMember::Kind::Attribute) {
                            auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
                            infer_expr(*attr->GetInitializer());
                        } else if (member.kind == TypeMember::Kind::Method) {
                            auto* method = static_cast<TypeMemberMethod*>(member.node.get());
                            HulkType body_type = infer_expr(*method->GetBody());
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

    HulkType TypeInferencer::from_string_type(const std::string& type_name) {
        if (type_name.empty()) return HulkType::make_unknown();
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
        HulkType left = infer_expr(*node.GetLeft());
        HulkType right = infer_expr(*node.GetRight());
        if (left.is_error() || right.is_error()) set_type(node, HulkType::make_error());
        else set_type(node, HulkType::make_number());
    }

    void TypeInferencer::visit(LogicBinOp& node) {
        HulkType left = infer_expr(*node.GetLeft());
        HulkType right = infer_expr(*node.GetRight());
        if (left.is_error() || right.is_error()) set_type(node, HulkType::make_error());
        else set_type(node, HulkType::make_boolean());
    }

    void TypeInferencer::visit(StringBinOp& node) {
        HulkType left = infer_expr(*node.GetLeft());
        HulkType right = infer_expr(*node.GetRight());
        if (left.is_error() || right.is_error()) set_type(node, HulkType::make_error());
        else set_type(node, HulkType::make_string());
    }

    void TypeInferencer::visit(ArithmeticUnaryOp& node) {
        HulkType op = infer_expr(*node.GetOperand());
        if (op.is_error()) set_type(node, HulkType::make_error());
        else set_type(node, HulkType::make_number());
    }

    void TypeInferencer::visit(LogicUnaryOp& node) {
        HulkType op = infer_expr(*node.GetOperand());
        if (op.is_error()) set_type(node, HulkType::make_error());
        else set_type(node, HulkType::make_boolean());
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
                set_type(node, HulkType::make_unknown());
                return;
            } else if (res.kind == ResolutionKind::Synthetic) {
                if (res.synthetic->kind == SyntheticKind::Self) {
                    set_type(node, HulkType::make_object(res.synthetic->type_name));
                    return;
                }
                if (!res.synthetic->type_name.empty()) {
                    set_type(node, from_string_type(res.synthetic->type_name));
                    return;
                }
                set_type(node, HulkType::make_unknown());
                return;
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
        HulkType lca = infer_expr(*node.GetThenBranch());
        
        for (auto& elif : node.GetElifBranches()) {
            infer_expr(*elif.condition);
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
                if (res.func_decl->HasReturnTypeAnnotation()) {
                    set_type(node, from_string_type(res.func_decl->GetReturnTypeAnnotation()));
                    return;
                }
                auto body_it = type_map_.find(res.func_decl->GetBody());
                if (body_it != type_map_.end()) {
                    set_type(node, body_it->second);
                    return;
                }
            } else if (res.kind == ResolutionKind::BuiltinFunction) {
                if (res.builtin_func->name == "print") set_type(node, HulkType::make_string());
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
        infer_expr(*node.GetExpr());
        set_type(node, HulkType::make_string()); // print retorna String o Void según HULK? Hacemos String como default seguro
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
        if (!current_type_decl_.empty()) {
            if (const SemanticTypeInfo* t_info = tables_.lookup_type(current_type_decl_)) {
                if (!t_info->parent_name.empty()) {
                    set_type(node, HulkType::make_object(t_info->parent_name));
                    return;
                }
            }
        }
        set_type(node, HulkType::make_error());
    }

    void TypeInferencer::visit(NewExpr& node) {
        for (auto& arg : node.GetArgs()) {
            infer_expr(*arg);
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
        if (it != resolution_map_.end()) {
            const ResolutionResult& res = it->second;
            if (res.kind == ResolutionKind::Method) {
                if (!res.method_info->return_type_annotation.empty()) {
                    set_type(node, from_string_type(res.method_info->return_type_annotation));
                    return;
                }
                if (res.method_info->body) {
                    auto body_it = type_map_.find(res.method_info->body);
                    if (body_it != type_map_.end()) {
                        set_type(node, body_it->second);
                        return;
                    }
                }
            }
        } else if (obj_type.kind() == HulkType::Kind::Object) {
            // Si no hay resolución (no es self o es dinámico), buscar en el tipo inferido
            if (const SemanticMethodInfo* method = tables_.find_method(obj_type.name(), node.GetMethodName())) {
                if (!method->return_type_annotation.empty()) {
                    set_type(node, from_string_type(method->return_type_annotation));
                    return;
                }
                if (method->body) {
                    auto body_it = type_map_.find(method->body);
                    if (body_it != type_map_.end()) {
                        set_type(node, body_it->second);
                        return;
                    }
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
        set_type(node, HulkType::make_object(node.GetTypeName()));
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
