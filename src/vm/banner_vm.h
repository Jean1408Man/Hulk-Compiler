#ifndef HULK_BANNER_VM_H
#define HULK_BANNER_VM_H

#include "../banner/banner_ir.h"
#include "vm_heap.h"
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

    struct CompiledType {
        const Banner::BannerType* type = nullptr;
        int type_id = -1;
        int parent_type_id = -1;
        std::unordered_map<std::string, std::size_t> field_slots;
        std::unordered_map<std::string, std::size_t> method_slots;
        std::vector<std::string> vtable;
    };

    CompiledFunction compile_function(const Banner::BannerFunction& function) const;
    Frame make_frame(const CompiledFunction& function,
                     const std::vector<VMValue>& args,
                     std::string return_dest = {}) const;
    std::unordered_map<std::string, CompiledType>
    compile_types(const Banner::BannerProgram& program) const;
    std::size_t slot_of(const CompiledFunction& function, const std::string& name) const;
    std::size_t label_of(const CompiledFunction& function, const std::string& label) const;
    const CompiledType& type_of(const std::unordered_map<std::string, CompiledType>& types,
                                const std::string& name) const;
    const CompiledType& type_of(const std::unordered_map<std::string, CompiledType>& types,
                                const VMValue& value) const;
    std::size_t field_slot(const CompiledType& type, const std::string& field_name) const;
    std::size_t method_slot(const CompiledType& type, const std::string& method_name) const;
    bool is_instance(const VMValue& value,
                     const std::string& type_name,
                     const std::unordered_map<std::string, CompiledType>& types) const;
    [[noreturn]] void unsupported(const std::string& feature) const;

    VMHeap heap_;
};

}

#endif
