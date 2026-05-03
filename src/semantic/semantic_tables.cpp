#include "semantic_tables.h"
#include <stdexcept>

namespace Hulk {

// Constructor — registra tipos builtin y funciones/constantes de dominio
SemanticTables::SemanticTables() {
    // Tipos builtin de la jerarquía raíz
    register_builtin_type("Object",  "");
    register_builtin_type("Number",  "Object");
    register_builtin_type("String",  "Object");
    register_builtin_type("Boolean", "Object");

    // Funciones builtin (nombre, aridad, param_types, return_type)
    struct BuiltinDef { const char* name; int arity; std::vector<std::string> param_types; const char* return_type; };
    for (auto& def : std::initializer_list<BuiltinDef>{
            {"print",  1, {"Object"}, "String"}, // o vacío según preferencia
            {"sqrt",   1, {"Number"}, "Number"},
            {"sin",    1, {"Number"}, "Number"},
            {"cos",    1, {"Number"}, "Number"},
            {"exp",    1, {"Number"}, "Number"},
            {"log",    2, {"Number", "Number"}, "Number"},
            {"rand",   0, {}, "Number"},
            {"range",  2, {"Number", "Number"}, "Iterable"},
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

}