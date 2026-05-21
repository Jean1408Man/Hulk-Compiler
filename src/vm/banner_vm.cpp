#include "banner_vm.h"

#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
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

}

Word BannerVM::run(const Banner::BannerProgram& program) {
    heap_ = VMHeap{};
    CompiledProgram compiled = compile_program(program);

    auto entry = compiled.function_ids.find(program.entry_function);
    if (entry == compiled.function_ids.end()) {
        throw std::runtime_error("Runtime error: funcion de entrada no encontrada.");
    }

    std::vector<Frame> stack;
    stack.push_back(make_frame(compiled.functions.at(entry->second), {}));

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

        const auto& instr = code[frame.pc++];
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
                break;
            }
            case Banner::Op::ConcatSpace: {
                set(instr.dest_slot, heap_.allocate_string(to_string(get(instr.src1_slot), heap_) +
                                                           " " +
                                                           to_string(get(instr.src2_slot), heap_)));
                collect_if_needed(stack, compiled);
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
    }

    return make_nil();
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

[[noreturn]] void BannerVM::unsupported(const std::string& feature) const {
    throw std::runtime_error("Runtime error: BannerVM aun no soporta " + feature + ".");
}

}
