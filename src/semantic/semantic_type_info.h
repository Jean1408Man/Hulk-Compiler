#ifndef HULK_SEMANTIC_TYPE_INFO_H
#define HULK_SEMANTIC_TYPE_INFO_H

#include "../ast/functions/param.h"
#include "../ast/abs_nodes/expr.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace Hulk { class TypeDecl; }

namespace Hulk {

    // SemanticAttrInfo — información estática de un atributo de tipo
    struct SemanticAttrInfo {
        std::string name;
        std::string type_annotation;  // "" si no tiene anotación explícita
        Expr*       initializer;      // puntero no-owning al AST (vive en Program)
    };

    // SemanticMethodInfo — información estática de un método de tipo
    struct SemanticMethodInfo {
        std::string              name;
        std::vector<Param>       params;          // copia de los params (para acceso por valor)
        std::vector<const Param*> ast_params;     // punteros a los Param en el AST (para param_types_ lookup)
        std::string              return_type_annotation;  // "" si no tiene
        Expr*                    body = nullptr;          // puntero no-owning al AST
        bool                     is_override = false;
    };

    // SemanticTypeInfo — información estática de un TypeDecl completo.
    //
    // Versión "análisis estático" de TypeDef (hulk_object.h).
    // No contiene valores en runtime — solo metadatos estructurales.
    struct SemanticTypeInfo {
        std::string         name;
        std::vector<Param>  ctor_params;
        std::string         parent_name;  // "" si no tiene padre declarado
        std::vector<SemanticAttrInfo>                        attributes;
        std::unordered_map<std::string, SemanticMethodInfo>  methods;
        TypeDecl*           decl = nullptr;  // puntero al nodo original (para spans en errores)
        bool                is_builtin = false; // true para Object, Number, String, Boolean
        bool                defines_constructor = false; // true si se definió ctor_params en el AST
    };

} 

#endif