#include "backend.h"
#include "output_packager.h"
#include "../vm/banner_vm.h"

#include <iostream>
#include <string>

namespace {

void print_usage() {
    std::cerr << "Uso: hulk <archivo.hulk> [-o salida]\n"
                 "  Por defecto compila a un ejecutable (./output).\n"
                 "Opciones de desarrollo:\n"
                 "  --run-banner             ejecutar en la VM sin emitir ejecutable\n"
                 "  --emit-ir                volcar HulkIR a archivo\n"
                 "  --emit-banner            volcar BannerIR a archivo\n"
                 "  --emit-banner-compiled   volcar vista compilada de la VM\n"
                 "  --restricted-inference   exigir anotaciones de tipo\n";
}

} 

int main(int argc, char** argv) {
    // ── Runtime mode ──
    if (auto embedded = Hulk::Backend::try_load_embedded_ir(); embedded) {
        try {
            Hulk::VM::BannerVM vm;
            (void)vm.run(*embedded);
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "runtime error: " << e.what() << "\n";
            return 1;
        }
    }

    // ── Driver mode (compiler) ───
    if (argc < 2) {
        print_usage();
        return 1;
    }

    Hulk::Backend::BackendOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 >= argc) {
                print_usage();
                return 1;
            }
            options.output_path = argv[++i];
        } else if (arg == "--emit-banner") {
            options.emit_banner = true;
        } else if (arg == "--emit-banner-compiled") {
            options.emit_banner_compiled = true;
        } else if (arg == "--run-banner") {
            options.run_banner = true;
        } else if (arg == "--emit-ir") {
            options.emit_ir = true;
        } else if (arg == "--restricted-inference") {
            options.semantic.restricted_inference = true;
        } else if (!arg.empty() && arg[0] != '-') {
            if (!options.input_path.empty()) {
                std::cerr << "Error: multiple archivos de entrada especificados.\n";
                print_usage();
                return 1;
            }
            options.input_path = arg;
        } else {
            std::cerr << "Opcion desconocida: " << arg << "\n";
            print_usage();
            return 1;
        }
    }

    if (options.input_path.empty()) {
        print_usage();
        return 1;
    }

    const int final_actions = static_cast<int>(options.emit_ir) +
                              static_cast<int>(options.emit_banner) +
                              static_cast<int>(options.emit_banner_compiled) +
                              static_cast<int>(options.run_banner);
    if (final_actions > 1) {
        std::cerr << "Use solo una accion final: --emit-ir, --emit-banner, "
                     "--emit-banner-compiled o --run-banner.\n";
        print_usage();
        return 1;
    }

    const Hulk::Backend::BackendResult result = Hulk::Backend::run_backend(options);
    return result.exit_code;
}
