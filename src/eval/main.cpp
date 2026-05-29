#include "evaluator.h"
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

static std::string read_file(const std::string& path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) throw std::runtime_error("No se pudo abrir: " + path);
    std::ostringstream buf;
    buf << input.rdbuf();
    return buf.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: hulk_eval <archivo.hulk> [--restricted-inference]\n";
        return 1;
    }

    try {
        std::string input_path;
        Hulk::SemanticOptions semantic_options;

        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--restricted-inference") {
                semantic_options.restricted_inference = true;
            } else if (input_path.empty()) {
                input_path = arg;
            } else {
                std::cerr << "Opcion desconocida: " << arg << "\n";
                std::cerr << "Uso: hulk_eval <archivo.hulk> [--restricted-inference]\n";
                return 1;
            }
        }

        if (input_path.empty()) {
            std::cerr << "Uso: hulk_eval <archivo.hulk> [--restricted-inference]\n";
            return 1;
        }

        hulk::common::DiagnosticRepository repo;
        repo.load("lib/es_errors.json");

        hulk::common::DiagnosticEngine engine(repo);
        const std::string source = read_file(input_path);

        hulk::lexer::Lexer lexer(source, engine);
        hulk::parser::ParserDriver driver(lexer, engine);
        hulk::parser::Parser parser(driver);

        const int parse_rc = parser.parse();

        if (engine.has_errors() || parse_rc != 0) {
            engine.print_all();
            return 2;
        }

        // El resultado del parser es un ASTnode*; hacemos downcast a Program
        Hulk::ASTnode* root = driver.result();
        if (!root) { std::cerr << "Parseo vacío\n"; return 1; }

        auto* program = dynamic_cast<Hulk::Program*>(root);
        if (!program) { std::cerr << "El AST raíz no es un Program\n"; return 1; }

        // --- análisis semántico antes de evaluar ---
        Hulk::SemanticAnalyzer sem(engine, semantic_options);
        sem.analyze(*program);

        if (engine.has_errors()) {
            engine.print_all();
            return 1;
        }

        Hulk::Evaluator ev(engine);
        try {
            ev.run(*program);
        } catch (const Hulk::EvalError&) {
            // El error ya fue registrado en el engine — solo imprimimos
        }

        if (engine.has_errors()) {
            engine.print_all();
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error inesperado: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
