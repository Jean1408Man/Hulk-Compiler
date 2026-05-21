#include "backend_driver.h"

#include "codegen_error.h"
#include "hulkir_to_banner.h"
#include "ir_gen.h"

#include "../banner/banner_printer.h"
#include "../ast/others/program.h"
#include "../common/diagnosticEngine.hpp"
#include "../common/diagnosticRepository.hpp"
#include "../ir/ir_printer.h"
#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"
#include "../parser/parser_driver.hpp"
#include "../semantic/analyzer.h"
#include "../vm/banner_vm.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Hulk::Backend {

BackendResult BackendDriver::run(const BackendOptions& options) {
    BackendResult result;

    try {
        hulk::common::DiagnosticRepository repo;
        repo.load("lib/es_errors.json");
        hulk::common::DiagnosticEngine engine(repo);

        const std::string source = read_file(options.input_path);
        hulk::lexer::Lexer lexer(source, engine);
        hulk::parser::ParserDriver parser_driver(lexer, engine);
        hulk::parser::Parser parser(parser_driver);

        const int parse_rc = parser.parse();
        if (engine.has_errors() || parse_rc != 0) {
            engine.print_all();
            return result;
        }

        Hulk::ASTnode* root = parser_driver.result();
        if (!root) {
            std::cerr << "Backend: parseo vacio.\n";
            return result;
        }

        auto* program = dynamic_cast<Hulk::Program*>(root);
        if (!program) {
            std::cerr << "Backend: el AST raiz no es Program.\n";
            return result;
        }

        Hulk::SemanticAnalyzer sem(engine);
        const bool sem_ok = sem.analyze(*program);
        if (!sem_ok || engine.has_blocking_errors()) {
            engine.print_all();
            return result;
        }

        IRGen irgen(sem.tables(), sem.resolution_map(), sem.type_map());
        const IR::IRProgram ir = irgen.generate(*program);

        if (options.emit_ir) {
            result.generated_ir_path = default_output_path(options);
            const IR::IRPrinter printer;
            if (!write_file(result.generated_ir_path, printer.print(ir))) {
                std::cerr << "Backend: no se pudo escribir " << result.generated_ir_path << "\n";
                return result;
            }
            result.ok = true;
            return result;
        }

        const HulkIRToBanner lowerer;
        const Banner::BannerProgram banner = lowerer.lower(ir);

        if (options.emit_banner) {
            result.generated_banner_path = default_output_path(options);
            const Banner::BannerPrinter printer;
            if (!write_file(result.generated_banner_path, printer.print(banner))) {
                std::cerr << "Backend: no se pudo escribir " << result.generated_banner_path << "\n";
                return result;
            }
            result.ok = true;
            return result;
        }

        VM::BannerVM vm;
        (void)vm.run(banner);
        result.ok = true;
        return result;
    } catch (const CodegenError& err) {
        std::cerr << err.what() << "\n";
        return result;
    } catch (const std::exception& err) {
        std::cerr << "Backend: error inesperado: " << err.what() << "\n";
        return result;
    }
}

std::string BackendDriver::read_file(const std::string& path) const {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) throw std::runtime_error("No se pudo abrir: " + path);
    std::ostringstream buf;
    buf << input.rdbuf();
    return buf.str();
}

std::string BackendDriver::default_output_path(const BackendOptions& options) const {
    if (!options.output_path.empty()) {
        return options.output_path;
    }
    if (options.emit_ir) {
        return options.input_path + ".hir";
    }
    if (options.emit_banner) {
        return options.input_path + ".banner";
    }
    return {};
}

bool BackendDriver::write_file(const std::string& path, const std::string& content) const {
    std::ofstream output(path, std::ios::out | std::ios::binary);
    if (!output) return false;
    output << content;
    return static_cast<bool>(output);
}

}
