#include "banner_vm.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace Hulk::VM {

VMValue BannerVM::run(const Banner::BannerProgram& program) {
    std::unordered_map<std::string, std::string> data;
    for (const auto& item : program.data) {
        data[item.label] = item.value;
    }

    for (const auto& function : program.functions) {
        if (function.name == program.entry_function) {
            return run_function(program, compile_function(function), data);
        }
    }

    throw std::runtime_error("Runtime error: funcion de entrada no encontrada.");
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

VMValue BannerVM::run_function(const Banner::BannerProgram&,
                               const CompiledFunction& function,
                               const std::unordered_map<std::string, std::string>& data) const {
    std::vector<VMValue> slots(function.slots.size());
    std::size_t pc = 0;
    const auto& code = function.function->code;

    auto get = [&](const std::string& name) -> const VMValue& {
        return slots.at(slot_of(function, name));
    };
    auto set = [&](const std::string& name, VMValue value) {
        slots.at(slot_of(function, name)) = std::move(value);
    };

    while (pc < code.size()) {
        const auto& instr = code[pc++];
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
                pc = label_of(function, instr.label);
                break;
            case Banner::Op::JumpIfTrue:
                if (truthy(get(instr.src1))) pc = label_of(function, instr.label);
                break;
            case Banner::Op::JumpIfFalse:
                if (!truthy(get(instr.src1))) pc = label_of(function, instr.label);
                break;
            case Banner::Op::Return:
                return get(instr.src1);
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
            case Banner::Op::Call:
                unsupported("funciones y llamadas");
            case Banner::Op::Allocate:
            case Banner::Op::GetAttr:
            case Banner::Op::SetAttr:
            case Banner::Op::VCall:
            case Banner::Op::SCall:
            case Banner::Op::IsType:
            case Banner::Op::AsType:
                unsupported("objetos y tipos");
        }
    }

    return VMValue();
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

[[noreturn]] void BannerVM::unsupported(const std::string& feature) const {
    throw std::runtime_error("Runtime error: BannerVM etapa 1 no soporta " + feature + ".");
}

}
