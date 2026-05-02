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
#include "../ast/vectors/vectorLiteral.h"
#include "../ast/vectors/vectorIndex.h"
#include "../ast/vectors/vectorGenerator.h"
#include "../ast/types/typeDecl.h"
#include "../ast/types/typeMemberAttribute.h"
#include "../ast/types/typeMemberMethod.h"
#include "../ast/protocols/protocolDecl.h"
#include "../ast/functions/param.h"
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

// ─── Punto de entrada ──────────────────────────────────────────────────────

bool SymbolResolver::run(Program& program) {
    // Pase 1: registrar declaraciones
    for (auto& decl : program.GetDeclarations())
        decl->accept(*this);

    // Pase 3: chequeos estructurales
    run_checks();

    // Pase 2: resolver referencias en la expresión global
    if (Expr* global = program.GetGlobalExpr())
        resolve(global);

    // Pase 2 (cuerpos de funciones y tipos)
    for (auto& decl : program.GetDeclarations()) {
        if (auto* fd = dynamic_cast<FunctionDecl*>(decl.get())) {
            auto old_ctx = context_;
            context_ = ResolverContext::Function;
            push_scope();
            check_duplicate_params(fd->GetParams(), fd->span, "función '" + fd->GetName() + "'");
            for (const auto& p : fd->GetParams())
                scope_->define_param(p.name, &p);
            current_func_name_ = fd->GetName();
            resolve(fd->GetBody());
            current_func_name_.clear();
            pop_scope();
            context_ = old_ctx;

        } else if (auto* td = dynamic_cast<TypeDecl*>(decl.get())) {
            current_type_name_ = td->GetName();

            // Caso 12: parent args ven ctor params
            if (td->HasParent()) {
                std::vector<Param> parent_params = tables_.get_effective_constructor(td->GetParentName());
                bool has_explicit_parent_args = !td->GetParentArgs().empty();

                if (has_explicit_parent_args) {
                    // Hay args explícitos al padre: resolverlos y validar aridad
                    push_scope();
                    for (const auto& p : td->GetCtorParams())
                        scope_->define_param(p.name, &p);
                    for (auto& arg : td->GetParentArgs()) resolve(arg.get());
                    pop_scope();

                    if (td->GetParentArgs().size() != parent_params.size()) {
                        std::ostringstream oss;
                        oss << "Tipo '" << td->GetParentName() << "' espera "
                            << parent_params.size() << " argumento(s) pero recibió "
                            << td->GetParentArgs().size() << " en herencia de '" << td->GetName() << "'.";
                        report_raw(td->span, oss.str());
                    }
                } else if (td->HasExplicitConstructor() && !parent_params.empty()) {
                    // Tiene su propio ctor pero no pasa args al padre que los necesita
                    std::ostringstream oss;
                    oss << "Tipo '" << td->GetName() << "' declara constructor propio pero no pasa "
                        << "argumentos al padre '" << td->GetParentName() << "' (que espera "
                        << parent_params.size() << " argumento(s)).";
                    report_raw(td->span, oss.str());
                }
                // Si no hay ctor propio y no hay parent args → herencia transparente, válido
            }

            for (auto& member : td->GetMembers()) {
                if (member.kind == TypeMember::Kind::Attribute) {
                    auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
                    // Caso 2: atributos ven ctor params pero NO self
                    auto old_ctx = context_;
                    context_ = ResolverContext::TypeAttributeInit;
                    push_scope();
                    for (const auto& p : td->GetCtorParams())
                        scope_->define_param(p.name, &p);
                    // NO se define self aquí
                    resolve(attr->GetInitializer());
                    pop_scope();
                    context_ = old_ctx;
                } else {
                    auto* method = static_cast<TypeMemberMethod*>(member.node.get());
                    // Caso 3: métodos NO ven ctor params
                    auto old_ctx = context_;
                    context_ = ResolverContext::Method;

                    // Caso 13: crear SyntheticSymbol para self
                    auto self_sym = std::make_unique<SyntheticSymbol>();
                    self_sym->name      = "self";
                    self_sym->kind      = SyntheticKind::Self;
                    self_sym->type_name = current_type_name_;
                    SyntheticSymbol* self_ptr = self_sym.get();
                    synthetic_symbols_.push_back(std::move(self_sym));
                    current_self_symbol_ = self_ptr;

                    push_scope();
                    scope_->define_synthetic("self", self_ptr);
                    check_duplicate_params(method->GetParams(), method->span,
                                           "método '" + method->GetName() + "'");
                    for (const auto& p : method->GetParams())
                        scope_->define_param(p.name, &p);

                    auto old_method = current_method_name_;
                    current_method_name_ = method->GetName();
                    current_func_name_   = method->GetName();
                    resolve(method->GetBody());
                    current_method_name_ = old_method;
                    current_func_name_.clear();
                    pop_scope();
                    current_self_symbol_ = nullptr;
                    context_ = old_ctx;
                }
            }
            current_type_name_.clear();
        }
    }

    return !has_errors_;
}

// ─── Helpers de scope ──────────────────────────────────────────────────────

void SymbolResolver::push_scope() {
    scope_ = std::make_shared<StaticScope>(scope_);
}
void SymbolResolver::pop_scope() {
    if (scope_->parent()) scope_ = scope_->parent();
}
std::shared_ptr<StaticScope> SymbolResolver::make_child_scope() {
    return std::make_shared<StaticScope>(scope_);
}

// ─── Reporte de errores ────────────────────────────────────────────────────

void SymbolResolver::report(const hulk::common::Span& span, const std::string& id) {
    engine_.report(id, hulk::common::DiagnosticLevel::Semantic,
                   hulk::common::Severity::Error, span);
    has_errors_ = true;
}
void SymbolResolver::report(const hulk::common::Span& span, const std::string& id,
                            const std::string& a1) {
    engine_.report(id, hulk::common::DiagnosticLevel::Semantic,
                   hulk::common::Severity::Error, span, a1);
    has_errors_ = true;
}
void SymbolResolver::report(const hulk::common::Span& span, const std::string& id,
                            const std::string& a1, const std::string& a2) {
    engine_.report(id, hulk::common::DiagnosticLevel::Semantic,
                   hulk::common::Severity::Error, span, a1, a2);
    has_errors_ = true;
}
void SymbolResolver::report_raw(const hulk::common::Span& span, const std::string& msg) {
    engine_.report_raw(hulk::common::DiagnosticLevel::Semantic,
                       hulk::common::Severity::Error, span, msg);
    has_errors_ = true;
}

// ─── Helpers de validación ─────────────────────────────────────────────────

bool SymbolResolver::is_known_type_name(const std::string& name) const {
    if (name.empty()) return true;
    return tables_.lookup_type(name) != nullptr;
}

void SymbolResolver::check_type_annotation(const hulk::common::Span& span,
                                           const std::string& type_name) {
    if (!type_name.empty() && !is_known_type_name(type_name))
        report(span, "SEM_UNDECLARED_TYPE", type_name);
}

void SymbolResolver::check_duplicate_params(const std::vector<Param>& params,
                                            const hulk::common::Span& span,
                                            const std::string& owner) {
    std::unordered_set<std::string> seen;
    for (const auto& p : params) {
        if (!seen.insert(p.name).second)
            report_raw(span, "Parámetro '" + p.name + "' duplicado en " + owner + ".");
    }
}

const SemanticAttrInfo* SymbolResolver::find_attribute_in_ancestors(
        const std::string& type_name, const std::string& attr_name) const {
    std::string current = type_name;
    constexpr int MAX = 256;
    int depth = 0;
    while (!current.empty() && depth < MAX) {
        const auto* ti = tables_.lookup_type(current);
        if (!ti) break;
        for (const auto& a : ti->attributes)
            if (a.name == attr_name) return &a;
        current = ti->parent_name;
        ++depth;
    }
    return nullptr;
}

// ─── Pase 1 — Registro de declaraciones ───────────────────────────────────

void SymbolResolver::visit(FunctionDecl& n) {
    // Caso 8: validar anotaciones de tipo en parámetros y retorno
    for (const auto& p : n.GetParams())
        check_type_annotation(n.span, p.typeAnnotation);
    check_type_annotation(n.span, n.GetReturnTypeAnnotation());

    SemanticFuncInfo info;
    info.name                   = n.GetName();
    info.params                 = n.GetParams();
    info.return_type_annotation = n.GetReturnTypeAnnotation();
    info.body                   = n.GetBody();
    info.decl                   = &n;

    if (!tables_.register_func(std::move(info)))
        report_raw(n.span, "Función '" + n.GetName() + "' ya fue declarada en este scope.");
}

void SymbolResolver::visit(TypeDecl& n) {
    // Caso 9: rechazar redeclaración de tipos builtin
    if (const auto* existing = tables_.lookup_type(n.GetName())) {
        if (existing->is_builtin)
            report_raw(n.span, "No se puede redeclarar el tipo builtin '" + n.GetName() + "'.");
        else
            report_raw(n.span, "Tipo '" + n.GetName() + "' ya fue declarado.");
        return;
    }

    SemanticTypeInfo info;
    info.name        = n.GetName();
    info.ctor_params = n.GetCtorParams();
    info.parent_name = n.HasParent() ? n.GetParentName() : "Object";
    info.decl                = &n;
    info.defines_constructor = n.HasExplicitConstructor();

    // Caso 12: parámetros duplicados en constructor
    check_duplicate_params(n.GetCtorParams(), n.span,
                           "constructor de '" + n.GetName() + "'");

    // Caso 8: anotaciones de tipo en ctor params
    for (const auto& p : n.GetCtorParams())
        check_type_annotation(n.span, p.typeAnnotation);

    // Caso 10: detectar atributos duplicados; caso 8: anotar tipos
    std::unordered_set<std::string> attr_names;
    std::unordered_set<std::string> member_names;

    for (auto& member : n.GetMembers()) {
        if (member.kind == TypeMember::Kind::Attribute) {
            auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
            SemanticAttrInfo ai;
            ai.name            = attr->GetName();
            ai.type_annotation = attr->GetTypeAnnotation();
            ai.initializer     = attr->GetInitializer();

            // Caso 10: duplicado
            if (!attr_names.insert(ai.name).second)
                report_raw(attr->span, "Atributo '" + ai.name +
                           "' duplicado en tipo '" + n.GetName() + "'.");
            member_names.insert(ai.name);

            // Caso 8
            check_type_annotation(attr->span, ai.type_annotation);
            info.attributes.push_back(std::move(ai));
        } else {
            auto* method = static_cast<TypeMemberMethod*>(member.node.get());
            SemanticMethodInfo mi;
            mi.name                   = method->GetName();
            mi.params                 = method->GetParams();
            mi.return_type_annotation = method->GetReturnTypeAnnotation();
            mi.body                   = method->GetBody();
            // Guardar punteros a los params del AST original (para param_types_ lookup)
            for (const auto& p : method->GetParams())
                mi.ast_params.push_back(&p);

            if (info.methods.count(mi.name)) {
                report_raw(method->span, "Método '" + mi.name +
                           "' duplicado en tipo '" + n.GetName() + "'.");
            } else {
                // Caso 12: params duplicados en método
                check_duplicate_params(mi.params, method->span,
                                       "método '" + mi.name + "'");
                // Caso 8: anotaciones
                for (const auto& p : mi.params)
                    check_type_annotation(method->span, p.typeAnnotation);
                check_type_annotation(method->span, mi.return_type_annotation);
                member_names.insert(mi.name);
                info.methods[mi.name] = std::move(mi);
            }
        }
    }

    tables_.register_type(std::move(info));
}

void SymbolResolver::visit(TypeMemberAttribute& n) { (void)n; }
void SymbolResolver::visit(TypeMemberMethod& n)    { (void)n; }
void SymbolResolver::visit(ProtocolDecl& n)        { (void)n; }

// ─── Pase 3 — Chequeos globales ───────────────────────────────────────────

void SymbolResolver::run_checks() {
    check_inheritance();
    check_methods();
}

void SymbolResolver::check_inheritance() {
    for (auto& [name, info] : tables_.all_types()) {
        if (info.is_builtin) continue;

        if (!info.parent_name.empty()) {
            const auto* parent = tables_.lookup_type(info.parent_name);
            if (!parent) {
                hulk::common::Span sp = info.decl ? info.decl->span : hulk::common::Span{};
                report(sp, "SEM_UNDEFINED_PARENT", name, info.parent_name);
            } else if (parent->is_builtin &&
                       (info.parent_name == "Number" ||
                        info.parent_name == "String"  ||
                        info.parent_name == "Boolean")) {
                // Caso 9: no heredar de Number/String/Boolean
                hulk::common::Span sp = info.decl ? info.decl->span : hulk::common::Span{};
                report_raw(sp, "No se puede heredar del tipo builtin '" +
                           info.parent_name + "'.");
            }
        }

        if (tables_.has_inheritance_cycle(name)) {
            hulk::common::Span sp = info.decl ? info.decl->span : hulk::common::Span{};
            report_raw(sp, "Ciclo de herencia detectado en el tipo '" + name + "'.");
        }
    }
}

// Caso 5: validar override — aridad y tipos de parámetros/retorno
void SymbolResolver::check_methods() {
    for (auto& [type_name, type_info] : tables_.all_types()) {
        if (type_info.is_builtin || type_info.parent_name.empty()) continue;

        for (auto& [method_name, method_info] : type_info.methods) {
            const SemanticMethodInfo* parent_method =
                tables_.find_method(type_info.parent_name, method_name);
            if (!parent_method) continue;  // método nuevo, no override

            hulk::common::Span sp = method_info.body
                ? method_info.body->span : hulk::common::Span{};

            // Aridad
            if (method_info.params.size() != parent_method->params.size()) {
                report_raw(sp, "Método '" + method_name + "' en tipo '" + type_name +
                           "' redefine un método heredado con distinta cantidad de parámetros.");
                continue;
            }

            // Tipos de parámetros
            for (size_t i = 0; i < method_info.params.size(); ++i) {
                const auto& ca = method_info.params[i].typeAnnotation;
                const auto& pa = parent_method->params[i].typeAnnotation;
                if (!ca.empty() && !pa.empty() && ca != pa)
                    report_raw(sp, "Método '" + method_name + "' redefine el parámetro " +
                               std::to_string(i+1) + " con tipo distinto ('" + ca +
                               "' vs '" + pa + "').");
            }

            // Tipo de retorno
            const auto& cr = method_info.return_type_annotation;
            const auto& pr = parent_method->return_type_annotation;
            if (!cr.empty() && !pr.empty() && cr != pr)
                report_raw(sp, "Método '" + method_name + "' redefine el retorno con tipo distinto ('" +
                           cr + "' vs '" + pr + "').");
        }
    }
}

void SymbolResolver::check_arities() {}  // inline en Pase 2

// ─── Helper resolve ────────────────────────────────────────────────────────

void SymbolResolver::resolve(Expr* node) {
    if (!node) return;
    node->accept(*this);
}

// ═══════════════════════════════════════════════════════════════════════════
// PASE 2 — Resolución de referencias (ExprVisitor)
// ═══════════════════════════════════════════════════════════════════════════

// ─── Literales ─────────────────────────────────────────────────────────────
void SymbolResolver::visit(Number&)  {}
void SymbolResolver::visit(String&)  {}
void SymbolResolver::visit(Boolean&) {}

// ─── Operaciones ───────────────────────────────────────────────────────────
void SymbolResolver::visit(ArithmeticBinOp& n) { resolve(n.GetLeft()); resolve(n.GetRight()); }
void SymbolResolver::visit(LogicBinOp& n)      { resolve(n.GetLeft()); resolve(n.GetRight()); }
void SymbolResolver::visit(StringBinOp& n)     { resolve(n.GetLeft()); resolve(n.GetRight()); }
void SymbolResolver::visit(ArithmeticUnaryOp& n){ resolve(n.GetOperand()); }
void SymbolResolver::visit(LogicUnaryOp& n)    { resolve(n.GetOperand()); }

// ─── Builtins ──────────────────────────────────────────────────────────────
void SymbolResolver::visit(Print& n) { resolve(n.GetExpr()); }

// Caso 11: validar nombre y aridad de BuiltinCall
// BuiltinCall usa un enum GetFunc(), así que mapeamos al nombre string.
static std::string builtin_func_name(Hulk::BuiltinFunc f) {
    switch (f) {
        case Hulk::BuiltinFunc::Sqrt:  return "sqrt";
        case Hulk::BuiltinFunc::Sin:   return "sin";
        case Hulk::BuiltinFunc::Cos:   return "cos";
        case Hulk::BuiltinFunc::Exp:   return "exp";
        case Hulk::BuiltinFunc::Log:   return "log";
        case Hulk::BuiltinFunc::Rand:  return "rand";
        case Hulk::BuiltinFunc::Range: return "range";
    }
    return "unknown";
}

void SymbolResolver::visit(BuiltinCall& n) {
    for (auto& arg : n.GetArgs())
        resolve(arg.get());

    const std::string fname = builtin_func_name(n.GetFunc());
    const BuiltinFuncInfo* bf = tables_.lookup_builtin_func(fname);
    if (!bf) {
        report_raw(n.span, "Función builtin desconocida: '" + fname + "'.");
        resolution_map_[&n] = ResolutionResult{};
        return;
    }
    if (bf->arity >= 0 && static_cast<int>(n.GetArgs().size()) != bf->arity) {
        std::ostringstream oss;
        oss << "Función builtin '" << fname << "' espera "
            << bf->arity << " argumento(s) pero recibió "
            << n.GetArgs().size() << ".";
        report_raw(n.span, oss.str());
    }
    resolution_map_[&n] = ResolutionResult::from_builtin_func(bf);
}

// ─── Bloques ───────────────────────────────────────────────────────────────
void SymbolResolver::visit(ExprBlock& n) {
    push_scope();
    for (auto& e : n.GetExprs()) resolve(e.get());
    pop_scope();
}
void SymbolResolver::visit(Group& n) { resolve(n.GetExpr()); }

// ─── Variables ─────────────────────────────────────────────────────────────

void SymbolResolver::visit(VariableReference& n) {
    // Caso 11: constantes builtin (PI, E)
    if (const BuiltinConstInfo* bc = tables_.lookup_builtin_const(n.GetName())) {
        resolution_map_[&n] = ResolutionResult::from_builtin_const(bc);
        return;
    }

    auto res = scope_->lookup(n.GetName());
    if (!res.is_resolved()) {
        report(n.span, "SEM_UNDECLARED_VAR", n.GetName());
        resolution_map_[&n] = ResolutionResult{};
        return;
    }

    if (res.binding) resolution_map_[&n] = ResolutionResult::from_binding(res.binding);
    else if (res.param) resolution_map_[&n] = ResolutionResult::from_param(res.param);
    else if (res.synthetic) resolution_map_[&n] = ResolutionResult::from_synthetic(res.synthetic);
}

void SymbolResolver::visit(VariableBinding& n) {
    // Caso 8: anotación de tipo en binding let
    check_type_annotation(n.span, n.GetTypeAnnotation());
    resolve(n.GetInitializer());
    scope_->define_binding(n.GetName(), &n);
}

void SymbolResolver::visit(LetIn& n) {
    auto prev = scope_;
    scope_ = make_child_scope();
    for (auto& binding : n.GetBindings()) {
        check_type_annotation(binding->span, binding->GetTypeAnnotation());
        resolve(binding->GetInitializer());
        scope_->define_binding(binding->GetName(), binding.get());
    }
    resolve(n.GetBody());
    scope_ = prev;
}

// ─── Asignaciones ──────────────────────────────────────────────────────────

void SymbolResolver::visit(DestructiveAssign& n) {
    if (const BuiltinConstInfo* bc = tables_.lookup_builtin_const(n.GetName())) {
        report_raw(n.span, "No se puede asignar a la constante builtin '" + n.GetName() + "'.");
        resolve(n.GetValue());
        return;
    }

    auto res = scope_->lookup(n.GetName());
    if (!res.is_resolved()) {
        report(n.span, "SEM_UNDECLARED_VAR", n.GetName());
    } else {
        if (res.binding) resolution_map_[&n] = ResolutionResult::from_binding(res.binding);
        else if (res.param) resolution_map_[&n] = ResolutionResult::from_param(res.param);
        else if (res.synthetic) {
            report_raw(n.span, "No se puede asignar al símbolo '" + n.GetName() + "'.");
        }
    }
    resolve(n.GetValue());
}

void SymbolResolver::visit(DestructiveAssignMember& n) {
    resolve(n.GetObject());
    resolve(n.GetValue());
}

// ─── Control de flujo ──────────────────────────────────────────────────────

void SymbolResolver::visit(IfStmt& n) {
    resolve(n.GetCondition());
    resolve(n.GetThenBranch());
    for (auto& elif : n.GetElifBranches()) {
        resolve(elif.condition.get());
        resolve(elif.body.get());
    }
    if (Expr* eb = n.GetElseBranch()) resolve(eb);
}

void SymbolResolver::visit(WhileStmt& n) {
    resolve(n.GetCondition());
    resolve(n.GetBody());
}

// Caso 7: variable sintética del for
void SymbolResolver::visit(For& n) {
    resolve(n.GetIterable());
    push_scope();

    auto syn = std::make_unique<SyntheticSymbol>();
    syn->name      = n.GetVarName();
    syn->kind      = SyntheticKind::ForVariable;
    syn->type_name = "";
    SyntheticSymbol* ptr = syn.get();
    synthetic_symbols_.push_back(std::move(syn));
    scope_->define_synthetic(n.GetVarName(), ptr);

    resolve(n.GetBody());
    pop_scope();
}

// ─── Funciones ─────────────────────────────────────────────────────────────

void SymbolResolver::visit(FunctionCall& n) {
    for (auto& arg : n.GetArgs()) resolve(arg.get());

    if (n.GetName() == "base") {
        if (context_ != ResolverContext::Method) {
            report_raw(n.span, "'base()' solo puede usarse dentro de métodos de tipo.");
            resolution_map_[&n] = ResolutionResult{};
            return;
        }
        const SemanticTypeInfo* type_info = tables_.lookup_type(current_type_name_);
        if (!type_info || type_info->parent_name.empty()) {
            report_raw(n.span, "'base()' solo puede usarse en tipos con herencia.");
            resolution_map_[&n] = ResolutionResult{};
            return;
        }
        const SemanticMethodInfo* parent_method = tables_.find_method(type_info->parent_name, current_func_name_);
        if (!parent_method) {
            report_raw(n.span, "El método '" + current_func_name_ + "' no existe en el padre '" + type_info->parent_name + "'.");
            resolution_map_[&n] = ResolutionResult{};
            return;
        }
        resolution_map_[&n] = ResolutionResult::from_method(parent_method);
        return;
    }

    const SemanticFuncInfo* info = tables_.lookup_func(n.GetName());
    if (!info) {
        // Fallback: algunas funciones builtin (como 'range') se parsean como FunctionCall
        const BuiltinFuncInfo* builtin = tables_.lookup_builtin_func(n.GetName());
        if (builtin) {
            if (builtin->arity >= 0 && static_cast<int>(n.GetArgs().size()) != builtin->arity) {
                std::ostringstream oss;
                oss << "Función builtin '" << n.GetName() << "' espera "
                    << builtin->arity << " argumento(s) pero recibió "
                    << n.GetArgs().size() << ".";
                report_raw(n.span, oss.str());
            }
            resolution_map_[&n] = ResolutionResult::from_builtin_func(builtin);
        } else {
            report(n.span, "SEM_UNDECLARED_FUNC", n.GetName());
            resolution_map_[&n] = ResolutionResult{};
        }
    } else {
        if (n.GetArgs().size() != info->params.size()) {
            std::ostringstream oss;
            oss << "Función '" << n.GetName() << "' espera "
                << info->params.size() << " argumento(s) pero recibió "
                << n.GetArgs().size() << ".";
            report_raw(n.span, oss.str());
        }
        resolution_map_[&n] = ResolutionResult::from_func(info->decl);
    }
}

void SymbolResolver::visit(Lambda& n) {
    push_scope();
    for (const auto& p : n.GetParams())
        scope_->define_param(p.name, &p);
    resolve(n.GetBody());
    pop_scope();
}

// ─── OOP ───────────────────────────────────────────────────────────────────

void SymbolResolver::visit(NewExpr& n) {
    const SemanticTypeInfo* info = tables_.lookup_type(n.GetTypeName());
    if (!info) {
        report(n.span, "SEM_UNDECLARED_TYPE", n.GetTypeName());
        resolution_map_[&n] = ResolutionResult{};
    } else {
        std::vector<Param> params = tables_.get_effective_constructor(n.GetTypeName());
        if (n.GetArgs().size() != params.size()) {
            std::ostringstream oss;
            oss << "Constructor de '" << n.GetTypeName() << "' espera "
                << params.size() << " argumento(s) pero recibió "
                << n.GetArgs().size() << ".";
            report_raw(n.span, oss.str());
        }
        resolution_map_[&n] = ResolutionResult::from_type(info);
    }
    for (auto& arg : n.GetArgs()) resolve(arg.get());
}

// Caso 6: resolver MemberAccess sobre self
void SymbolResolver::visit(MemberAccess& n) {
    resolve(n.GetObject());
    auto* var_ref = dynamic_cast<VariableReference*>(n.GetObject());
    auto* self_ref = dynamic_cast<SelfRef*>(n.GetObject());
    
    if ((var_ref && var_ref->GetName() == "self") || self_ref) {
        if (current_type_name_.empty()) {
            report_raw(n.span, "'self' no es válido en este contexto.");
            resolution_map_[&n] = ResolutionResult{};
            return;
        }
        const SemanticAttrInfo* attr = find_attribute_in_ancestors(current_type_name_, n.GetMemberName());
        if (!attr) {
            report_raw(n.span, "Atributo '" + n.GetMemberName() +
                       "' no existe en tipo '" + current_type_name_ + "'.");
            resolution_map_[&n] = ResolutionResult{};
        } else {
            resolution_map_[&n] = ResolutionResult::from_attr(attr);
        }
    } else {
        report_raw(n.span, "Los atributos son privados. Solo se pueden acceder mediante 'self'.");
        resolution_map_[&n] = ResolutionResult{};
    }
}

// Caso 6: resolver MethodCall sobre self
void SymbolResolver::visit(MethodCall& n) {
    resolve(n.GetObject());
    for (auto& arg : n.GetArgs()) resolve(arg.get());

    auto* var_ref = dynamic_cast<VariableReference*>(n.GetObject());
    if (var_ref && var_ref->GetName() == "self" && !current_type_name_.empty()) {
        const SemanticMethodInfo* method =
            tables_.find_method(current_type_name_, n.GetMethodName());
        if (!method) {
            report_raw(n.span, "Método '" + n.GetMethodName() +
                       "' no existe en tipo '" + current_type_name_ + "'.");
            resolution_map_[&n] = ResolutionResult{};
        } else {
            if (n.GetArgs().size() != method->params.size()) {
                std::ostringstream oss;
                oss << "Método '" << n.GetMethodName() << "' espera "
                    << method->params.size() << " argumento(s) pero recibió "
                    << n.GetArgs().size() << ".";
                report_raw(n.span, oss.str());
            }
            resolution_map_[&n] = ResolutionResult::from_method(method);
        }
    }
}

// Caso 2 y 13: self solo en métodos, y se anota en resolution_map_
void SymbolResolver::visit(SelfRef& n) {
    if (context_ != ResolverContext::Method) {
        report_raw(n.span, "'self' solo puede usarse dentro de métodos de tipo.");
        resolution_map_[&n] = ResolutionResult{};
        return;
    }
    if (current_self_symbol_)
        resolution_map_[&n] = ResolutionResult::from_synthetic(current_self_symbol_);
}

// Caso 4: base() solo en métodos, con padre y método base válidos
void SymbolResolver::visit(BaseCall& n) {
    for (auto& arg : n.GetArgs()) resolve(arg.get());

    if (context_ != ResolverContext::Method) {
        report_raw(n.span, "'base()' solo puede usarse dentro de métodos de tipo.");
        return;
    }

    const SemanticTypeInfo* type = tables_.lookup_type(current_type_name_);
    if (!type || type->parent_name.empty()) {
        report_raw(n.span, "'base()' usado en tipo sin padre.");
        return;
    }

    const SemanticMethodInfo* parent_method =
        tables_.find_method(type->parent_name, current_method_name_);
    if (!parent_method) {
        report_raw(n.span, "'base()' no tiene método base para '" +
                   current_method_name_ + "' en el padre.");
        return;
    }

    // Registrar resolución para que el inferencer y checker puedan usarla
    resolution_map_[&n] = ResolutionResult::from_method(parent_method);

    if (n.GetArgs().size() != parent_method->params.size()) {
        std::ostringstream oss;
        oss << "Aridad incorrecta en base(): espera "
            << parent_method->params.size() << " pero recibió "
            << n.GetArgs().size() << ".";
        report_raw(n.span, oss.str());
    }
}

void SymbolResolver::visit(IsExpr& n) {
    resolve(n.GetExpr());
    check_type_annotation(n.span, n.GetTypeName());
}

void SymbolResolver::visit(AsExpr& n) {
    resolve(n.GetExpr());
    check_type_annotation(n.span, n.GetTypeName());
}

// ─── Vectores ──────────────────────────────────────────────────────────────

void SymbolResolver::visit(VectorLiteral& n) {
    for (auto& e : n.GetElements()) resolve(e.get());
}

void SymbolResolver::visit(VectorIndex& n) {
    resolve(n.GetVector());
    resolve(n.GetIndex());
}

// Caso 7: variable sintética del VectorGenerator
void SymbolResolver::visit(VectorGenerator& n) {
    resolve(n.GetIterable());
    push_scope();

    auto syn = std::make_unique<SyntheticSymbol>();
    syn->name      = n.GetVarName();
    syn->kind      = SyntheticKind::VectorGeneratorVariable;
    syn->type_name = "";
    SyntheticSymbol* ptr = syn.get();
    synthetic_symbols_.push_back(std::move(syn));
    scope_->define_synthetic(n.GetVarName(), ptr);

    resolve(n.GetBody());
    pop_scope();
}

} // namespace Hulk