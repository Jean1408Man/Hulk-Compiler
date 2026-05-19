#ifndef HULK_BANNER_VM_H
#define HULK_BANNER_VM_H

#include "../banner/banner_ir.h"
#include "vm_value.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace Hulk::VM {

class BannerVM {
public:
    VMValue run(const Banner::BannerProgram& program);

private:
    struct CompiledFunction {
        const Banner::BannerFunction* function = nullptr;
        std::unordered_map<std::string, std::size_t> slots;
        std::unordered_map<std::string, std::size_t> labels;
    };

    CompiledFunction compile_function(const Banner::BannerFunction& function) const;
    VMValue run_function(const Banner::BannerProgram& program,
                         const CompiledFunction& function,
                         const std::unordered_map<std::string, std::string>& data) const;
    std::size_t slot_of(const CompiledFunction& function, const std::string& name) const;
    std::size_t label_of(const CompiledFunction& function, const std::string& label) const;
    [[noreturn]] void unsupported(const std::string& feature) const;
};

}

#endif
