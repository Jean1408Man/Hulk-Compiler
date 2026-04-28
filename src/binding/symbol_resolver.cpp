#include "symbol_resolver.h"

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
#include "../ast/protocols/protocolDecl.h"
#include "../common/diagnosticRepository.hpp"

#include <sstream>
#include <unordered_set>

namespace Hulk {

SymbolResolver::SymbolResolver(SemanticTables& tables,
                               hulk::common::DiagnosticEngine& engine)
    : tables_(tables),
      engine_(engine),
      scope_(std::make_shared<StaticScope>())
{}

// Punto de entrada
bool SymbolResolver::run(Program& program) {
    // --- Pase 1: registrar todas las declaraciones en las tablas ---
    for (auto& decl : program.GetDeclarations())
        decl->accept(*this);

    // --- Pase 3: chequeos estructurales sobre las tablas ---
    run_checks();

    // --- Pase 2: resolver referencias en la expresión global ---
    if (Expr* global = program.GetGlobalExpr())
        resolve(global);

    // También resolver los cuerpos de funciones y métodos registrados
    for (auto& decl : program.GetDeclarations()) {
        if (auto* fd = dynamic_cast<FunctionDecl*>(decl.get())) {
            // Scope de la función: parámetros 
            push_scope();
            for (const auto& p : fd->GetParams())
                scope_->define_param(p.name, &p);
            current_func_name_ = fd->GetName();
            resolve(fd->GetBody());
            current_func_name_.clear();
            pop_scope();
        } else if (auto* td = dynamic_cast<TypeDecl*>(decl.get())) {
            current_type_name_ = td->GetName();
            // Resolver atributos e inicializadores de métodos
            for (auto& member : td->GetMembers()) {
                if (member.kind == TypeMember::Kind::Attribute) {
                    auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
                    // Los atributos se resuelven en el scope del tipo (con ctor params + self)
                    push_scope();
                    for (const auto& p : td->GetCtorParams())
                        scope_->define_param(p.name, &p);
                    scope_->define_param("self", nullptr); // self disponible en atributos
                    resolve(attr->GetInitializer());
                    pop_scope();
                } else {
                    auto* method = static_cast<TypeMemberMethod*>(member.node.get());
                    push_scope();
                    for (const auto& p : td->GetCtorParams())
                        scope_->define_param(p.name, &p);
                    for (const auto& p : method->GetParams())
                        scope_->define_param(p.name, &p);
                    scope_->define_param("self", nullptr); // self disponible en métodos
                    current_func_name_ = method->GetName();
                    resolve(method->GetBody());
                    current_func_name_.clear();
                    pop_scope();
                }
            }
            current_type_name_.clear();
        }
    }

    return !has_errors_;
}

// Helpers de scope
void SymbolResolver::push_scope() {
    scope_ = std::make_shared<StaticScope>(scope_);
}

void SymbolResolver::pop_scope() {
    if (scope_->parent()) scope_ = scope_->parent();
}

std::shared_ptr<StaticScope> SymbolResolver::make_child_scope() {
    return std::make_shared<StaticScope>(scope_);
}


// Reporte de errores (sin lanzar excepción — el resolver continúa)
void SymbolResolver::report(const hulk::common::Span& span,
                            const std::string& error_id) {
    engine_.report(error_id,
                   hulk::common::DiagnosticLevel::Semantic,
                   hulk::common::Severity::Error,
                   span);
    has_errors_ = true;
}

void SymbolResolver::report(const hulk::common::Span& span,
                            const std::string& error_id,
                            const std::string& arg1) {
    engine_.report(error_id,
                   hulk::common::DiagnosticLevel::Semantic,
                   hulk::common::Severity::Error,
                   span,
                   arg1);
    has_errors_ = true;
}

void SymbolResolver::report(const hulk::common::Span& span,
                            const std::string& error_id,
                            const std::string& arg1,
                            const std::string& arg2) {
    engine_.report(error_id,
                   hulk::common::DiagnosticLevel::Semantic,
                   hulk::common::Severity::Error,
                   span,
                   arg1,
                   arg2);
    has_errors_ = true;
}

void SymbolResolver::report_raw(const hulk::common::Span& span,
                                const std::string& msg) {
    engine_.report_raw(hulk::common::DiagnosticLevel::Semantic,
                       hulk::common::Severity::Error,
                       span,
                       msg);
    has_errors_ = true;
}


// Helper: resolve() — análogo a eval() en el Evaluator
void SymbolResolver::resolve(Expr* node) {
    if (!node) return;
    node->accept(*this);
}

// PASE 1 — Registro de declaraciones en las tablas
void SymbolResolver::visit(FunctionDecl& n) {
    SemanticFuncInfo info;
    info.name                   = n.GetName();
    info.params                 = n.GetParams();
    info.return_type_annotation = n.GetReturnTypeAnnotation();
    info.body                   = n.GetBody();
    info.decl                   = &n;

    if (!tables_.register_func(std::move(info))) {
        // Función duplicada
        report_raw(n.span,
                   "Función '" + n.GetName() + "' ya fue declarada en este scope.");
    }
}

void SymbolResolver::visit(TypeDecl& n) {
    SemanticTypeInfo info;
    info.name        = n.GetName();
    info.ctor_params = n.GetCtorParams();
    info.parent_name = n.HasParent() ? n.GetParentName() : "";
    info.decl        = &n;

    for (auto& member : n.GetMembers()) {
        if (member.kind == TypeMember::Kind::Attribute) {
            auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
            SemanticAttrInfo ai;
            ai.name             = attr->GetName();
            ai.type_annotation  = attr->GetTypeAnnotation();
            ai.initializer      = attr->GetInitializer();
            info.attributes.push_back(std::move(ai));
        } else {
            auto* method = static_cast<TypeMemberMethod*>(member.node.get());
            SemanticMethodInfo mi;
            mi.name                   = method->GetName();
            mi.params                 = method->GetParams();
            mi.return_type_annotation = method->GetReturnTypeAnnotation();
            mi.body                   = method->GetBody();
            // Detectar override duplicado dentro del mismo tipo
            if (info.methods.count(mi.name)) {
                report_raw(method->span,
                           "Método '" + mi.name + "' duplicado en tipo '" + n.GetName() + "'.");
            } else {
                info.methods[mi.name] = std::move(mi);
            }
        }
    }

    if (!tables_.register_type(std::move(info))) {
        report_raw(n.span, "Tipo '" + n.GetName() + "' ya fue declarado.");
    }
}

// Estos nodos son procesados como parte de TypeDecl — no-op aquí
void SymbolResolver::visit(TypeMemberAttribute& n) { (void)n; }
void SymbolResolver::visit(TypeMemberMethod& n)    { (void)n; }
void SymbolResolver::visit(ProtocolDecl& n)        { (void)n; }


// PASE 3 — Chequeos globales sobre las tablas
void SymbolResolver::run_checks() {
    check_inheritance();
    check_methods();
    // check_arities() se hace inline durante el Pase 2 para tener el span correcto
}

void SymbolResolver::check_inheritance() {
    for (auto& [name, info] : tables_.all_types()) {
        // ¿Existe el padre?
        if (!info.parent_name.empty()) {
            if (!tables_.lookup_type(info.parent_name)) {
                hulk::common::Span span = info.decl ? info.decl->span : hulk::common::Span{};
                report(span, "SEM_UNDEFINED_PARENT", name, info.parent_name);
            }
        }
        // ¿Hay ciclo de herencia?
        if (tables_.has_inheritance_cycle(name)) {
            hulk::common::Span span = info.decl ? info.decl->span : hulk::common::Span{};
            report_raw(span, "Ciclo de herencia detectado en el tipo '" + name + "'.");
        }
    }
}

void SymbolResolver::check_methods() {
    for (auto& [type_name, type_info] : tables_.all_types()) {
        for (auto& [method_name, method_info] : type_info.methods) {
            // Si el método no está marcado como override, no hay nada que chequear.
            // Si está marcado como override, debe existir en el padre.
            if (method_info.is_override) {
                if (type_info.parent_name.empty()) {
                    hulk::common::Span span = method_info.body
                        ? method_info.body->span
                        : hulk::common::Span{};
                    report_raw(span,
                               "Método '" + method_name + "' en tipo '" + type_name +
                               "' marcado como override pero el tipo no tiene padre.");
                } else {
                    const SemanticMethodInfo* parent_method =
                        tables_.find_method(type_info.parent_name, method_name);
                    if (!parent_method) {
                        hulk::common::Span span = method_info.body
                            ? method_info.body->span
                            : hulk::common::Span{};
                        report_raw(span,
                                   "Método '" + method_name + "' en tipo '" + type_name +
                                   "' marcado como override pero no existe en el tipo padre '" +
                                   type_info.parent_name + "'.");
                    }
                }
            }
        }
    }
}

// PASE 2 — Resolución de referencias (ExprVisitor)

// --- Hojas (literales) — no resuelven nada ------------------------------

void SymbolResolver::visit(Number&)  {}
void SymbolResolver::visit(String&)  {}
void SymbolResolver::visit(Boolean&) {}

// --- Operaciones binarias — recorren subexpresiones ---------------------

void SymbolResolver::visit(ArithmeticBinOp& n) {
    resolve(n.GetLeft());
    resolve(n.GetRight());
}

void SymbolResolver::visit(LogicBinOp& n) {
    resolve(n.GetLeft());
    resolve(n.GetRight());
}

void SymbolResolver::visit(StringBinOp& n) {
    resolve(n.GetLeft());
    resolve(n.GetRight());
}

void SymbolResolver::visit(ArithmeticUnaryOp& n) {
    resolve(n.GetOperand());
}

void SymbolResolver::visit(LogicUnaryOp& n) {
    resolve(n.GetOperand());
}

// --- Builtins -----------------------------------------------------------

void SymbolResolver::visit(Print& n) {
    resolve(n.GetExpr());
}

void SymbolResolver::visit(BuiltinCall& n) {
    for (auto& arg : n.GetArgs())
        resolve(arg.get());
}

// --- Bloques ------------------------------------------------------------

void SymbolResolver::visit(ExprBlock& n) {
    push_scope();
    for (auto& expr : n.GetExprs())
        resolve(expr.get());
    pop_scope();
}

void SymbolResolver::visit(Group& n) {
    resolve(n.GetExpr());
}

// --- Variables ----------------------------------------------------------

void SymbolResolver::visit(VariableReference& n) {
    if (!scope_->contains(n.GetName())) {
        report(n.span, "SEM_UNDECLARED_VAR", n.GetName());
        resolution_map_[&n] = ResolutionResult{};  // Unresolved
        return;
    }
    VariableBinding* b = scope_->get_binding(n.GetName());
    if (b) {
        resolution_map_[&n] = ResolutionResult::from_binding(b);
    } else {
        const Param* p = scope_->get_param(n.GetName());
        if (p) resolution_map_[&n] = ResolutionResult::from_param(p);
    }
}

void SymbolResolver::visit(VariableBinding& n) {
    // El inicializador se resuelve en el scope EXTERIOR (no puede verse a sí mismo)
    resolve(n.GetInitializer());
    // El nombre se define en el scope actual DESPUÉS de resolver
    scope_->define_binding(n.GetName(), &n);
}

void SymbolResolver::visit(LetIn& n) {
    // Cada binding ve los anteriores (scopes encadenados dentro del let)
    auto prev_scope = scope_;
    scope_ = make_child_scope();

    for (auto& binding : n.GetBindings()) {
        // Inicializador se resuelve antes de definir el nombre
        resolve(binding->GetInitializer());
        scope_->define_binding(binding->GetName(), binding.get());
    }

    resolve(n.GetBody());
    scope_ = prev_scope;
}

// --- Asignaciones -------------------------------------------------------

void SymbolResolver::visit(DestructiveAssign& n) {
    if (!scope_->contains(n.GetName()))
        report(n.span, "SEM_UNDECLARED_VAR", n.GetName());
    resolve(n.GetValue());
}

void SymbolResolver::visit(DestructiveAssignMember& n) {
    resolve(n.GetObject());
    resolve(n.GetValue());
}

// --- Control de flujo ---------------------------------------------------

void SymbolResolver::visit(IfStmt& n) {
    resolve(n.GetCondition());
    resolve(n.GetThenBranch());
    for (auto& elif : n.GetElifBranches()) {
        resolve(elif.condition.get());
        resolve(elif.body.get());
    }
    if (Expr* else_b = n.GetElseBranch())
        resolve(else_b);
}

void SymbolResolver::visit(WhileStmt& n) {
    resolve(n.GetCondition());
    resolve(n.GetBody());
}

void SymbolResolver::visit(For& n) {
    resolve(n.GetIterable());
    push_scope();
    // La variable del for se introduce como Param sintético
    // (no tenemos un VariableBinding real, usamos un marcador nulo)
    scope_->define_binding(n.GetVarName(), nullptr);
    resolve(n.GetBody());
    pop_scope();
}

// --- Funciones ----------------------------------------------------------

void SymbolResolver::visit(FunctionCall& n) {
    const SemanticFuncInfo* info = tables_.lookup_func(n.GetName());
    if (!info) {
        report(n.span, "SEM_UNDECLARED_FUNC", n.GetName());
        resolution_map_[&n] = ResolutionResult{};
    } else {
        // Chequeo de aridad inline (Pase 3 implícito)
        if (n.GetArgs().size() != info->params.size()) {
            std::ostringstream oss;
            oss << "Función '" << n.GetName() << "' espera "
                << info->params.size() << " argumento(s) pero recibió "
                << n.GetArgs().size() << ".";
            report_raw(n.span, oss.str());
        }
        resolution_map_[&n] = ResolutionResult::from_func(info->decl);
    }
    // Resolver los argumentos independientemente del resultado
    for (auto& arg : n.GetArgs())
        resolve(arg.get());
}

void SymbolResolver::visit(Lambda& n) {
    // Lambdas de primera clase: resolver el cuerpo con sus parámetros en scope
    push_scope();
    for (const auto& p : n.GetParams())
        scope_->define_param(p.name, &p);
    resolve(n.GetBody());
    pop_scope();
}

// --- OOP ----------------------------------------------------------------

void SymbolResolver::visit(NewExpr& n) {
    const SemanticTypeInfo* info = tables_.lookup_type(n.GetTypeName());
    if (!info) {
        report(n.span, "SEM_UNDECLARED_TYPE", n.GetTypeName());
        resolution_map_[&n] = ResolutionResult{};
    } else {
        // Chequeo de aridad del constructor
        if (n.GetArgs().size() != info->ctor_params.size()) {
            std::ostringstream oss;
            oss << "Constructor de '" << n.GetTypeName() << "' espera "
                << info->ctor_params.size() << " argumento(s) pero recibió "
                << n.GetArgs().size() << ".";
            report_raw(n.span, oss.str());
        }
        resolution_map_[&n] = ResolutionResult::from_type(info);
    }
    for (auto& arg : n.GetArgs())
        resolve(arg.get());
}

void SymbolResolver::visit(MemberAccess& n) {
    resolve(n.GetObject());
}

void SymbolResolver::visit(MethodCall& n) {
    resolve(n.GetObject());
    for (auto& arg : n.GetArgs())
        resolve(arg.get());
    // Igual que MemberAccess — la resolución completa requiere tipos.
}

void SymbolResolver::visit(SelfRef& n) {
    // self solo es válido dentro de un tipo
    if (current_type_name_.empty()) {
        report_raw(n.span, "'self' solo puede usarse dentro de un método de tipo.");
    }
}

void SymbolResolver::visit(BaseCall& n) {
    if (current_type_name_.empty()) {
        report_raw(n.span, "'base()' solo puede usarse dentro de un método de tipo.");
    }
    for (auto& arg : n.GetArgs())
        resolve(arg.get());
}

void SymbolResolver::visit(IsExpr& n) {
    resolve(n.GetExpr());
    // Verificar que el tipo mencionado existe (si es un tipo de usuario)
    const std::string& t = n.GetTypeName();
    if (t != "Number" && t != "String" && t != "Boolean" && t != "Object") {
        if (!tables_.lookup_type(t))
            report(n.span, "SEM_UNDECLARED_TYPE", t);
    }
}

void SymbolResolver::visit(AsExpr& n) {
    resolve(n.GetExpr());
    const std::string& t = n.GetTypeName();
    if (t != "Number" && t != "String" && t != "Boolean" && t != "Object") {
        if (!tables_.lookup_type(t))
            report(n.span, "SEM_UNDECLARED_TYPE", t);
    }
}

// --- Vectores -----------------------------------------------------------

void SymbolResolver::visit(VectorLiteral& n) {
    for (auto& elem : n.GetElements())
        resolve(elem.get());
}

void SymbolResolver::visit(VectorIndex& n) {
    resolve(n.GetVector());
    resolve(n.GetIndex());
}

void SymbolResolver::visit(VectorGenerator& n) {
    resolve(n.GetIterable());
    push_scope();
    scope_->define_binding(n.GetVarName(), nullptr);
    resolve(n.GetBody());
    pop_scope();
}

}