#include "banner_vm.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace Hulk::VM {
namespace {

bool word_equal(Word a, Word b, const VMHeap& heap) {
    if (is_number(a) && is_number(b)) return as_number(a) == as_number(b);
    if (is_bool(a) && is_bool(b)) return as_bool(a) == as_bool(b);
    if (is_string(a) && is_string(b)) return heap.string_value(a) == heap.string_value(b);
    if (is_nil(a) && is_nil(b)) return true;
    if (is_object(a) && is_object(b)) return a == b;
    return false;
}

std::string slot_ref(std::size_t slot) {
    return "s" + std::to_string(slot);
}

void append_slot_list(std::ostringstream& out, const std::vector<std::size_t>& slots) {
    out << "(";
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (i > 0) out << ", ";
        out << slot_ref(slots.at(i));
    }
    out << ")";
}

std::string format_source_ref(const IR::SourceSpan& source) {
    std::ostringstream out;
    out << (source.file.empty() ? "<desconocido>" : source.file)
        << ":" << source.span.start.line
        << ":" << source.span.start.column;
    return out.str();
}

}

Word BannerVM::run(const Banner::BannerProgram& program, const VMOptions& options) {
    heap_ = VMHeap{};
    CompiledProgram compiled = compile_program(program);
    enforce_heap_limit(options);

    auto entry = compiled.function_ids.find(program.entry_function);
    if (entry == compiled.function_ids.end()) {
        throw std::runtime_error("Runtime error: funcion de entrada no encontrada.");
    }

    std::vector<Frame> stack;
    enforce_frame_limit(1, options);
    stack.push_back(make_frame(compiled.functions.at(entry->second), {}));
    std::uint64_t steps = 0;

    while (!stack.empty()) {
        Frame& frame = stack.back();
        const auto& code = frame.function->code;

        auto get = [&](std::size_t slot) -> Word {
            return frame.slots.at(slot);
        };
        auto set = [&](std::size_t slot, Word value) {
            frame.slots.at(slot) = value;
        };

        if (frame.pc >= code.size()) {
            const Word result = make_nil();
            const bool has_return_slot = frame.has_return_slot;
            const std::size_t return_slot = frame.return_slot;
            stack.pop_back();
            if (stack.empty()) return result;
            if (has_return_slot) {
                stack.back().slots.at(return_slot) = result;
            }
            continue;
        }

        const std::size_t instr_pc = frame.pc;
        const auto& instr = code.at(instr_pc);
        ++frame.pc;
        try {
            if (++steps > options.max_steps) {
                throw std::runtime_error("Runtime error: limite de instrucciones de VM excedido.");
            }
            switch (instr.op) {
            case Banner::Op::Nop:
            case Banner::Op::Label:
                break;
            case Banner::Op::ConstNil:
                set(instr.dest_slot, make_nil());
                break;
            case Banner::Op::ConstNumber:
                set(instr.dest_slot, make_number(instr.number_value));
                break;
            case Banner::Op::ConstBool:
                set(instr.dest_slot, make_bool(instr.bool_value));
                break;
            case Banner::Op::LoadData: {
                auto it = compiled.data.find(instr.data_label);
                if (it == compiled.data.end()) {
                    throw std::runtime_error("Runtime error: string data no encontrado.");
                }
                set(instr.dest_slot, it->second);
                break;
            }
            case Banner::Op::Move:
                set(instr.dest_slot, get(instr.src1_slot));
                break;
            case Banner::Op::Add:
                set(instr.dest_slot, make_number(as_number(get(instr.src1_slot)) +
                                                 as_number(get(instr.src2_slot))));
                break;
            case Banner::Op::Sub:
                set(instr.dest_slot, make_number(as_number(get(instr.src1_slot)) -
                                                 as_number(get(instr.src2_slot))));
                break;
            case Banner::Op::Mul:
                set(instr.dest_slot, make_number(as_number(get(instr.src1_slot)) *
                                                 as_number(get(instr.src2_slot))));
                break;
            case Banner::Op::Div: {
                const double rhs = as_number(get(instr.src2_slot));
                if (rhs == 0) throw std::runtime_error("Runtime error: division por cero.");
                set(instr.dest_slot, make_number(as_number(get(instr.src1_slot)) / rhs));
                break;
            }
            case Banner::Op::Mod: {
                const double rhs = as_number(get(instr.src2_slot));
                if (rhs == 0) throw std::runtime_error("Runtime error: modulo por cero.");
                set(instr.dest_slot, make_number(std::fmod(as_number(get(instr.src1_slot)), rhs)));
                break;
            }
            case Banner::Op::Pow:
                set(instr.dest_slot, make_number(std::pow(as_number(get(instr.src1_slot)),
                                                          as_number(get(instr.src2_slot)))));
                break;
            case Banner::Op::Neg:
                set(instr.dest_slot, make_number(-as_number(get(instr.src1_slot))));
                break;
            case Banner::Op::And:
                set(instr.dest_slot, make_bool(truthy(get(instr.src1_slot)) &&
                                               truthy(get(instr.src2_slot))));
                break;
            case Banner::Op::Or:
                set(instr.dest_slot, make_bool(truthy(get(instr.src1_slot)) ||
                                               truthy(get(instr.src2_slot))));
                break;
            case Banner::Op::Not:
                set(instr.dest_slot, make_bool(!truthy(get(instr.src1_slot))));
                break;
            case Banner::Op::Equal:
                set(instr.dest_slot, make_bool(word_equal(get(instr.src1_slot),
                                                          get(instr.src2_slot),
                                                          heap_)));
                break;
            case Banner::Op::NotEqual:
                set(instr.dest_slot, make_bool(!word_equal(get(instr.src1_slot),
                                                           get(instr.src2_slot),
                                                           heap_)));
                break;
            case Banner::Op::Less:
                set(instr.dest_slot, make_bool(as_number(get(instr.src1_slot)) <
                                               as_number(get(instr.src2_slot))));
                break;
            case Banner::Op::Greater:
                set(instr.dest_slot, make_bool(as_number(get(instr.src1_slot)) >
                                               as_number(get(instr.src2_slot))));
                break;
            case Banner::Op::LessEqual:
                set(instr.dest_slot, make_bool(as_number(get(instr.src1_slot)) <=
                                               as_number(get(instr.src2_slot))));
                break;
            case Banner::Op::GreaterEqual:
                set(instr.dest_slot, make_bool(as_number(get(instr.src1_slot)) >=
                                               as_number(get(instr.src2_slot))));
                break;
            case Banner::Op::Concat: {
                set(instr.dest_slot, heap_.allocate_string(to_string(get(instr.src1_slot), heap_) +
                                                           to_string(get(instr.src2_slot), heap_)));
                collect_if_needed(stack, compiled);
                enforce_heap_limit(options);
                break;
            }
            case Banner::Op::ConcatSpace: {
                set(instr.dest_slot, heap_.allocate_string(to_string(get(instr.src1_slot), heap_) +
                                                           " " +
                                                           to_string(get(instr.src2_slot), heap_)));
                collect_if_needed(stack, compiled);
                enforce_heap_limit(options);
                break;
            }
            case Banner::Op::Jump:
                frame.pc = instr.label_pc;
                break;
            case Banner::Op::JumpIfTrue:
                if (truthy(get(instr.src1_slot))) frame.pc = instr.label_pc;
                break;
            case Banner::Op::JumpIfFalse:
                if (!truthy(get(instr.src1_slot))) frame.pc = instr.label_pc;
                break;
            case Banner::Op::Return: {
                const Word result = get(instr.src1_slot);
                const bool has_return_slot = frame.has_return_slot;
                const std::size_t return_slot = frame.return_slot;
                stack.pop_back();
                if (stack.empty()) return result;
                if (has_return_slot) {
                    stack.back().slots.at(return_slot) = result;
                }
                break;
            }
            case Banner::Op::Print: {
                const Word value = get(instr.arg_slots.at(0));
                std::cout << to_string(value, heap_) << "\n";
                set(instr.dest_slot, value);
                break;
            }
            case Banner::Op::Sqrt:
                set(instr.dest_slot, make_number(std::sqrt(as_number(get(instr.arg_slots.at(0))))));
                break;
            case Banner::Op::Sin:
                set(instr.dest_slot, make_number(std::sin(as_number(get(instr.arg_slots.at(0))))));
                break;
            case Banner::Op::Cos:
                set(instr.dest_slot, make_number(std::cos(as_number(get(instr.arg_slots.at(0))))));
                break;
            case Banner::Op::Exp:
                set(instr.dest_slot, make_number(std::exp(as_number(get(instr.arg_slots.at(0))))));
                break;
            case Banner::Op::Log:
                set(instr.dest_slot, make_number(std::log(as_number(get(instr.arg_slots.at(1)))) /
                                                 std::log(as_number(get(instr.arg_slots.at(0))))));
                break;
            case Banner::Op::Rand:
                set(instr.dest_slot, make_number(static_cast<double>(std::rand()) / RAND_MAX));
                break;
            case Banner::Op::Param:
                frame.param_buffer.push_back(get(instr.src1_slot));
                break;
            case Banner::Op::Call: {
                std::vector<Word> args = frame.param_buffer;
                frame.param_buffer.clear();
                enforce_frame_limit(stack.size() + 1, options);
                stack.push_back(make_frame(compiled.functions.at(instr.callee_id),
                                           args,
                                           instr.dest_slot,
                                           instr.has_dest));
                break;
            }
            case Banner::Op::Allocate:
                set(instr.dest_slot, heap_.allocate_object(instr.type_id,
                                                           type_of(compiled, instr.type_name)
                                                               .field_slots.size()));
                collect_if_needed(stack, compiled);
                enforce_heap_limit(options);
                break;
            case Banner::Op::GetAttr: {
                const Word receiver = get(instr.src1_slot);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                const auto& type = type_of(compiled, receiver);
                const std::size_t slot = instr.has_field_slot
                                             ? instr.field_slot
                                             : field_slot(type, instr.field_name);
                set(instr.dest_slot, heap_.object(receiver).fields.at(slot));
                break;
            }
            case Banner::Op::SetAttr: {
                const Word receiver = get(instr.src1_slot);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                const auto& type = type_of(compiled, receiver);
                const std::size_t slot = instr.has_field_slot
                                             ? instr.field_slot
                                             : field_slot(type, instr.field_name);
                const Word value = get(instr.src2_slot);
                heap_.object(receiver).fields.at(slot) = value;
                if (instr.has_dest) {
                    set(instr.dest_slot, value);
                }
                break;
            }
            case Banner::Op::VCall: {
                std::vector<Word> args = frame.param_buffer;
                frame.param_buffer.clear();
                const Word receiver = get(instr.src1_slot);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                if (args.empty()) {
                    for (const auto slot : instr.arg_slots) {
                        args.push_back(get(slot));
                    }
                }
                const auto& runtime_type = type_of(compiled, receiver);
                const std::size_t slot = instr.has_method_slot
                                             ? instr.method_slot
                                             : method_slot(runtime_type, instr.method_name);
                std::vector<Word> call_args;
                call_args.reserve(args.size() + 1);
                call_args.push_back(receiver);
                call_args.insert(call_args.end(), args.begin(), args.end());
                enforce_frame_limit(stack.size() + 1, options);
                stack.push_back(make_frame(compiled.functions.at(runtime_type.vtable.at(slot)),
                                           call_args,
                                           instr.dest_slot,
                                           instr.has_dest));
                break;
            }
            case Banner::Op::SCall: {
                std::vector<Word> args = frame.param_buffer;
                frame.param_buffer.clear();
                const Word receiver = get(instr.src1_slot);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                if (args.empty()) {
                    for (const auto slot : instr.arg_slots) {
                        args.push_back(get(slot));
                    }
                }
                const auto& start_type = type_of(compiled, instr.type_name);
                const std::size_t slot = instr.has_method_slot
                                             ? instr.method_slot
                                             : method_slot(start_type, instr.method_name);
                std::vector<Word> call_args;
                call_args.reserve(args.size() + 1);
                call_args.push_back(receiver);
                call_args.insert(call_args.end(), args.begin(), args.end());
                enforce_frame_limit(stack.size() + 1, options);
                stack.push_back(make_frame(compiled.functions.at(start_type.vtable.at(slot)),
                                           call_args,
                                           instr.dest_slot,
                                           instr.has_dest));
                break;
            }
            case Banner::Op::IsType:
                set(instr.dest_slot, make_bool(is_instance(get(instr.src1_slot),
                                                           instr.type_name,
                                                           compiled)));
                break;
            case Banner::Op::AsType: {
                const Word value = get(instr.src1_slot);
                if (!is_instance(value, instr.type_name, compiled)) {
                    throw std::runtime_error("Runtime error: no se puede castear '" +
                                             to_string(value, heap_) + "' a '" +
                                             instr.type_name + "'.");
                }
                set(instr.dest_slot, value);
                break;
            }
            }
        } catch (const std::exception& err) {
            throw std::runtime_error(format_runtime_error(err.what(), stack, instr, instr_pc));
        }
    }

    return make_nil();
}

std::string BannerVM::compiled_view(const Banner::BannerProgram& program) {
    heap_ = VMHeap{};
    const CompiledProgram compiled = compile_program(program);

    std::ostringstream out;
    out << ".COMPILED_BANNER\n";
    out << "entry function #" << compiled.function_ids.at(program.entry_function)
        << " " << program.entry_function << "\n";

    for (std::size_t function_id = 0; function_id < compiled.functions.size(); ++function_id) {
        const auto& function = compiled.functions.at(function_id);
        out << "\nfunction #" << function_id << " " << function.function->name;
        if (!function.function->source_name.empty() &&
            function.function->source_name != function.function->name) {
            out << " ; source=" << function.function->source_name;
        }
        out << "\n";

        std::vector<std::pair<std::string, std::size_t>> slots(function.slots.begin(),
                                                              function.slots.end());
        std::sort(slots.begin(),
                  slots.end(),
                  [](const auto& left, const auto& right) {
                      if (left.second != right.second) return left.second < right.second;
                      return left.first < right.first;
                  });

        out << "  slots:\n";
        for (const auto& [name, slot] : slots) {
            out << "    " << slot_ref(slot) << " = " << name << "\n";
        }

        out << "  code:\n";
        for (std::size_t pc = 0; pc < function.code.size(); ++pc) {
            const auto& instr = function.code.at(pc);
            out << "    " << pc << ": " << format_compiled_instr(instr);
            if (instr.source) out << " ; source=" << format_source_ref(*instr.source);
            out << "\n";
        }
    }

    if (!compiled.types.empty()) {
        out << "\ntypes:\n";
        std::vector<const CompiledType*> types;
        types.reserve(compiled.types.size());
        for (const auto& [_, type] : compiled.types) {
            types.push_back(&type);
        }
        std::sort(types.begin(),
                  types.end(),
                  [](const auto* left, const auto* right) {
                      return left->type_id < right->type_id;
                  });

        for (const auto* type : types) {
            out << "  type #" << type->type_id << " " << type->type->name;
            if (type->parent_type_id != -1) out << " parent=#" << type->parent_type_id;
            out << "\n";
        }
    }

    if (!compiled.data.empty()) {
        out << "\ndata:\n";
        std::vector<std::string> labels;
        labels.reserve(compiled.data.size());
        for (const auto& [label, _] : compiled.data) {
            labels.push_back(label);
        }
        std::sort(labels.begin(), labels.end());
        for (const auto& label : labels) {
            out << "  " << label << " = <string>\n";
        }
    }

    return out.str();
}

BannerVM::CompiledProgram BannerVM::compile_program(const Banner::BannerProgram& program) {
    CompiledProgram out;

    out.functions.resize(program.functions.size());
    for (std::size_t i = 0; i < program.functions.size(); ++i) {
        out.function_ids.emplace(program.functions.at(i).name, i);
        out.functions.at(i).function = &program.functions.at(i);
    }

    out.types = compile_types(program, out.function_ids);
    for (const auto& [_, type] : out.types) {
        out.types_by_id.emplace(type.type_id, &type);
    }

    const auto field_slots = consistent_field_slots(out.types);
    const auto method_slots = consistent_method_slots(out.types);
    for (auto& function : out.functions) {
        compile_function_code(out, function, field_slots, method_slots);
    }

    for (const auto& item : program.data) {
        out.data[item.label] = heap_.allocate_string(item.value);
    }

    return out;
}

std::unordered_map<std::string, std::size_t>
BannerVM::consistent_field_slots(const std::unordered_map<std::string, CompiledType>& types) const {
    std::unordered_map<std::string, std::size_t> slots;
    std::unordered_set<std::string> ambiguous;
    for (const auto& [_, type] : types) {
        for (const auto& [name, slot] : type.field_slots) {
            auto existing = slots.find(name);
            if (existing == slots.end()) {
                slots.emplace(name, slot);
            } else if (existing->second != slot) {
                ambiguous.insert(name);
            }
        }
    }
    for (const auto& name : ambiguous) slots.erase(name);
    return slots;
}

std::unordered_map<std::string, std::size_t>
BannerVM::consistent_method_slots(const std::unordered_map<std::string, CompiledType>& types) const {
    std::unordered_map<std::string, std::size_t> slots;
    std::unordered_set<std::string> ambiguous;
    for (const auto& [_, type] : types) {
        for (const auto& [name, slot] : type.method_slots) {
            auto existing = slots.find(name);
            if (existing == slots.end()) {
                slots.emplace(name, slot);
            } else if (existing->second != slot) {
                ambiguous.insert(name);
            }
        }
    }
    for (const auto& name : ambiguous) slots.erase(name);
    return slots;
}

void BannerVM::compile_function_code(
    CompiledProgram& program,
    CompiledFunction& function,
    const std::unordered_map<std::string, std::size_t>& field_slots,
    const std::unordered_map<std::string, std::size_t>& method_slots) const {
    std::size_t next_slot = 0;
    for (const auto& param : function.function->params) {
        function.slots.emplace(param, next_slot++);
    }
    for (const auto& local : function.function->locals) {
        function.slots.emplace(local, next_slot++);
    }

    for (std::size_t pc = 0; pc < function.function->code.size(); ++pc) {
        const auto& instr = function.function->code.at(pc);
        if (instr.op == Banner::Op::Label) {
            function.labels[instr.label] = pc;
        }
    }

    function.code.reserve(function.function->code.size());
    for (const auto& source : function.function->code) {
        CompiledInstr instr;
        instr.op = source.op;
        instr.number_value = source.number_value;
        instr.bool_value = source.bool_value;
        instr.data_label = source.src1;
        instr.field_name = source.field_name;
        instr.method_name = source.method_name;
        instr.type_name = source.type_name;
        instr.has_dest = !source.dest.empty();

        if (!source.dest.empty()) instr.dest_slot = slot_of(function, source.dest);
        if (!source.src1.empty() &&
            source.op != Banner::Op::LoadData &&
            source.op != Banner::Op::JumpIfTrue &&
            source.op != Banner::Op::JumpIfFalse) {
            instr.src1_slot = slot_of(function, source.src1);
        }
        if ((source.op == Banner::Op::JumpIfTrue || source.op == Banner::Op::JumpIfFalse) &&
            !source.src1.empty()) {
            instr.src1_slot = slot_of(function, source.src1);
        }
        if (!source.src2.empty()) instr.src2_slot = slot_of(function, source.src2);
        if (!source.label.empty()) instr.label_pc = label_of(function, source.label);
        if (!source.callee.empty()) {
            auto callee = program.function_ids.find(source.callee);
            if (callee == program.function_ids.end()) {
                throw std::runtime_error("Runtime error: funcion no encontrada: " + source.callee);
            }
            instr.callee_id = callee->second;
        }
        if (!source.type_name.empty() && source.op == Banner::Op::Allocate) {
            instr.type_id = type_of(program, source.type_name).type_id;
        }
        if (!source.field_name.empty()) {
            auto field_it = field_slots.find(source.field_name);
            if (field_it != field_slots.end()) {
                instr.field_slot = field_it->second;
                instr.has_field_slot = true;
            }
        }
        if (!source.method_name.empty()) {
            auto method_it = method_slots.find(source.method_name);
            if (method_it != method_slots.end()) {
                instr.method_slot = method_it->second;
                instr.has_method_slot = true;
            }
            if (source.op == Banner::Op::SCall && !source.type_name.empty()) {
                const auto& type = type_of(program, source.type_name);
                instr.method_slot = method_slot(type, source.method_name);
                instr.has_method_slot = true;
            }
        }
        for (const auto& arg : source.args) {
            instr.arg_slots.push_back(slot_of(function, arg));
        }
        instr.source = source.source;

        function.code.push_back(std::move(instr));
    }
}

BannerVM::Frame BannerVM::make_frame(const CompiledFunction& function,
                                     const std::vector<Word>& args,
                                     std::size_t return_slot,
                                     bool has_return_slot) const {
    if (args.size() != function.function->params.size()) {
        throw std::runtime_error("Runtime error: aridad incorrecta en " +
                                 function.function->source_name + ".");
    }

    Frame frame;
    frame.function = &function;
    frame.pc = 0;
    frame.slots.resize(function.slots.size(), make_nil());
    frame.return_slot = return_slot;
    frame.has_return_slot = has_return_slot;
    for (std::size_t i = 0; i < args.size(); ++i) {
        frame.slots.at(slot_of(function, function.function->params.at(i))) = args.at(i);
    }

    return frame;
}

std::unordered_map<std::string, BannerVM::CompiledType>
BannerVM::compile_types(const Banner::BannerProgram& program,
                        const std::unordered_map<std::string, std::size_t>& function_ids) const {
    std::unordered_map<std::string, const Banner::BannerType*> source_types;
    for (const auto& type : program.types) {
        source_types.emplace(type.name, &type);
    }

    std::unordered_map<std::string, CompiledType> compiled;
    int next_type_id = 0;

    std::function<const CompiledType&(const std::string&)> compile_one =
        [&](const std::string& type_name) -> const CompiledType& {
            auto existing = compiled.find(type_name);
            if (existing != compiled.end()) return existing->second;

            auto source_it = source_types.find(type_name);
            if (source_it == source_types.end()) {
                throw std::runtime_error("Runtime error: tipo no encontrado: " + type_name);
            }

            CompiledType type;
            type.type = source_it->second;
            type.type_id = next_type_id++;

            if (!type.type->parent.empty() && type.type->parent != "Object") {
                const auto& parent = compile_one(type.type->parent);
                type.parent_type_id = parent.type_id;
                type.field_slots = parent.field_slots;
                type.method_slots = parent.method_slots;
                type.vtable = parent.vtable;
            }

            for (const auto& field : type.type->fields) {
                if (type.field_slots.find(field.name) == type.field_slots.end()) {
                    type.field_slots.emplace(field.name, type.field_slots.size());
                }
            }

            for (const auto& method : type.type->methods) {
                auto fn_it = function_ids.find(method.function_name);
                if (fn_it == function_ids.end()) {
                    throw std::runtime_error("Runtime error: metodo no encontrado: " +
                                             method.function_name);
                }
                auto slot_it = type.method_slots.find(method.name);
                if (slot_it == type.method_slots.end()) {
                    const std::size_t slot = type.vtable.size();
                    type.method_slots.emplace(method.name, slot);
                    type.vtable.push_back(fn_it->second);
                } else {
                    type.vtable.at(slot_it->second) = fn_it->second;
                }
            }

            auto [it, _] = compiled.emplace(type_name, std::move(type));
            return it->second;
        };

    for (const auto& type : program.types) {
        (void)compile_one(type.name);
    }

    return compiled;
}

std::size_t BannerVM::slot_of(const CompiledFunction& function, const std::string& name) const {
    auto it = function.slots.find(name);
    if (it == function.slots.end()) {
        throw std::runtime_error("Runtime error: slot local no encontrado: " + name);
    }
    return it->second;
}

std::size_t BannerVM::label_of(const CompiledFunction& function, const std::string& label) const {
    auto it = function.labels.find(label);
    if (it == function.labels.end()) {
        throw std::runtime_error("Runtime error: label no encontrado: " + label);
    }
    return it->second;
}

const BannerVM::CompiledType& BannerVM::type_of(const CompiledProgram& program,
                                                const std::string& name) const {
    auto it = program.types.find(name);
    if (it == program.types.end()) {
        throw std::runtime_error("Runtime error: tipo no encontrado: " + name);
    }
    return it->second;
}

const BannerVM::CompiledType& BannerVM::type_of(const CompiledProgram& program, Word value) const {
    if (!is_object(value)) {
        throw std::runtime_error("Runtime error: se esperaba Object.");
    }
    const int type_id = heap_.object(value).type_id;
    auto it = program.types_by_id.find(type_id);
    if (it == program.types_by_id.end()) {
        throw std::runtime_error("Runtime error: type_id de objeto invalido.");
    }
    return *it->second;
}

std::size_t BannerVM::field_slot(const CompiledType& type, const std::string& field_name) const {
    auto it = type.field_slots.find(field_name);
    if (it == type.field_slots.end()) {
        throw std::runtime_error("Runtime error: atributo '" + field_name +
                                 "' no existe en tipo '" + type.type->name + "'.");
    }
    return it->second;
}

std::size_t BannerVM::method_slot(const CompiledType& type, const std::string& method_name) const {
    auto it = type.method_slots.find(method_name);
    if (it == type.method_slots.end()) {
        throw std::runtime_error("Runtime error: metodo '" + method_name +
                                 "' no existe en tipo '" + type.type->name + "'.");
    }
    return it->second;
}

bool BannerVM::is_instance(Word value,
                           const std::string& type_name,
                           const CompiledProgram& program) const {
    if (!is_object(value)) {
        if (type_name == "Number") return is_number(value);
        if (type_name == "String") return is_string(value);
        if (type_name == "Boolean") return is_bool(value);
        return false;
    }

    const auto& start_type = type_of(program, value);
    const CompiledType* current = &start_type;
    while (current != nullptr) {
        if (current->type->name == type_name) return true;
        if (current->parent_type_id == -1) break;

        auto parent = program.types_by_id.find(current->parent_type_id);
        current = parent == program.types_by_id.end() ? nullptr : parent->second;
    }

    return type_name == "Object";
}

std::vector<Word> BannerVM::gc_roots(const std::vector<Frame>& stack,
                                     const CompiledProgram& program) const {
    std::vector<Word> roots;
    for (const auto& frame : stack) {
        roots.insert(roots.end(), frame.slots.begin(), frame.slots.end());
        roots.insert(roots.end(), frame.param_buffer.begin(), frame.param_buffer.end());
    }
    for (const auto& [_, value] : program.data) {
        roots.push_back(value);
    }
    return roots;
}

void BannerVM::collect_if_needed(const std::vector<Frame>& stack,
                                 const CompiledProgram& program) {
    if (!heap_.should_collect()) return;
    heap_.collect(gc_roots(stack, program));
}

std::string BannerVM::format_compiled_instr(const CompiledInstr& instr) const {
    std::ostringstream out;
    auto dest = [&] {
        if (instr.has_dest) out << slot_ref(instr.dest_slot) << " = ";
    };
    auto binary = [&](const char* name) {
        dest();
        out << name << " " << slot_ref(instr.src1_slot) << ", " << slot_ref(instr.src2_slot);
    };
    auto unary = [&](const char* name) {
        dest();
        out << name << " " << slot_ref(instr.src1_slot);
    };
    auto builtin = [&](const char* name) {
        dest();
        out << name << " ";
        append_slot_list(out, instr.arg_slots);
    };

    switch (instr.op) {
        case Banner::Op::Nop:
            out << "NOP";
            break;
        case Banner::Op::Label:
            out << "LABEL pc=" << instr.label_pc;
            break;
        case Banner::Op::ConstNil:
            dest();
            out << "CONST_NIL";
            break;
        case Banner::Op::ConstNumber:
            dest();
            out << "CONST_NUMBER " << instr.number_value;
            break;
        case Banner::Op::ConstBool:
            dest();
            out << "CONST_BOOL " << (instr.bool_value ? "true" : "false");
            break;
        case Banner::Op::LoadData:
            dest();
            out << "LOAD_DATA " << instr.data_label;
            break;
        case Banner::Op::Move:
            unary("MOVE");
            break;
        case Banner::Op::Add:
            binary("ADD");
            break;
        case Banner::Op::Sub:
            binary("SUB");
            break;
        case Banner::Op::Mul:
            binary("MUL");
            break;
        case Banner::Op::Div:
            binary("DIV");
            break;
        case Banner::Op::Mod:
            binary("MOD");
            break;
        case Banner::Op::Pow:
            binary("POW");
            break;
        case Banner::Op::Neg:
            unary("NEG");
            break;
        case Banner::Op::And:
            binary("AND");
            break;
        case Banner::Op::Or:
            binary("OR");
            break;
        case Banner::Op::Not:
            unary("NOT");
            break;
        case Banner::Op::Equal:
            binary("EQUAL");
            break;
        case Banner::Op::NotEqual:
            binary("NOT_EQUAL");
            break;
        case Banner::Op::Less:
            binary("LESS");
            break;
        case Banner::Op::Greater:
            binary("GREATER");
            break;
        case Banner::Op::LessEqual:
            binary("LESS_EQUAL");
            break;
        case Banner::Op::GreaterEqual:
            binary("GREATER_EQUAL");
            break;
        case Banner::Op::Concat:
            binary("CONCAT");
            break;
        case Banner::Op::ConcatSpace:
            binary("CONCAT_SPACE");
            break;
        case Banner::Op::Jump:
            out << "JUMP pc=" << instr.label_pc;
            break;
        case Banner::Op::JumpIfTrue:
            out << "JUMP_IF_TRUE " << slot_ref(instr.src1_slot) << " pc=" << instr.label_pc;
            break;
        case Banner::Op::JumpIfFalse:
            out << "JUMP_IF_FALSE " << slot_ref(instr.src1_slot) << " pc=" << instr.label_pc;
            break;
        case Banner::Op::Param:
            out << "PARAM " << slot_ref(instr.src1_slot);
            break;
        case Banner::Op::Call:
            dest();
            out << "CALL function #" << instr.callee_id;
            break;
        case Banner::Op::Return:
            out << "RETURN " << slot_ref(instr.src1_slot);
            break;
        case Banner::Op::Allocate:
            dest();
            out << "ALLOCATE type #" << instr.type_id << " " << instr.type_name;
            break;
        case Banner::Op::GetAttr:
            dest();
            out << "GET_ATTR " << slot_ref(instr.src1_slot);
            out << "." << (instr.has_field_slot ? "#" + std::to_string(instr.field_slot)
                                                : instr.field_name);
            break;
        case Banner::Op::SetAttr:
            dest();
            out << "SET_ATTR " << slot_ref(instr.src1_slot);
            out << "." << (instr.has_field_slot ? "#" + std::to_string(instr.field_slot)
                                                : instr.field_name);
            out << ", " << slot_ref(instr.src2_slot);
            break;
        case Banner::Op::VCall:
            dest();
            out << "VCALL " << slot_ref(instr.src1_slot) << ".";
            out << (instr.has_method_slot ? "#" + std::to_string(instr.method_slot)
                                          : instr.method_name);
            if (!instr.arg_slots.empty()) {
                out << " ";
                append_slot_list(out, instr.arg_slots);
            }
            break;
        case Banner::Op::SCall:
            dest();
            out << "SCALL " << instr.type_name << "::";
            out << (instr.has_method_slot ? "#" + std::to_string(instr.method_slot)
                                          : instr.method_name);
            out << " receiver=" << slot_ref(instr.src1_slot);
            if (!instr.arg_slots.empty()) {
                out << " ";
                append_slot_list(out, instr.arg_slots);
            }
            break;
        case Banner::Op::IsType:
            dest();
            out << "IS_TYPE " << slot_ref(instr.src1_slot) << ", " << instr.type_name;
            break;
        case Banner::Op::AsType:
            dest();
            out << "AS_TYPE " << slot_ref(instr.src1_slot) << ", " << instr.type_name;
            break;
        case Banner::Op::Print:
            builtin("PRINT");
            break;
        case Banner::Op::Sqrt:
            builtin("SQRT");
            break;
        case Banner::Op::Sin:
            builtin("SIN");
            break;
        case Banner::Op::Cos:
            builtin("COS");
            break;
        case Banner::Op::Exp:
            builtin("EXP");
            break;
        case Banner::Op::Log:
            builtin("LOG");
            break;
        case Banner::Op::Rand:
            dest();
            out << "RAND";
            break;
    }

    return out.str();
}

std::string BannerVM::format_runtime_error(const std::string& cause,
                                           const std::vector<Frame>& stack,
                                           const CompiledInstr& instr,
                                           std::size_t pc) const {
    const Frame* current = stack.empty() ? nullptr : &stack.back();
    const std::string function_name =
        current != nullptr && current->function != nullptr && current->function->function != nullptr
            ? current->function->function->name
            : "<sin-funcion>";

    std::ostringstream out;
    out << "Runtime error en " << function_name << " pc=" << pc << "\n";
    if (instr.source) {
        out << "  source: " << format_source_ref(*instr.source) << "\n";
    }
    out << "  instr: " << format_compiled_instr(instr) << "\n";
    out << "  causa: " << cause << "\n";
    out << "  stack:\n";
    for (std::size_t i = stack.size(); i > 0; --i) {
        const auto& frame = stack.at(i - 1);
        const std::string name =
            frame.function != nullptr && frame.function->function != nullptr
                ? frame.function->function->name
                : "<sin-funcion>";
        const std::size_t frame_pc = (i == stack.size()) ? pc : frame.pc;
        out << "    at " << name << " pc=" << frame_pc << "\n";
    }
    return out.str();
}

void BannerVM::enforce_frame_limit(std::size_t next_size, const VMOptions& options) const {
    if (next_size > options.max_frames) {
        throw std::runtime_error("Runtime error: limite de frames de VM excedido.");
    }
}

void BannerVM::enforce_heap_limit(const VMOptions& options) const {
    const std::size_t live_values = heap_.live_object_count() + heap_.live_string_count();
    if (live_values > options.max_heap_values) {
        throw std::runtime_error("Runtime error: limite de heap de VM excedido.");
    }
}

[[noreturn]] void BannerVM::unsupported(const std::string& feature) const {
    throw std::runtime_error("Runtime error: BannerVM aun no soporta " + feature + ".");
}

}
