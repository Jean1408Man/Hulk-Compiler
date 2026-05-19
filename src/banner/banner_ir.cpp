#include "banner_ir.h"

namespace Hulk::Banner {

std::string op_name(Op op) {
    switch (op) {
        case Op::Nop: return "NOP";
        case Op::ConstNil: return "CONST_NIL";
        case Op::ConstNumber: return "CONST_NUMBER";
        case Op::ConstBool: return "CONST_BOOL";
        case Op::LoadData: return "LOAD_DATA";
        case Op::Move: return "MOVE";
        case Op::Add: return "ADD";
        case Op::Sub: return "SUB";
        case Op::Mul: return "MUL";
        case Op::Div: return "DIV";
        case Op::Mod: return "MOD";
        case Op::Pow: return "POW";
        case Op::Neg: return "NEG";
        case Op::And: return "AND";
        case Op::Or: return "OR";
        case Op::Not: return "NOT";
        case Op::Equal: return "EQ";
        case Op::NotEqual: return "NE";
        case Op::Less: return "LT";
        case Op::Greater: return "GT";
        case Op::LessEqual: return "LE";
        case Op::GreaterEqual: return "GE";
        case Op::Concat: return "CONCAT";
        case Op::ConcatSpace: return "CONCAT_SPACE";
        case Op::Label: return "LABEL";
        case Op::Jump: return "JUMP";
        case Op::JumpIfTrue: return "JUMP_IF_TRUE";
        case Op::JumpIfFalse: return "JUMP_IF_FALSE";
        case Op::Param: return "PARAM";
        case Op::Call: return "CALL";
        case Op::Return: return "RETURN";
        case Op::Allocate: return "ALLOCATE";
        case Op::GetAttr: return "GETATTR";
        case Op::SetAttr: return "SETATTR";
        case Op::VCall: return "VCALL";
        case Op::SCall: return "SCALL";
        case Op::IsType: return "IS_TYPE";
        case Op::AsType: return "AS_TYPE";
        case Op::Print: return "PRINT";
        case Op::Sqrt: return "SQRT";
        case Op::Sin: return "SIN";
        case Op::Cos: return "COS";
        case Op::Exp: return "EXP";
        case Op::Log: return "LOG";
        case Op::Rand: return "RAND";
    }
    return "UNKNOWN";
}

} 
