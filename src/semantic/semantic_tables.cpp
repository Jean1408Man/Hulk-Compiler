#include "semantic_tables.h"
#include <stdexcept>

namespace Hulk {

// Registro
bool SemanticTables::register_type(SemanticTypeInfo info) {
    const std::string name = info.name;
    auto [_, inserted] = types_.emplace(name, std::move(info));
    return inserted;
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

// Iteración
const std::unordered_map<std::string, SemanticTypeInfo>& SemanticTables::all_types() const {
    return types_;
}

const std::unordered_map<std::string, SemanticFuncInfo>& SemanticTables::all_funcs() const {
    return funcs_;
}

}