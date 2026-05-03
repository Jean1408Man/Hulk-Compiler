#ifndef HULK_SEMANTIC_FUNC_INFO_H
#define HULK_SEMANTIC_FUNC_INFO_H

#include "../ast/functions/param.h"
#include "../ast/abs_nodes/expr.h"
#include <string>
#include <vector>

// Forward declaration
namespace Hulk { class FunctionDecl; }

namespace Hulk {

    // -----------------------------------------------------------------------
    // SemanticFuncInfo — información estática de un FunctionDecl completo.
    //
    // Registrada en SemanticTables durante el Pase 1 del SymbolResolver.
    // -----------------------------------------------------------------------
    struct SemanticFuncInfo {
        std::string         name;
        std::vector<Param>  params;
        std::string         return_type_annotation;  // "" si no tiene anotación
        Expr*               body;                    // puntero no-owning al AST
        FunctionDecl*       decl = nullptr;          // puntero al nodo original
    };

}

#endif 