#ifndef HULK_SEMANTIC_TABLES_H
#define HULK_SEMANTIC_TABLES_H

#include "semantic_type_info.h"
#include "semantic_func_info.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace Hulk {

    // -----------------------------------------------------------------------
    // SemanticTables — contenedor de todas las tablas de análisis estático.
    //
    // Construido por el SymbolResolver (Pase 1) y consumido por:
    //   - los chequeos semánticos del propio SymbolResolver (Pase 3),
    //   - el TypeInferencer del Corte 8,
    //   - el TypeChecker del Corte 9.
    // -----------------------------------------------------------------------
    class SemanticTables {
    public:
        // Registro (llamado durante Pase 1 del SymbolResolver)

        // Devuelve false si ya existía un tipo con ese nombre (duplicado).
        bool register_type(SemanticTypeInfo info);

        // Devuelve false si ya existía una función con ese nombre (duplicado).
        bool register_func(SemanticFuncInfo info);

        // Consulta

        // nullptr si no existe.
        const SemanticTypeInfo* lookup_type(const std::string& name) const;
        SemanticTypeInfo*       lookup_type(const std::string& name);

        // nullptr si no existe.
        const SemanticFuncInfo* lookup_func(const std::string& name) const;

        // Jerarquía de herencia

        // ¿Es 'child' un subtipo (igual o descendiente) de 'parent'?
        // Sube la cadena parent_name hasta encontrar 'parent' o llegar a "".
        bool is_subtype(const std::string& child, const std::string& parent) const;

        // ¿Existe un ciclo de herencia que incluya 'type_name'?
        // Usa detección de ciclos via conjunto de visitados.
        bool has_inheritance_cycle(const std::string& type_name) const;

        // Devuelve el nombre del primer tipo en la cadena de herencia
        // de 'type_name' (o vacío si no existe ninguno).
        std::string find_ancestor(const std::string& type_name,
                                  const std::string& target) const;

        // ¿Existe el método 'method_name' en 'type_name' o en algún ancestro?
        const SemanticMethodInfo* find_method(const std::string& type_name,
                                              const std::string& method_name) const;

        // Iteración (para chequeos globales)
        const std::unordered_map<std::string, SemanticTypeInfo>& all_types() const;
        const std::unordered_map<std::string, SemanticFuncInfo>& all_funcs() const;

    private:
        std::unordered_map<std::string, SemanticTypeInfo> types_;
        std::unordered_map<std::string, SemanticFuncInfo> funcs_;

        // Auxiliar para detección de ciclos — DFS con conjunto de visitados
        bool has_cycle_impl(const std::string& name,
                            std::unordered_set<std::string>& visited,
                            std::unordered_set<std::string>& in_stack) const;
    };

}

#endif