#ifndef HULK_BANNER_IR_H
#define HULK_BANNER_IR_H

#include "../ir/ir.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Hulk::Banner {

enum class Op {
    Nop,
    ConstNil,
    ConstNumber,
    ConstBool,
    LoadData,
    Move,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
    Neg,
    And,
    Or,
    Not,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    Concat,
    ConcatSpace,
    Label,
    Jump,
    JumpIfTrue,
    JumpIfFalse,
    Param,
    Call,
    Return,
    Allocate,
    GetAttr,
    SetAttr,
    VCall,
    SCall,
    IsType,
    AsType,
    Print,
    Sqrt,
    Sin,
    Cos,
    Exp,
    Log,
    Rand
};

struct BannerField {
    std::string owner_type;
    std::string name;
    std::string lowered_name;
    std::string type_name = "Object";
    int slot = -1;
};

struct BannerMethod {
    std::string owner_type;
    std::string name;
    std::string function_name;
    std::size_t arity = 0;
    int slot = -1;
};

struct BannerType {
    std::string name;
    std::string parent = "Object";
    std::string init_name;
    std::string ctor_name;
    std::vector<BannerField> fields;
    std::vector<BannerMethod> methods;
};

struct BannerData {
    std::string label;
    std::string value;
};

struct BannerInstr {
    Op op = Op::Nop;
    std::string dest;
    std::string src1;
    std::string src2;
    std::string label;
    std::string callee;
    std::string type_name;
    std::string field_name;
    std::string method_name;
    std::vector<std::string> args;
    double number_value = 0.0;
    bool bool_value = false;
};

struct BannerFunction {
    std::string name;
    std::string source_name;
    IR::IRFunctionKind kind = IR::IRFunctionKind::Global;
    std::vector<std::string> params;
    std::vector<std::string> locals;
    std::vector<BannerInstr> code;
};

struct BannerProgram {
    std::vector<BannerType> types;
    std::vector<BannerData> data;
    std::vector<BannerFunction> functions;
    std::string entry_function = "hulk_main";
};

std::string op_name(Op op);

}

#endif
