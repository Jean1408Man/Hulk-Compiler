#ifndef HULK_SEMANTIC_ANALYZER_H
#define HULK_SEMANTIC_ANALYZER_H

#include "semantic_tables.h"
#include "../binding/symbol_resolver.h"
#include "../inference/hulk_type.h"
#include "../common/diagnosticEngine.hpp"
#include <memory>
#include <unordered_map>

namespace Hulk { 
    class Program; 
    class TypeInferencer;
    class TypeChecker;
}

namespace Hulk {

    // -----------------------------------------------------------------------
    // SemanticAnalyzer
    //
    // Orquesta los tres pases del SymbolResolver y expone los resultados
    // que el(TypeInferencer) necesita consumir:
    //
    //   - SemanticTables con todos los tipos y funciones registrados.
    //   - El mapa de resolución (Expr* → ResolutionResult)
    //
    // Uso típico en el pipeline:
    //
    //   hulk::common::DiagnosticEngine engine(repo);
    //   Hulk::SemanticAnalyzer sem(engine);
    //   bool ok = sem.analyze(*program);
    //   if (!ok) { engine.print_all(); return; }
    //   const auto& tables = sem.tables();
    //   const auto& res_map = sem.resolution_map();
    // -----------------------------------------------------------------------
    class SemanticAnalyzer {
    public:
        explicit SemanticAnalyzer(hulk::common::DiagnosticEngine& engine);
        ~SemanticAnalyzer();

        // Ejecuta el análisis semántico completo sobre el programa.
        // Devuelve true si no hubo errores.
        bool analyze(Program& program);

        // Acceso a los resultados (válidos solo si analyze() fue llamado)
        const SemanticTables& tables() const { return tables_; }
        SemanticTables&       tables()       { return tables_; }

        const std::unordered_map<Expr*, ResolutionResult>& resolution_map() const;
        const std::unordered_map<Expr*, HulkType>& type_map() const;

        bool has_errors() const { return has_errors_; }

    private:
        hulk::common::DiagnosticEngine& engine_;
        SemanticTables                  tables_;
        std::unique_ptr<SymbolResolver> resolver_;
        std::unique_ptr<TypeInferencer> inferencer_;
        std::unique_ptr<TypeChecker>    type_checker_;
        bool                            has_errors_ = false;
    };

}

#endif