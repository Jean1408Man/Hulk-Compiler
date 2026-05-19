#include "backend.h"

#include <iostream>
#include <string>

namespace {

void print_usage() {
    std::cerr << "Uso: hulk_backend <archivo.hulk> [-o salida] [--emit-ir] [--emit-banner] [--run-banner] [--emit-cpp] [--keep-temp]\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    Hulk::Backend::BackendOptions options;
    options.input_path = argv[1];

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 >= argc) {
                print_usage();
                return 1;
            }
            options.output_path = argv[++i];
        } else if (arg == "--emit-cpp") {
            options.emit_cpp = true;
        } else if (arg == "--emit-banner") {
            options.emit_banner = true;
        } else if (arg == "--run-banner") {
            options.run_banner = true;
        } else if (arg == "--emit-ir") {
            options.emit_ir = true;
        } else if (arg == "--keep-temp") {
            options.keep_temp = true;
        } else {
            std::cerr << "Opcion desconocida: " << arg << "\n";
            print_usage();
            return 1;
        }
    }

    const int final_actions = static_cast<int>(options.emit_ir) +
                              static_cast<int>(options.emit_cpp) +
                              static_cast<int>(options.emit_banner) +
                              static_cast<int>(options.run_banner);
    if (final_actions > 1) {
        std::cerr << "Use solo una accion final: --emit-ir, --emit-banner, --run-banner o --emit-cpp.\n";
        print_usage();
        return 1;
    }

    const Hulk::Backend::BackendResult result = Hulk::Backend::run_backend(options);
    return result.ok ? 0 : 1;
}
