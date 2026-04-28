#include "../semantic/analyzer.h"
#include "../parser/parser.hpp"
#include "../parser/parser_driver.hpp"
#include "../common/diagnosticEngine.hpp"
#include "../common/diagnosticRepository.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/others/program.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

// -----------------------------------------------------------------------
// main_semantic.cpp — punto de entrada para probar el Corte 7 en aislamiento.
//
// Uso: hulk_semantic <archivo.hulk>
//
// Ejecuta solo el análisis semántico (sin evaluar) e imprime los errores
// encontrados o un resumen de los tipos y funciones registrados.
// -----------------------------------------------------------------------

static std::string read_file(const std::string& path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) throw std::runtime_error("No se pudo abrir: " + path);
    std::ostringstream buf;
    buf << input.rdbuf();
    return buf.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: hulk_semantic <archivo.hulk>\n";
        return 1;
    }

    try {
        hulk::common::DiagnosticRepository repo;
        repo.load("lib/es_errors.json");
        hulk::common::DiagnosticEngine engine(repo);

        const std::string source = read_file(argv[1]);
        hulk::lexer::Lexer lexer(source, engine);
        hulk::parser::ParserDriver driver(lexer, engine);
        hulk::parser::Parser parser(driver);

        const int parse_rc = parser.parse();
        if (engine.has_errors() || parse_rc != 0) {
            engine.print_all();
            return 2;
        }

        Hulk::ASTnode* root = driver.result();
        if (!root) { std::cerr << "Parseo vacío\n"; return 1; }

        auto* program = dynamic_cast<Hulk::Program*>(root);
        if (!program) { std::cerr << "El AST raíz no es un Program\n"; return 1; }

        // --- Corte 7: análisis semántico ---
        Hulk::SemanticAnalyzer sem(engine);
        const bool ok = sem.analyze(*program);

        if (engine.has_errors()) {
            engine.print_all();
        }

        if (ok) {
            // Resumen de lo registrado — útil para depuración
            std::cout << "\n=== Análisis semántico OK ===\n";
            std::cout << "Funciones registradas:\n";
            for (auto& [name, info] : sem.tables().all_funcs()) {
                std::cout << "  function " << name << "(" << info.params.size() << " params)";
                if (!info.return_type_annotation.empty())
                    std::cout << " : " << info.return_type_annotation;
                std::cout << "\n";
            }
            std::cout << "Tipos registrados:\n";
            for (auto& [name, info] : sem.tables().all_types()) {
                std::cout << "  type " << name;
                if (!info.parent_name.empty())
                    std::cout << " inherits " << info.parent_name;
                std::cout << " { " << info.attributes.size() << " attrs, "
                          << info.methods.size() << " methods }\n";
            }
            std::cout << "Referencias resueltas: "
                      << sem.resolution_map().size() << "\n";
            return 0;
        }

        return 1;

    } catch (const std::exception& e) {
        std::cerr << "Error inesperado: " << e.what() << "\n";
        return 1;
    }
}
