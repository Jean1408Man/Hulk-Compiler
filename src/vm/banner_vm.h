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

    struct Frame {
        const CompiledFunction* function = nullptr;
        std::size_t pc = 0;
        std::vector<VMValue> slots;
        std::vector<VMValue> param_buffer;
        std::string return_dest;
        bool has_return_dest = false;
    };

    CompiledFunction compile_function(const Banner::BannerFunction& function) const;
    Frame make_frame(const CompiledFunction& function,
                     const std::vector<VMValue>& args,
                     std::string return_dest = {}) const;
    std::size_t slot_of(const CompiledFunction& function, const std::string& name) const;
    std::size_t label_of(const CompiledFunction& function, const std::string& label) const;
    [[noreturn]] void unsupported(const std::string& feature) const;
};

}

#endif
