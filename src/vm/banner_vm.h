#ifndef HULK_BANNER_VM_H
#define HULK_BANNER_VM_H

#include "../banner/banner_ir.h"
#include "vm_heap.h"
#include "vm_value.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Hulk::VM {

struct VMOptions {
    std::size_t max_frames = 100000;
    std::uint64_t max_steps = 10000000;
    std::size_t max_heap_values = 1000000;
};

class BannerVM {
public:
    Word run(const Banner::BannerProgram& program, const VMOptions& options = {});
    std::string compiled_view(const Banner::BannerProgram& program);

private:
    struct CompiledInstr {
        Banner::Op op = Banner::Op::Nop;
        std::size_t dest_slot = 0;
        std::size_t src1_slot = 0;
        std::size_t src2_slot = 0;
        std::size_t label_pc = 0;
        std::size_t callee_id = 0;
        std::size_t field_slot = 0;
        std::size_t method_slot = 0;
        int type_id = -1;
        bool has_dest = false;
        bool has_field_slot = false;
        bool has_method_slot = false;
        std::string data_label;
        std::string field_name;
        std::string method_name;
        std::string type_name;
        std::vector<std::size_t> arg_slots;
        double number_value = 0.0;
        bool bool_value = false;
        std::optional<IR::SourceSpan> source;
    };

    struct CompiledFunction {
        const Banner::BannerFunction* function = nullptr;
        std::unordered_map<std::string, std::size_t> slots;
        std::unordered_map<std::string, std::size_t> labels;
        std::vector<CompiledInstr> code;
    };

    struct Frame {
        const CompiledFunction* function = nullptr;
        std::size_t pc = 0;
        std::vector<Word> slots;
        std::vector<Word> param_buffer;
        std::size_t return_slot = 0;
        bool has_return_slot = false;
    };

    struct CompiledType {
        const Banner::BannerType* type = nullptr;
        int type_id = -1;
        int parent_type_id = -1;
        std::unordered_map<std::string, std::size_t> field_slots;
        std::unordered_map<std::string, std::size_t> method_slots;
        std::vector<std::size_t> vtable;
    };

    struct CompiledProgram {
        std::vector<CompiledFunction> functions;
        std::unordered_map<std::string, std::size_t> function_ids;
        std::unordered_map<std::string, CompiledType> types;
        std::unordered_map<int, const CompiledType*> types_by_id;
        std::unordered_map<std::string, Word> data;
    };

    CompiledProgram compile_program(const Banner::BannerProgram& program);
    std::unordered_map<std::string, std::size_t>
    consistent_field_slots(const std::unordered_map<std::string, CompiledType>& types) const;
    std::unordered_map<std::string, std::size_t>
    consistent_method_slots(const std::unordered_map<std::string, CompiledType>& types) const;
    void compile_function_code(CompiledProgram& program,
                               CompiledFunction& function,
                               const std::unordered_map<std::string, std::size_t>& field_slots,
                               const std::unordered_map<std::string, std::size_t>& method_slots) const;
    Frame make_frame(const CompiledFunction& function,
                     const std::vector<Word>& args,
                     std::size_t return_slot = 0,
                     bool has_return_slot = false) const;
    std::unordered_map<std::string, CompiledType>
    compile_types(const Banner::BannerProgram& program,
                  const std::unordered_map<std::string, std::size_t>& function_ids) const;
    std::size_t slot_of(const CompiledFunction& function, const std::string& name) const;
    std::size_t label_of(const CompiledFunction& function, const std::string& label) const;
    const CompiledType& type_of(const CompiledProgram& program, const std::string& name) const;
    const CompiledType& type_of(const CompiledProgram& program, Word value) const;
    std::size_t field_slot(const CompiledType& type, const std::string& field_name) const;
    std::size_t method_slot(const CompiledType& type, const std::string& method_name) const;
    bool is_instance(Word value, const std::string& type_name, const CompiledProgram& program) const;
    std::vector<Word> gc_roots(const std::vector<Frame>& stack,
                               const CompiledProgram& program) const;
    void collect_if_needed(const std::vector<Frame>& stack, const CompiledProgram& program);
    std::string format_compiled_instr(const CompiledInstr& instr) const;
    std::string format_runtime_error(const std::string& cause,
                                     const std::vector<Frame>& stack,
                                     const CompiledInstr& instr,
                                     std::size_t pc) const;
    void enforce_frame_limit(std::size_t next_size, const VMOptions& options) const;
    void enforce_heap_limit(const VMOptions& options) const;
    [[noreturn]] void unsupported(const std::string& feature) const;

    VMHeap heap_;
};

}

#endif
