#include "banner_vm.h"

#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace Hulk::VM {

VMValue BannerVM::run(const Banner::BannerProgram& program) {
    std::unordered_map<std::string, std::string> data;
    for (const auto& item : program.data) {
        data[item.label] = item.value;
    }

    std::unordered_map<std::string, CompiledFunction> functions;
    for (const auto& function : program.functions) {
        functions.emplace(function.name, compile_function(function));
    }
    const auto types = compile_types(program);

    auto entry = functions.find(program.entry_function);
    if (entry == functions.end()) {
        throw std::runtime_error("Runtime error: funcion de entrada no encontrada.");
    }

    std::vector<Frame> stack;
    stack.push_back(make_frame(entry->second, {}));

    while (!stack.empty()) {
        Frame& frame = stack.back();
        const auto& code = frame.function->function->code;

        auto get = [&](const std::string& name) -> const VMValue& {
            return frame.slots.at(slot_of(*frame.function, name));
        };
        auto set = [&](const std::string& name, VMValue value) {
            frame.slots.at(slot_of(*frame.function, name)) = std::move(value);
        };

        if (frame.pc >= code.size()) {
            VMValue result;
            const bool has_return_dest = frame.has_return_dest;
            const std::string return_dest = frame.return_dest;
            stack.pop_back();
            if (stack.empty()) return result;
            Frame& caller = stack.back();
            if (has_return_dest) {
                caller.slots.at(slot_of(*caller.function, return_dest)) = std::move(result);
            }
            continue;
        }

        const auto& instr = code[frame.pc++];
        switch (instr.op) {
            case Banner::Op::Nop:
            case Banner::Op::Label:
                break;
            case Banner::Op::ConstNil:
                set(instr.dest, VMValue());
                break;
            case Banner::Op::ConstNumber:
                set(instr.dest, VMValue(instr.number_value));
                break;
            case Banner::Op::ConstBool:
                set(instr.dest, VMValue(instr.bool_value));
                break;
            case Banner::Op::LoadData: {
                auto it = data.find(instr.src1);
                if (it == data.end()) throw std::runtime_error("Runtime error: string data no encontrado.");
                set(instr.dest, VMValue(it->second));
                break;
            }
            case Banner::Op::Move:
                set(instr.dest, get(instr.src1));
                break;
            case Banner::Op::Add:
                set(instr.dest, VMValue(as_number(get(instr.src1)) + as_number(get(instr.src2))));
                break;
            case Banner::Op::Sub:
                set(instr.dest, VMValue(as_number(get(instr.src1)) - as_number(get(instr.src2))));
                break;
            case Banner::Op::Mul:
                set(instr.dest, VMValue(as_number(get(instr.src1)) * as_number(get(instr.src2))));
                break;
            case Banner::Op::Div: {
                const double rhs = as_number(get(instr.src2));
                if (rhs == 0) throw std::runtime_error("Runtime error: division por cero.");
                set(instr.dest, VMValue(as_number(get(instr.src1)) / rhs));
                break;
            }
            case Banner::Op::Mod: {
                const double rhs = as_number(get(instr.src2));
                if (rhs == 0) throw std::runtime_error("Runtime error: modulo por cero.");
                set(instr.dest, VMValue(std::fmod(as_number(get(instr.src1)), rhs)));
                break;
            }
            case Banner::Op::Pow:
                set(instr.dest, VMValue(std::pow(as_number(get(instr.src1)), as_number(get(instr.src2)))));
                break;
            case Banner::Op::Neg:
                set(instr.dest, VMValue(-as_number(get(instr.src1))));
                break;
            case Banner::Op::And:
                set(instr.dest, VMValue(truthy(get(instr.src1)) && truthy(get(instr.src2))));
                break;
            case Banner::Op::Or:
                set(instr.dest, VMValue(truthy(get(instr.src1)) || truthy(get(instr.src2))));
                break;
            case Banner::Op::Not:
                set(instr.dest, VMValue(!truthy(get(instr.src1))));
                break;
            case Banner::Op::Equal: {
                const auto& a = get(instr.src1);
                const auto& b = get(instr.src2);
                bool result = false;
                if (is_number(a) && is_number(b)) result = as_number(a) == as_number(b);
                else if (is_bool(a) && is_bool(b)) result = as_bool(a) == as_bool(b);
                else if (is_string(a) && is_string(b)) result = as_string(a) == as_string(b);
                else if (is_nil(a) && is_nil(b)) result = true;
                set(instr.dest, VMValue(result));
                break;
            }
            case Banner::Op::NotEqual: {
                const auto& a = get(instr.src1);
                const auto& b = get(instr.src2);
                bool equal = false;
                if (is_number(a) && is_number(b)) equal = as_number(a) == as_number(b);
                else if (is_bool(a) && is_bool(b)) equal = as_bool(a) == as_bool(b);
                else if (is_string(a) && is_string(b)) equal = as_string(a) == as_string(b);
                else if (is_nil(a) && is_nil(b)) equal = true;
                set(instr.dest, VMValue(!equal));
                break;
            }
            case Banner::Op::Less:
                set(instr.dest, VMValue(as_number(get(instr.src1)) < as_number(get(instr.src2))));
                break;
            case Banner::Op::Greater:
                set(instr.dest, VMValue(as_number(get(instr.src1)) > as_number(get(instr.src2))));
                break;
            case Banner::Op::LessEqual:
                set(instr.dest, VMValue(as_number(get(instr.src1)) <= as_number(get(instr.src2))));
                break;
            case Banner::Op::GreaterEqual:
                set(instr.dest, VMValue(as_number(get(instr.src1)) >= as_number(get(instr.src2))));
                break;
            case Banner::Op::Concat:
                set(instr.dest, VMValue(to_string(get(instr.src1)) + to_string(get(instr.src2))));
                break;
            case Banner::Op::ConcatSpace:
                set(instr.dest, VMValue(to_string(get(instr.src1)) + " " + to_string(get(instr.src2))));
                break;
            case Banner::Op::Jump:
                frame.pc = label_of(*frame.function, instr.label);
                break;
            case Banner::Op::JumpIfTrue:
                if (truthy(get(instr.src1))) frame.pc = label_of(*frame.function, instr.label);
                break;
            case Banner::Op::JumpIfFalse:
                if (!truthy(get(instr.src1))) frame.pc = label_of(*frame.function, instr.label);
                break;
            case Banner::Op::Return: {
                VMValue result = get(instr.src1);
                const bool has_return_dest = frame.has_return_dest;
                const std::string return_dest = frame.return_dest;
                stack.pop_back();
                if (stack.empty()) return result;
                Frame& caller = stack.back();
                if (has_return_dest) {
                    caller.slots.at(slot_of(*caller.function, return_dest)) = std::move(result);
                }
                break;
            }
            case Banner::Op::Print: {
                VMValue value = get(instr.args.at(0));
                std::cout << to_string(value) << "\n";
                set(instr.dest, value);
                break;
            }
            case Banner::Op::Sqrt:
                set(instr.dest, VMValue(std::sqrt(as_number(get(instr.args.at(0))))));
                break;
            case Banner::Op::Sin:
                set(instr.dest, VMValue(std::sin(as_number(get(instr.args.at(0))))));
                break;
            case Banner::Op::Cos:
                set(instr.dest, VMValue(std::cos(as_number(get(instr.args.at(0))))));
                break;
            case Banner::Op::Exp:
                set(instr.dest, VMValue(std::exp(as_number(get(instr.args.at(0))))));
                break;
            case Banner::Op::Log:
                set(instr.dest, VMValue(std::log(as_number(get(instr.args.at(1)))) /
                                        std::log(as_number(get(instr.args.at(0))))));
                break;
            case Banner::Op::Rand:
                set(instr.dest, VMValue(static_cast<double>(std::rand()) / RAND_MAX));
                break;
            case Banner::Op::Param:
                frame.param_buffer.push_back(get(instr.src1));
                break;
            case Banner::Op::Call: {
                auto callee = functions.find(instr.callee);
                if (callee == functions.end()) {
                    throw std::runtime_error("Runtime error: funcion no encontrada: " + instr.callee);
                }
                std::vector<VMValue> args = frame.param_buffer;
                frame.param_buffer.clear();
                stack.push_back(make_frame(callee->second, args, instr.dest));
                break;
            }
            case Banner::Op::Allocate: {
                const auto& type = type_of(types, instr.type_name);
                set(instr.dest, VMValue(heap_.allocate_object(type.type_id, type.field_slots.size())));
                break;
            }
            case Banner::Op::GetAttr: {
                const VMValue& receiver = get(instr.src1);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                auto object = std::get<VMObjectRef>(receiver.inner);
                const auto& type = type_of(types, receiver);
                set(instr.dest, object->fields.at(field_slot(type, instr.field_name)));
                break;
            }
            case Banner::Op::SetAttr: {
                const VMValue& receiver = get(instr.src1);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                auto object = std::get<VMObjectRef>(receiver.inner);
                const auto& type = type_of(types, receiver);
                VMValue value = get(instr.src2);
                object->fields.at(field_slot(type, instr.field_name)) = value;
                if (!instr.dest.empty()) {
                    set(instr.dest, value);
                }
                break;
            }
            case Banner::Op::VCall: {
                std::vector<VMValue> args = frame.param_buffer;
                frame.param_buffer.clear();
                VMValue receiver = get(instr.src1);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                if (args.empty()) {
                    for (const auto& arg_name : instr.args) {
                        args.push_back(get(arg_name));
                    }
                }
                const auto& runtime_type = type_of(types, receiver);
                const std::size_t slot = method_slot(runtime_type, instr.method_name);
                auto callee = functions.find(runtime_type.vtable.at(slot));
                if (callee == functions.end()) {
                    throw std::runtime_error("Runtime error: metodo no encontrado: " + instr.method_name);
                }
                std::vector<VMValue> call_args;
                call_args.reserve(args.size() + 1);
                call_args.push_back(receiver);
                call_args.insert(call_args.end(), args.begin(), args.end());
                stack.push_back(make_frame(callee->second, call_args, instr.dest));
                break;
            }
            case Banner::Op::SCall: {
                std::vector<VMValue> args = frame.param_buffer;
                frame.param_buffer.clear();
                VMValue receiver = get(instr.src1);
                if (!is_object(receiver)) {
                    throw std::runtime_error("Runtime error: se esperaba Object.");
                }
                if (args.empty()) {
                    for (const auto& arg_name : instr.args) {
                        args.push_back(get(arg_name));
                    }
                }
                const auto& start_type = type_of(types, instr.type_name);
                const std::size_t slot = method_slot(start_type, instr.method_name);
                auto callee = functions.find(start_type.vtable.at(slot));
                if (callee == functions.end()) {
                    throw std::runtime_error("Runtime error: metodo no encontrado: " + instr.method_name);
                }
                std::vector<VMValue> call_args;
                call_args.reserve(args.size() + 1);
                call_args.push_back(receiver);
                call_args.insert(call_args.end(), args.begin(), args.end());
                stack.push_back(make_frame(callee->second, call_args, instr.dest));
                break;
            }
            case Banner::Op::IsType:
                set(instr.dest, VMValue(is_instance(get(instr.src1), instr.type_name, types)));
                break;
            case Banner::Op::AsType: {
                VMValue value = get(instr.src1);
                if (!is_instance(value, instr.type_name, types)) {
                    throw std::runtime_error("Runtime error: no se puede castear '" +
                                             to_string(value) + "' a '" + instr.type_name + "'.");
                }
                set(instr.dest, value);
                break;
            }
        }
    }

    return VMValue();
}

BannerVM::CompiledFunction BannerVM::compile_function(const Banner::BannerFunction& function) const {
    CompiledFunction out;
    out.function = &function;

    std::size_t next_slot = 0;
    for (const auto& param : function.params) {
        out.slots.emplace(param, next_slot++);
    }
    for (const auto& local : function.locals) {
        out.slots.emplace(local, next_slot++);
    }

    for (std::size_t pc = 0; pc < function.code.size(); ++pc) {
        const auto& instr = function.code[pc];
        if (instr.op == Banner::Op::Label) {
            out.labels[instr.label] = pc;
        }
    }

    return out;
}

BannerVM::Frame BannerVM::make_frame(const CompiledFunction& function,
                                     const std::vector<VMValue>& args,
                                     std::string return_dest) const {
    if (args.size() != function.function->params.size()) {
        throw std::runtime_error("Runtime error: aridad incorrecta en " +
                                 function.function->source_name + ".");
    }

    Frame frame;
    frame.function = &function;
    frame.pc = 0;
    frame.slots.resize(function.slots.size());
    frame.return_dest = std::move(return_dest);
    frame.has_return_dest = !frame.return_dest.empty();
    for (std::size_t i = 0; i < args.size(); ++i) {
        frame.slots.at(slot_of(function, function.function->params.at(i))) = args.at(i);
    }

    return frame;
}

std::unordered_map<std::string, BannerVM::CompiledType>
BannerVM::compile_types(const Banner::BannerProgram& program) const {
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
                auto slot_it = type.method_slots.find(method.name);
                if (slot_it == type.method_slots.end()) {
                    const std::size_t slot = type.vtable.size();
                    type.method_slots.emplace(method.name, slot);
                    type.vtable.push_back(method.function_name);
                } else {
                    type.vtable.at(slot_it->second) = method.function_name;
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

const BannerVM::CompiledType& BannerVM::type_of(
    const std::unordered_map<std::string, CompiledType>& types,
    const std::string& name) const {
    auto it = types.find(name);
    if (it == types.end()) {
        throw std::runtime_error("Runtime error: tipo no encontrado: " + name);
    }
    return it->second;
}

const BannerVM::CompiledType& BannerVM::type_of(
    const std::unordered_map<std::string, CompiledType>& types,
    const VMValue& value) const {
    if (!is_object(value)) {
        throw std::runtime_error("Runtime error: se esperaba Object.");
    }
    auto object = std::get<VMObjectRef>(value.inner);
    for (auto it = types.begin(); it != types.end(); ++it) {
        if (it->second.type_id == object->type_id) {
            return it->second;
        }
    }
    throw std::runtime_error("Runtime error: type_id de objeto invalido.");
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

bool BannerVM::is_instance(const VMValue& value,
                           const std::string& type_name,
                           const std::unordered_map<std::string, CompiledType>& types) const {
    if (!is_object(value)) {
        if (type_name == "Number") return is_number(value);
        if (type_name == "String") return is_string(value);
        if (type_name == "Boolean") return is_bool(value);
        return false;
    }

    const auto& start_type = type_of(types, value);
    const CompiledType* current = &start_type;
    while (current != nullptr) {
        if (current->type->name == type_name) return true;
        if (current->parent_type_id == -1) break;

        const CompiledType* parent = nullptr;
        for (auto it = types.begin(); it != types.end(); ++it) {
            if (it->second.type_id == current->parent_type_id) {
                parent = &it->second;
                break;
            }
        }
        current = parent;
    }

    return type_name == "Object";
}

[[noreturn]] void BannerVM::unsupported(const std::string& feature) const {
    throw std::runtime_error("Runtime error: BannerVM aun no soporta " + feature + ".");
}

}
