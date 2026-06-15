#include "symbol_resolver.h"
#include "../inference/hulk_type.h"

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
#include "../ast/protocols/protocolDecl.h"
#include "../ast/functions/param.h"
#include "../common/diagnosticRepository.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <vector>

namespace Hulk {

namespace {

bool is_typed_iterable_annotation(const std::string& name) {
    return !name.empty() && name.back() == '*';
}

std::string typed_iterable_element(const std::string& name) {
    if (!is_typed_iterable_annotation(name)) return "";
    return name.substr(0, name.size() - 1);
}

bool is_valid_typed_iterable_element_name(const std::string& name) {
    return !name.empty() && name != "auto" && name != "_" &&
           name != "Void";
}

}

SymbolResolver::SymbolResolver(SemanticTables& tables,
                               hulk::common::DiagnosticEngine& engine)
    : tables_(tables),
      engine_(engine),
      scope_(std::make_shared<StaticScope>())
{}


bool SymbolResolver::run(Program& program) {
    // Pase 1: registrar declaraciones
    for (auto& decl : program.GetDeclarations())
        decl->accept(*this);

    // Pase 1.5: validar anotaciones
    for (auto& decl : program.GetDeclarations()) {
        if (auto* fd = dynamic_cast<FunctionDecl*>(decl.get())) {
            for (const auto& p : fd->GetParams())
                check_type_annotation(fd->span, p.typeAnnotation);
            check_type_annotation(fd->span, fd->GetReturnTypeAnnotation());
        } else if (auto* td = dynamic_cast<TypeDecl*>(decl.get())) {
            for (const auto& p : td->GetCtorParams())
                check_type_annotation(td->span, p.typeAnnotation);
            for (auto& member : td->GetMembers()) {
                if (member.kind == TypeMember::Kind::Attribute) {
                    auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
                    check_type_annotation(attr->span, attr->GetTypeAnnotation());
                } else if (member.kind == TypeMember::Kind::Method) {
                    auto* method = static_cast<TypeMemberMethod*>(member.node.get());
                    for (const auto& p : method->GetParams())
                        check_type_annotation(method->span, p.typeAnnotation);
                    check_type_annotation(method->span, method->GetReturnTypeAnnotation());
                }
            }
        } else if (auto* pd = dynamic_cast<ProtocolDecl*>(decl.get())) {
            for (const auto& sig : pd->GetMethodSigs()) {
                if (sig.returnType.empty()) {
                    report_raw(pd->span, "Método '" + sig.name +
                               "' de protocolo '" + pd->GetName() +
                               "' debe declarar tipo de retorno.");
                }
                check_type_annotation(pd->span, sig.returnType);
                for (const auto& p : sig.params) {
                    if (!p.HasTypeAnnotation()) {
                        report_raw(pd->span, "Parámetro '" + p.name +
                                   "' del método de protocolo '" + sig.name +
                                   "' debe declarar tipo.");
                    }
                    check_type_annotation(pd->span, p.typeAnnotation);
                }
            }
        }
    }

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

            if (td->HasParent()) {
                std::vector<Param> parent_params = tables_.get_effective_constructor(td->GetParentName());
                bool has_explicit_parent_args = !td->GetParentArgs().empty();

                if (has_explicit_parent_args) {
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
                    std::ostringstream oss;
                    oss << "Tipo '" << td->GetName() << "' declara constructor propio pero no pasa "
                        << "argumentos al padre '" << td->GetParentName() << "' (que espera "
                        << parent_params.size() << " argumento(s)).";
                    report_raw(td->span, oss.str());
                }
            }

            for (auto& member : td->GetMembers()) {
                if (member.kind == TypeMember::Kind::Attribute) {
                    auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
                    auto old_ctx = context_;
                    context_ = ResolverContext::TypeAttributeInit;
                    push_scope();
                    for (const auto& p : td->GetCtorParams())
                        scope_->define_param(p.name, &p);
                    resolve(attr->GetInitializer());
                    pop_scope();
                    context_ = old_ctx;
                } else {
                    auto* method = static_cast<TypeMemberMethod*>(member.node.get());
                    auto old_ctx = context_;
                    context_ = ResolverContext::Method;

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

//  Helpers de scope 

void SymbolResolver::push_scope() {
    scope_ = std::make_shared<StaticScope>(scope_);
}
void SymbolResolver::pop_scope() {
    if (scope_->parent()) scope_ = scope_->parent();
}
std::shared_ptr<StaticScope> SymbolResolver::make_child_scope() {
    return std::make_shared<StaticScope>(scope_);
}

//  Reporte de errores 

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

//  Helpers de validación 

bool SymbolResolver::is_known_type_name(const std::string& name) const {
    if (name.empty()) return true;
    if (is_typed_iterable_annotation(name)) {
        const std::string element = typed_iterable_element(name);
        return is_valid_typed_iterable_element_name(element) &&
               is_known_type_name(element);
    }
    if (name == "auto" || name == "_") return true;
    return tables_.lookup_type(name) != nullptr || tables_.lookup_protocol(name) != nullptr;
}

void SymbolResolver::check_type_annotation(const hulk::common::Span& span,
                                           const std::string& type_name) {
    if (type_name.empty()) return;

    if (is_typed_iterable_annotation(type_name)) {
        const std::string element = typed_iterable_element(type_name);
        if (!is_known_type_name(type_name)) {
            report(span, "SEM_UNDECLARED_TYPE", type_name);
            return;
        }
        if (!tables_.ensure_typed_iterable_protocol(element)) {
            report(span, "SEM_UNDECLARED_TYPE", type_name);
        }
        return;
    }

    if (!is_known_type_name(type_name))
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

//  Pase 1 — Registro de declaraciones 

void SymbolResolver::visit(FunctionDecl& n) {
    // Validaciones movidas a Pase 1.5
    if (tables_.lookup_builtin_func(n.GetName()) || tables_.lookup_protocol(n.GetName()) ||
        tables_.lookup_type(n.GetName())) {
        report_raw(n.span, "No se puede redeclarar el nombre reservado '" + n.GetName() + "'.");
        return;
    }

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
    if (const auto* protocol = tables_.lookup_protocol(n.GetName())) {
        if (protocol->is_builtin)
            report_raw(n.span, "No se puede redeclarar el protocolo builtin '" + n.GetName() + "'.");
        else
            report_raw(n.span, "El nombre '" + n.GetName() + "' ya fue declarado como protocolo.");
        return;
    }
    if (tables_.lookup_builtin_func(n.GetName())) {
        report_raw(n.span, "No se puede redeclarar la función builtin '" + n.GetName() + "'.");
        return;
    }

    SemanticTypeInfo info;
    info.name        = n.GetName();
    info.ctor_params = n.GetCtorParams();
    info.parent_name = n.HasParent() ? n.GetParentName() : "Object";
    info.decl                = &n;
    info.defines_constructor = n.HasExplicitConstructor();

    // parámetros duplicados en constructor
    check_duplicate_params(n.GetCtorParams(), n.span,
                           "constructor de '" + n.GetName() + "'");

    std::unordered_set<std::string> attr_names;
    std::unordered_set<std::string> member_names;

    for (auto& member : n.GetMembers()) {
        if (member.kind == TypeMember::Kind::Attribute) {
            auto* attr = static_cast<TypeMemberAttribute*>(member.node.get());
            SemanticAttrInfo ai;
            ai.name            = attr->GetName();
            ai.type_annotation = attr->GetTypeAnnotation();
            ai.initializer     = attr->GetInitializer();

            if (!attr_names.insert(ai.name).second)
                report_raw(attr->span, "Atributo '" + ai.name +
                           "' duplicado en tipo '" + n.GetName() + "'.");
            member_names.insert(ai.name);

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
                //  params duplicados en método
                check_duplicate_params(mi.params, method->span,
                                       "método '" + mi.name + "'");
                member_names.insert(mi.name);
                info.methods[mi.name] = std::move(mi);
            }
        }
    }

    tables_.register_type(std::move(info));
}

void SymbolResolver::visit(TypeMemberAttribute& n) { (void)n; }
void SymbolResolver::visit(TypeMemberMethod& n)    { (void)n; }
void SymbolResolver::visit(ProtocolDecl& n) {
    if (tables_.lookup_type(n.GetName())) {
        report_raw(n.span, "El nombre '" + n.GetName() + "' ya fue declarado como tipo.");
        return;
    }
    if (tables_.lookup_builtin_func(n.GetName())) {
        report_raw(n.span, "No se puede redeclarar la función builtin '" + n.GetName() + "'.");
        return;
    }
    if (tables_.lookup_func(n.GetName())) {
        report_raw(n.span, "El nombre '" + n.GetName() + "' ya fue declarado como función.");
        return;
    }

    SemanticProtocolInfo info;
    info.name = n.GetName();
    info.parent_name = n.HasParent() ? n.GetParentName() : "";
    info.decl = &n;

    for (const auto& sig : n.GetMethodSigs()) {
        if (info.methods.count(sig.name)) {
            report_raw(n.span, "Método '" + sig.name +
                       "' duplicado en protocolo '" + n.GetName() + "'.");
            continue;
        }
        check_duplicate_params(sig.params, n.span,
                               "método de protocolo '" + sig.name + "'");
        info.methods.emplace(sig.name,
                             SemanticProtocolMethodInfo{
                                 sig.name,
                                 sig.params,
                                 sig.returnType,
                             });
    }

    if (!tables_.register_protocol(std::move(info)))
        report_raw(n.span, "Protocolo '" + n.GetName() + "' ya fue declarado.");
}

//  Chequeos globales 

void SymbolResolver::run_checks() {
    check_inheritance();
    check_protocols();
    check_methods();
}

void SymbolResolver::check_inheritance() {
    std::vector<std::string> names;
    for (auto& [n, _] : tables_.all_types()) names.push_back(n);
    std::sort(names.begin(), names.end());

    std::unordered_set<std::string> cycle_reported;

    for (const auto& name : names) {
        const auto& info = tables_.all_types().at(name);
        if (info.is_builtin) continue;

        if (!info.parent_name.empty()) {
            const auto* parent = tables_.lookup_type(info.parent_name);
            if (!parent) {
                hulk::common::Span sp = info.decl ? info.decl->span : hulk::common::Span{};
                report(sp, "BIND_PARENT_NOT_FOUND", info.parent_name, name);
            } else if (parent->is_builtin &&
                       (info.parent_name == "Number" ||
                        info.parent_name == "String"  ||
                        info.parent_name == "Boolean" ||
                        info.parent_name == "Range")) {
                hulk::common::Span sp = info.decl ? info.decl->span : hulk::common::Span{};
                report_raw(sp, "No se puede heredar del tipo builtin '" +
                           info.parent_name + "'.");
            }
        }

        if (cycle_reported.count(name) == 0 && tables_.has_inheritance_cycle(name)) {
            hulk::common::Span sp = info.decl ? info.decl->span : hulk::common::Span{};
            report(sp, "BIND_INHERIT_CYCLE", name, info.parent_name);
            std::string cur = name;
            while (!cur.empty()) {
                if (!cycle_reported.insert(cur).second) break;
                auto it = tables_.all_types().find(cur);
                if (it == tables_.all_types().end()) break;
                cur = it->second.parent_name;
            }
        }
    }
}

void SymbolResolver::check_protocols() {
    for (const auto& [name, info] : tables_.all_protocols()) {
        if (info.is_builtin) continue;
        hulk::common::Span sp = info.decl ? info.decl->span : hulk::common::Span{};

        if (!info.parent_name.empty()) {
            if (!tables_.lookup_protocol(info.parent_name)) {
                report_raw(sp, "Protocolo padre '" + info.parent_name +
                           "' no existe para protocolo '" + name + "'.");
            }
        }

        if (tables_.has_protocol_cycle(name)) {
            report_raw(sp, "Ciclo de herencia detectado en el protocolo '" + name + "'.");
            continue;
        }

        if (!info.parent_name.empty() &&
            tables_.lookup_protocol(info.parent_name) &&
            !tables_.protocol_conforms_to_protocol(name, info.parent_name)) {
            report_raw(sp, "Protocolo '" + name +
                       "' redefine firmas heredadas incompatibles con '" +
                       info.parent_name + "'.");
        }
    }
}

// validar override — aridad y tipos de parámetros/retorno
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
            if (!cr.empty() && !pr.empty() && cr != pr) {
                // Verificar si cr conforma a pr en lugar de exigir igualdad estricta
                HulkType child_ret = HulkType::make_unknown();
                if (cr == "Number") child_ret = HulkType::make_number();
                else if (cr == "String") child_ret = HulkType::make_string();
                else if (cr == "Boolean") child_ret = HulkType::make_boolean();
                else child_ret = HulkType::make_object(cr);

                HulkType parent_ret = HulkType::make_unknown();
                if (pr == "Number") parent_ret = HulkType::make_number();
                else if (pr == "String") parent_ret = HulkType::make_string();
                else if (pr == "Boolean") parent_ret = HulkType::make_boolean();
                else parent_ret = HulkType::make_object(pr);

                if (!child_ret.conforms_to(parent_ret, tables_)) {
                    report_raw(sp, "Método '" + method_name + "' redefine el retorno con tipo incompatible ('" +
                               cr + "' no conforma a '" + pr + "').");
                }
            }
        }
    }
}

void SymbolResolver::check_arities() {} 

//  Helper resolve 

void SymbolResolver::resolve(Expr* node) {
    if (!node) return;
    node->accept(*this);
}

// Resolución de referencias

//  Literales 
void SymbolResolver::visit(Number&)  {}
void SymbolResolver::visit(String&)  {}
void SymbolResolver::visit(Boolean&) {}

//  Operaciones 
void SymbolResolver::visit(ArithmeticBinOp& n) { resolve(n.GetLeft()); resolve(n.GetRight()); }
void SymbolResolver::visit(LogicBinOp& n)      { resolve(n.GetLeft()); resolve(n.GetRight()); }
void SymbolResolver::visit(StringBinOp& n)     { resolve(n.GetLeft()); resolve(n.GetRight()); }
void SymbolResolver::visit(ArithmeticUnaryOp& n){ resolve(n.GetOperand()); }
void SymbolResolver::visit(LogicUnaryOp& n)    { resolve(n.GetOperand()); }

//  Builtins 
void SymbolResolver::visit(Print& n) { resolve(n.GetExpr()); }

// validar nombre y aridad de BuiltinCall
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
    if (!bf->is_variadic && static_cast<int>(n.GetArgs().size()) != bf->arity) {
        std::ostringstream oss;
        oss << "Función builtin '" << fname << "' espera "
            << bf->arity << " argumento(s) pero recibió "
            << n.GetArgs().size() << ".";
        report_raw(n.span, oss.str());
    }
    resolution_map_[&n] = ResolutionResult::from_builtin_func(bf);
}

//  Bloques 
void SymbolResolver::visit(ExprBlock& n) {
    push_scope();
    for (auto& e : n.GetExprs()) resolve(e.get());
    pop_scope();
}
void SymbolResolver::visit(Group& n) { resolve(n.GetExpr()); }

//  Variables 

void SymbolResolver::visit(VariableReference& n) {
    // constantes builtin 
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
    // anotación de tipo en binding let
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

//  Asignaciones 

void SymbolResolver::visit(DestructiveAssign& n) {
    if (tables_.lookup_builtin_const(n.GetName())) {
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

    auto* self_ref = dynamic_cast<SelfRef*>(n.GetObject());
    const bool same_type =
        !self_ref && !current_type_name_.empty() &&
        object_declared_type(n.GetObject()) == current_type_name_;

    if (!self_ref && !same_type) {
        report_raw(n.span, "Los atributos son privados. Solo se pueden modificar mediante 'self'.");
        resolution_map_[&n] = ResolutionResult{};
    }

    resolve(n.GetValue());
}

//  Control de flujo 

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

// variable sintética del for
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
    resolution_map_[&n] = ResolutionResult::from_synthetic(ptr);

    resolve(n.GetBody());
    pop_scope();
}

//  Funciones 

void SymbolResolver::visit(FunctionCall& n) {
    for (auto& arg : n.GetArgs()) resolve(arg.get());

    const SemanticFuncInfo* info = tables_.lookup_func(n.GetName());
    if (!info) {
        // Fallback: algunas funciones builtin (como 'range') se parsean como FunctionCall
        const BuiltinFuncInfo* builtin = tables_.lookup_builtin_func(n.GetName());
        if (builtin) {
            if (!builtin->is_variadic && static_cast<int>(n.GetArgs().size()) != builtin->arity) {
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

//  OOP 

void SymbolResolver::visit(NewExpr& n) {
    if (tables_.lookup_protocol(n.GetTypeName())) {
        report_raw(n.span, "No se puede instanciar el protocolo '" + n.GetTypeName() + "'.");
        resolution_map_[&n] = ResolutionResult{};
        for (auto& arg : n.GetArgs()) resolve(arg.get());
        return;
    }
    if (n.GetTypeName() == "Range") {
        report_raw(n.span, "El tipo builtin 'Range' solo se construye mediante range(...).");
        resolution_map_[&n] = ResolutionResult{};
        for (auto& arg : n.GetArgs()) resolve(arg.get());
        return;
    }
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

// resolver MemberAccess sobre self
std::string SymbolResolver::object_declared_type(Expr* obj) const {
    auto* var_ref = dynamic_cast<VariableReference*>(obj);
    if (!var_ref) return "";
    auto it = resolution_map_.find(var_ref);
    if (it == resolution_map_.end()) return "";
    const ResolutionResult& r = it->second;
    if (r.kind == ResolutionKind::Param && r.param && r.param->HasTypeAnnotation())
        return r.param->typeAnnotation;
    if (r.kind == ResolutionKind::Variable && r.binding && r.binding->HasTypeAnnotation())
        return r.binding->GetTypeAnnotation();
    return "";
}

void SymbolResolver::visit(MemberAccess& n) {
    resolve(n.GetObject());
    auto* self_ref = dynamic_cast<SelfRef*>(n.GetObject());

    const bool same_type =
        !self_ref && !current_type_name_.empty() &&
        object_declared_type(n.GetObject()) == current_type_name_;

    if (self_ref && current_type_name_.empty()) {
        report_raw(n.span, "'self' no es válido en este contexto.");
        resolution_map_[&n] = ResolutionResult{};
        return;
    }

    if (self_ref || same_type) {
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

// resolver MethodCall sobre self
void SymbolResolver::visit(MethodCall& n) {
    resolve(n.GetObject());
    for (auto& arg : n.GetArgs()) resolve(arg.get());

    auto* self_ref = dynamic_cast<SelfRef*>(n.GetObject());
    if (self_ref && !current_type_name_.empty()) {
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

// self solo en métodos, y se anota en resolution_map_
void SymbolResolver::visit(SelfRef& n) {
    if (context_ != ResolverContext::Method) {
        report_raw(n.span, "'self' solo puede usarse dentro de métodos de tipo.");
        resolution_map_[&n] = ResolutionResult{};
        return;
    }
    if (current_self_symbol_)
        resolution_map_[&n] = ResolutionResult::from_synthetic(current_self_symbol_);
}

// base() solo en métodos, con padre y método base válidos
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
    if (tables_.lookup_protocol(n.GetTypeName())) {
        report_raw(n.span, "No se puede usar el protocolo '" + n.GetTypeName() +
                   "' en una operación 'is'.");
        return;
    }
}

void SymbolResolver::visit(AsExpr& n) {
    resolve(n.GetExpr());
    check_type_annotation(n.span, n.GetTypeName());
    if (tables_.lookup_protocol(n.GetTypeName())) {
        report_raw(n.span, "No se puede usar el protocolo '" + n.GetTypeName() +
                   "' en una operación 'as'.");
        return;
    }
}


}
