#ifndef HULK_BACKEND_DRIVER_H
#define HULK_BACKEND_DRIVER_H

#include "../semantic/analyzer.h"

#include <string>

namespace Hulk::Backend {

struct BackendOptions {
    std::string input_path;
    std::string output_path;
    SemanticOptions semantic;
    bool emit_ir = false;
    bool emit_banner = false;
    bool emit_banner_compiled = false;
    bool run_banner = false;
};

struct BackendResult {
    bool ok = false;
    int exit_code = 1; 
    std::string generated_ir_path;
    std::string generated_banner_path;
};

class BackendDriver {
public:
    BackendResult run(const BackendOptions& options);

private:
    std::string read_file(const std::string& path) const;
    std::string default_output_path(const BackendOptions& options) const;
    bool write_file(const std::string& path, const std::string& content) const;
};

}

#endif
