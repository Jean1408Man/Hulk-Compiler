#include "hulkir_to_banner.h"

#include "codegen_error.h"

namespace Hulk::Backend {

Banner::BannerProgram HulkIRToBanner::lower(const IR::IRProgram& program) const {
    Banner::BannerProgram out;
    out.entry_function = program.entry_function;

    for (const auto& type : program.types) {
        Banner::BannerType lowered;
        lowered.name = type.name;
        lowered.parent = type.parent;
        lowered.init_name = type.init_name;
        lowered.ctor_name = type.ctor_name;
        for (const auto& field : type.fields) {
            lowered.fields.push_back(Banner::BannerField{
                field.owner_type,
                field.name,
                field.lowered_name,
                field.type_name,
                field.slot,
            });
        }
        for (const auto& method : type.methods) {
            lowered.methods.push_back(Banner::BannerMethod{
                method.owner_type,
                method.name,
                method.function_name,
                method.arity,
                method.slot,
            });
        }
        out.types.push_back(std::move(lowered));
    }

    for (const auto& data : program.data) {
        out.data.push_back(Banner::BannerData{data.label, data.value});
    }

    for (const auto& fn : program.functions) {
        Banner::BannerFunction lowered;
        lowered.name = fn.name;
        lowered.source_name = fn.source_name;
        lowered.kind = fn.kind;
        lowered.params = fn.params;
        lowered.locals = fn.locals;
        for (const auto& instr : fn.body) {
            (void)lower_instr(instr, lowered.code);
        }
        out.functions.push_back(std::move(lowered));
    }

    return out;
}

Banner::BannerInstr HulkIRToBanner::lower_instr(const IR::IRInstr& instr,
                                                std::vector<Banner::BannerInstr>& out) const {
    Banner::BannerInstr lowered;
    lowered.dest = instr.dest;
    lowered.src1 = instr.src1;
    lowered.src2 = instr.src2;
    lowered.label = instr.label;
    lowered.callee = instr.callee;
    lowered.type_name = instr.type_name;
    lowered.field_name = instr.field_name;
    lowered.method_name = instr.method_name;
    lowered.args = instr.args;
    lowered.number_value = instr.number_value;
    lowered.bool_value = instr.bool_value;

    switch (instr.op) {
        case IR::IROp::Nop: lowered.op = Banner::Op::Nop; break;
        case IR::IROp::ConstNil: lowered.op = Banner::Op::ConstNil; break;
        case IR::IROp::ConstNumber: lowered.op = Banner::Op::ConstNumber; break;
        case IR::IROp::ConstBool: lowered.op = Banner::Op::ConstBool; break;
        case IR::IROp::LoadData: lowered.op = Banner::Op::LoadData; break;
        case IR::IROp::Move: lowered.op = Banner::Op::Move; break;
        case IR::IROp::Add: lowered.op = Banner::Op::Add; break;
        case IR::IROp::Sub: lowered.op = Banner::Op::Sub; break;
        case IR::IROp::Mul: lowered.op = Banner::Op::Mul; break;
        case IR::IROp::Div: lowered.op = Banner::Op::Div; break;
        case IR::IROp::Mod: lowered.op = Banner::Op::Mod; break;
        case IR::IROp::Pow: lowered.op = Banner::Op::Pow; break;
        case IR::IROp::Neg: lowered.op = Banner::Op::Neg; break;
        case IR::IROp::And: lowered.op = Banner::Op::And; break;
        case IR::IROp::Or: lowered.op = Banner::Op::Or; break;
        case IR::IROp::Not: lowered.op = Banner::Op::Not; break;
        case IR::IROp::Equal: lowered.op = Banner::Op::Equal; break;
        case IR::IROp::NotEqual: lowered.op = Banner::Op::NotEqual; break;
        case IR::IROp::Less: lowered.op = Banner::Op::Less; break;
        case IR::IROp::Greater: lowered.op = Banner::Op::Greater; break;
        case IR::IROp::LessEqual: lowered.op = Banner::Op::LessEqual; break;
        case IR::IROp::GreaterEqual: lowered.op = Banner::Op::GreaterEqual; break;
        case IR::IROp::Concat: lowered.op = Banner::Op::Concat; break;
        case IR::IROp::ConcatSpace: lowered.op = Banner::Op::ConcatSpace; break;
        case IR::IROp::Label: lowered.op = Banner::Op::Label; break;
        case IR::IROp::Jump: lowered.op = Banner::Op::Jump; break;
        case IR::IROp::JumpIfTrue: lowered.op = Banner::Op::JumpIfTrue; break;
        case IR::IROp::JumpIfFalse: lowered.op = Banner::Op::JumpIfFalse; break;
        case IR::IROp::Return: lowered.op = Banner::Op::Return; break;
        case IR::IROp::NewObject: lowered.op = Banner::Op::Allocate; break;
        case IR::IROp::GetField: lowered.op = Banner::Op::GetAttr; break;
        case IR::IROp::DefineField:
        case IR::IROp::SetField:
            lowered.op = Banner::Op::SetAttr;
            break;
        case IR::IROp::VCall: lowered.op = Banner::Op::VCall; break;
        case IR::IROp::SCallMethod: lowered.op = Banner::Op::SCall; break;
        case IR::IROp::IsType: lowered.op = Banner::Op::IsType; break;
        case IR::IROp::AsType: lowered.op = Banner::Op::AsType; break;
        case IR::IROp::BuiltinPrint: lowered.op = Banner::Op::Print; break;
        case IR::IROp::BuiltinSqrt: lowered.op = Banner::Op::Sqrt; break;
        case IR::IROp::BuiltinSin: lowered.op = Banner::Op::Sin; break;
        case IR::IROp::BuiltinCos: lowered.op = Banner::Op::Cos; break;
        case IR::IROp::BuiltinExp: lowered.op = Banner::Op::Exp; break;
        case IR::IROp::BuiltinLog: lowered.op = Banner::Op::Log; break;
        case IR::IROp::BuiltinRand: lowered.op = Banner::Op::Rand; break;
        case IR::IROp::Call:
            for (const auto& arg : instr.args) {
                Banner::BannerInstr param;
                param.op = Banner::Op::Param;
                param.src1 = arg;
                out.push_back(std::move(param));
            }
            lowered.op = Banner::Op::Call;
            break;
    }

    out.push_back(lowered);
    return lowered;
}

} 
