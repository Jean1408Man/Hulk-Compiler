#include "semantic_tables.h"
#include <stdexcept>

namespace {

std::string canonical_type_name(const std::string& name) {
    if (name.empty() || name == "auto" || name == "_") return "";
    return name;
}

bool is_typed_iterable_name(const std::string& name) {
    return !name.empty() && name.back() == '*';
}

std::string typed_iterable_element_name(const std::string& name) {
    return is_typed_iterable_name(name) ? name.substr(0, name.size() - 1) : "";
}

bool is_valid_typed_iterable_element_name(const std::string& name) {
    return !name.empty() && name != "auto" && name != "_" && name != "Void";
}

}

namespace Hulk {

// Constructor — registra tipos builtin y funciones/constantes de dominio
SemanticTables::SemanticTables() {
    // Tipos builtin de la jerarquía raíz
    register_builtin_type("Object",  "");
    register_builtin_type("Number",  "Object");
    register_builtin_type("String",  "Object");
    register_builtin_type("Boolean", "Object");

    SemanticTypeInfo range;
    range.name = "Range";
    range.parent_name = "Object";
    range.is_builtin = true;
    range.decl = nullptr;
    range.defines_constructor = true;
    range.ctor_params = {Param("min", "Number"), Param("max", "Number")};
    range.attributes.push_back(SemanticAttrInfo{"current", "Number", nullptr});
    range.attributes.push_back(SemanticAttrInfo{"max", "Number", nullptr});
    range.methods.emplace("next",
                          SemanticMethodInfo{"next", {}, {}, "Boolean", nullptr, false});
    range.methods.emplace("current",
                          SemanticMethodInfo{"current", {}, {}, "Number", nullptr, false});
    types_.emplace(range.name, std::move(range));

    SemanticProtocolInfo iterable;
    iterable.name = "Iterable";
    iterable.is_builtin = true;
    iterable.methods.emplace(
        "next",
        SemanticProtocolMethodInfo{"next", {}, "Boolean"});
    iterable.methods.emplace(
        "current",
        SemanticProtocolMethodInfo{"current", {}, "Object"});
    protocols_.emplace(iterable.name, std::move(iterable));

    // Funciones builtin (nombre, aridad, param_types, return_type)
    struct BuiltinDef { const char* name; int arity; std::vector<std::string> param_types; const char* return_type; };
    for (auto& def : std::initializer_list<BuiltinDef>{
            {"print",  1, {"Object"}, "Object"},
            {"sqrt",   1, {"Number"}, "Number"},
            {"sin",    1, {"Number"}, "Number"},
            {"cos",    1, {"Number"}, "Number"},
            {"exp",    1, {"Number"}, "Number"},
            {"log",    2, {"Number", "Number"}, "Number"},
            {"rand",   0, {}, "Number"},
            {"range",  2, {"Number", "Number"}, "Range"},
        })
    {
        BuiltinFuncInfo bfi;
        bfi.name  = def.name;
        bfi.arity = def.arity;
        bfi.param_types = def.param_types;
        bfi.return_type = def.return_type;
        builtin_funcs_[bfi.name] = bfi;
    }

    // Constantes builtin
    for (auto& [n, t] : std::initializer_list<std::pair<const char*, const char*>>{
            {"PI", "Number"},
            {"E",  "Number"},
        })
    {
        BuiltinConstInfo bci;
        bci.name = n;
        bci.type = t;
        builtin_consts_[bci.name] = bci;
    }
}

std::vector<Param> SemanticTables::get_effective_constructor(const std::string& type_name) const {
    const SemanticTypeInfo* info = lookup_type(type_name);
    if (!info) return {};
    
    if (info->defines_constructor) {
        return info->ctor_params;
    }
    
    if (!info->parent_name.empty()) {
        return get_effective_constructor(info->parent_name);
    }
    
    return {};
}

void SemanticTables::register_builtin_type(const std::string& name,
                                           const std::string& parent) {
    SemanticTypeInfo info;
    info.name        = name;
    info.parent_name = parent;
    info.is_builtin  = true;
    info.decl        = nullptr;
    types_.emplace(name, std::move(info));
}

// Registro
bool SemanticTables::register_type(SemanticTypeInfo info) {
    // Rechazar redeclaración de tipos builtin
    auto it = types_.find(info.name);
    if (it != types_.end()) return false;   // duplicado (incluye builtins)
    types_.emplace(info.name, std::move(info));
    return true;
}

bool SemanticTables::register_func(SemanticFuncInfo info) {
    const std::string name = info.name;
    auto [_, inserted] = funcs_.emplace(name, std::move(info));
    return inserted;
}

bool SemanticTables::register_protocol(SemanticProtocolInfo info) {
    if (protocols_.count(info.name) || types_.count(info.name)) return false;
    auto [_, inserted] = protocols_.emplace(info.name, std::move(info));
    return inserted;
}

bool SemanticTables::ensure_typed_iterable_protocol(const std::string& element_type_name) {
    if (!is_valid_typed_iterable_element_name(element_type_name)) return false;

    if (is_typed_iterable_name(element_type_name)) {
        const std::string nested_element = typed_iterable_element_name(element_type_name);
        if (!ensure_typed_iterable_protocol(nested_element)) return false;
    }

    const std::string protocol_name = element_type_name + "*";
    if (protocols_.count(protocol_name)) return true;
    if (types_.count(protocol_name)) return false;
    if (!lookup_type(element_type_name) && !lookup_protocol(element_type_name)) return false;

    SemanticProtocolInfo info;
    info.name = protocol_name;
    info.parent_name = "Iterable";
    info.is_builtin = true;
    info.methods.emplace(
        "current",
        SemanticProtocolMethodInfo{"current", {}, element_type_name});
    protocols_.emplace(info.name, std::move(info));
    return true;
}

// Consulta
const SemanticTypeInfo* SemanticTables::lookup_type(const std::string& name) const {
    auto it = types_.find(name);
    return (it != types_.end()) ? &it->second : nullptr;
}

SemanticTypeInfo* SemanticTables::lookup_type(const std::string& name) {
    auto it = types_.find(name);
    return (it != types_.end()) ? &it->second : nullptr;
}

const SemanticFuncInfo* SemanticTables::lookup_func(const std::string& name) const {
    auto it = funcs_.find(name);
    return (it != funcs_.end()) ? &it->second : nullptr;
}

const SemanticProtocolInfo* SemanticTables::lookup_protocol(const std::string& name) const {
    auto it = protocols_.find(name);
    return (it != protocols_.end()) ? &it->second : nullptr;
}

SemanticProtocolInfo* SemanticTables::lookup_protocol(const std::string& name) {
    auto it = protocols_.find(name);
    return (it != protocols_.end()) ? &it->second : nullptr;
}

bool SemanticTables::is_protocol(const std::string& name) const {
    return lookup_protocol(name) != nullptr;
}

const BuiltinFuncInfo* SemanticTables::lookup_builtin_func(const std::string& name) const {
    auto it = builtin_funcs_.find(name);
    return (it != builtin_funcs_.end()) ? &it->second : nullptr;
}

const BuiltinConstInfo* SemanticTables::lookup_builtin_const(const std::string& name) const {
    auto it = builtin_consts_.find(name);
    return (it != builtin_consts_.end()) ? &it->second : nullptr;
}

// Jerarquía de herencia
bool SemanticTables::is_subtype(const std::string& child,
                                const std::string& parent) const {
    if (child == parent) return true;
    std::string current = child;
    // Sube la cadena parent_name; límite defensivo para evitar loops infinitos
    // (los ciclos deben haberse detectado antes con has_inheritance_cycle)
    constexpr int MAX_DEPTH = 256;
    int depth = 0;
    while (!current.empty() && depth < MAX_DEPTH) {
        auto it = types_.find(current);
        if (it == types_.end()) break;
        current = it->second.parent_name;
        if (current == parent) return true;
        ++depth;
    }
    return false;
}

// Detección de ciclos: DFS con coloreado (blanco / gris / negro)
bool SemanticTables::has_cycle_impl(const std::string& name,
                                    std::unordered_set<std::string>& visited,
                                    std::unordered_set<std::string>& in_stack) const {
    if (in_stack.count(name)) return true;   // volvemos a un nodo en la pila → ciclo
    if (visited.count(name))  return false;  // ya procesado completamente

    visited.insert(name);
    in_stack.insert(name);

    auto it = types_.find(name);
    if (it != types_.end() && !it->second.parent_name.empty()) {
        if (has_cycle_impl(it->second.parent_name, visited, in_stack))
            return true;
    }

    in_stack.erase(name);
    return false;
}

bool SemanticTables::has_inheritance_cycle(const std::string& type_name) const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> in_stack;
    return has_cycle_impl(type_name, visited, in_stack);
}

std::string SemanticTables::find_ancestor(const std::string& type_name,
                                          const std::string& target) const {
    std::string current = type_name;
    constexpr int MAX_DEPTH = 256;
    int depth = 0;
    while (!current.empty() && depth < MAX_DEPTH) {
        if (current == target) return current;
        auto it = types_.find(current);
        if (it == types_.end()) break;
        current = it->second.parent_name;
        ++depth;
    }
    return "";
}

std::string SemanticTables::find_lca(const std::string& a, const std::string& b) const {
    if (a == b) return a;
    if (a.empty() || b.empty()) return "Object";

    std::unordered_set<std::string> ancestors_a;
    std::string current = a;
    int depth = 0;
    while (!current.empty() && depth < 256) {
        ancestors_a.insert(current);
        auto it = types_.find(current);
        if (it == types_.end()) break;
        current = it->second.parent_name;
        depth++;
    }

    current = b;
    depth = 0;
    while (!current.empty() && depth < 256) {
        if (ancestors_a.count(current)) return current;
        auto it = types_.find(current);
        if (it == types_.end()) break;
        current = it->second.parent_name;
        depth++;
    }

    return "Object";
}

const SemanticMethodInfo* SemanticTables::find_method(
        const std::string& type_name,
        const std::string& method_name) const {
    std::string current = type_name;
    constexpr int MAX_DEPTH = 256;
    int depth = 0;
    while (!current.empty() && depth < MAX_DEPTH) {
        auto it = types_.find(current);
        if (it == types_.end()) break;
        auto mit = it->second.methods.find(method_name);
        if (mit != it->second.methods.end()) return &mit->second;
        current = it->second.parent_name;
        ++depth;
    }
    return nullptr;
}

const SemanticProtocolMethodInfo* SemanticTables::find_protocol_method(
        const std::string& protocol_name,
        const std::string& method_name) const {
    const SemanticProtocolInfo* protocol = lookup_protocol(protocol_name);
    constexpr int MAX_DEPTH = 256;
    int depth = 0;
    while (protocol && depth < MAX_DEPTH) {
        auto it = protocol->methods.find(method_name);
        if (it != protocol->methods.end()) return &it->second;
        protocol = protocol->parent_name.empty() ? nullptr : lookup_protocol(protocol->parent_name);
        ++depth;
    }
    return nullptr;
}

const SemanticAttrInfo* SemanticTables::find_attribute(
        const std::string& type_name,
        const std::string& attr_name) const {
    std::string current = type_name;
    constexpr int MAX_DEPTH = 256;
    int depth = 0;
    while (!current.empty() && depth < MAX_DEPTH) {
        auto it = types_.find(current);
        if (it == types_.end()) break;
        for (const auto& attr : it->second.attributes) {
            if (attr.name == attr_name) return &attr;
        }
        current = it->second.parent_name;
        ++depth;
    }
    return nullptr;
}

// Iteración
const std::unordered_map<std::string, SemanticTypeInfo>& SemanticTables::all_types() const {
    return types_;
}

const std::unordered_map<std::string, SemanticFuncInfo>& SemanticTables::all_funcs() const {
    return funcs_;
}

const std::unordered_map<std::string, SemanticProtocolInfo>& SemanticTables::all_protocols() const {
    return protocols_;
}

bool SemanticTables::method_satisfies_protocol(
        const SemanticMethodInfo& actual,
        const SemanticProtocolMethodInfo& required) const {
    if (actual.params.size() != required.params.size()) return false;
    if (actual.return_type_annotation.empty() || required.return_type_annotation.empty()) return false;

    for (std::size_t i = 0; i < actual.params.size(); ++i) {
        const std::string actual_param = canonical_type_name(actual.params[i].typeAnnotation);
        const std::string required_param = canonical_type_name(required.params[i].typeAnnotation);
        if (actual_param.empty() || required_param.empty()) return false;
        if (!type_name_conforms(required_param, actual_param)) return false;
    }

    return type_name_conforms(actual.return_type_annotation, required.return_type_annotation);
}

bool SemanticTables::protocol_method_satisfies_protocol(
        const SemanticProtocolMethodInfo& actual,
        const SemanticProtocolMethodInfo& required) const {
    if (actual.params.size() != required.params.size()) return false;
    if (actual.return_type_annotation.empty() || required.return_type_annotation.empty()) return false;

    for (std::size_t i = 0; i < actual.params.size(); ++i) {
        const std::string actual_param = canonical_type_name(actual.params[i].typeAnnotation);
        const std::string required_param = canonical_type_name(required.params[i].typeAnnotation);
        if (actual_param.empty() || required_param.empty()) return false;
        if (!type_name_conforms(required_param, actual_param)) return false;
    }

    return type_name_conforms(actual.return_type_annotation, required.return_type_annotation);
}

bool SemanticTables::type_name_conforms(const std::string& actual,
                                        const std::string& expected) const {
    if (actual == expected) return true;
    if (expected == "Object") return lookup_type(actual) != nullptr || lookup_protocol(actual) != nullptr;

    if (lookup_protocol(expected)) {
        if (lookup_protocol(actual)) return protocol_conforms_to_protocol(actual, expected);
        return type_conforms_to_protocol(actual, expected);
    }

    if (lookup_protocol(actual)) return false;
    return is_subtype(actual, expected);
}

bool SemanticTables::type_conforms_to_protocol(const std::string& type_name,
                                               const std::string& protocol_name) const {
    const SemanticTypeInfo* type = lookup_type(type_name);
    const SemanticProtocolInfo* protocol = lookup_protocol(protocol_name);
    if (!type || !protocol) return false;

    if (!protocol->parent_name.empty() &&
        !type_conforms_to_protocol(type_name, protocol->parent_name)) {
        return false;
    }

    for (const auto& [method_name, required] : protocol->methods) {
        const SemanticMethodInfo* actual = find_method(type_name, method_name);
        if (!actual || !method_satisfies_protocol(*actual, required)) return false;
    }
    return true;
}

bool SemanticTables::protocol_conforms_to_protocol(const std::string& child,
                                                   const std::string& parent) const {
    if (child == parent) return true;
    const SemanticProtocolInfo* child_info = lookup_protocol(child);
    const SemanticProtocolInfo* parent_info = lookup_protocol(parent);
    if (!child_info || !parent_info) return false;

    if (!parent_info->parent_name.empty() &&
        !protocol_conforms_to_protocol(child, parent_info->parent_name)) {
        return false;
    }

    for (const auto& [method_name, required] : parent_info->methods) {
        const SemanticProtocolMethodInfo* actual = find_protocol_method(child, method_name);
        if (!actual || !protocol_method_satisfies_protocol(*actual, required)) return false;
    }
    return true;
}

bool SemanticTables::has_protocol_cycle_impl(
        const std::string& name,
        std::unordered_set<std::string>& visited,
        std::unordered_set<std::string>& in_stack) const {
    if (in_stack.count(name)) return true;
    if (visited.count(name)) return false;
    visited.insert(name);
    in_stack.insert(name);

    auto it = protocols_.find(name);
    if (it != protocols_.end() && !it->second.parent_name.empty()) {
        if (has_protocol_cycle_impl(it->second.parent_name, visited, in_stack)) return true;
    }

    in_stack.erase(name);
    return false;
}

bool SemanticTables::has_protocol_cycle(const std::string& protocol_name) const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> in_stack;
    return has_protocol_cycle_impl(protocol_name, visited, in_stack);
}

}
