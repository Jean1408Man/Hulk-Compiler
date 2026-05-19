#include "banner_printer.h"

#include "../ir/ir.h"

#include <iomanip>
#include <sstream>

namespace Hulk::Banner {

std::string BannerPrinter::print(const BannerProgram& program) const {
    std::ostringstream out;

    out << ".TYPES\n";
    for (const auto& type : program.types) {
        out << "type " << type.name << " parent " << type.parent << " {\n";
        for (const auto& field : type.fields) {
            out << "  field " << field.lowered_name << " : " << field.type_name
                << " slot " << field.slot << "\n";
        }
        for (const auto& method : type.methods) {
            out << "  method " << method.name << " -> " << method.function_name
                << " slot " << method.slot << "\n";
        }
        out << "}\n\n";
    }

    out << ".DATA\n";
    for (const auto& data : program.data) {
        out << data.label << " = " << escape_string(data.value) << "\n";
    }

    out << "\n.CODE\n";
    for (const auto& fn : program.functions) {
        out << "function " << fn.name << "(" << join_args(fn.params) << ")"
            << " ; kind=" << IR::function_kind_name(fn.kind) << "\n";
        out << "{\n";
        for (const auto& local : fn.locals) {
            out << "  local " << local << "\n";
        }
        for (const auto& instr : fn.code) {
            out << "  " << print_instr(instr) << "\n";
        }
        out << "}\n\n";
    }

    return out.str();
}

std::string BannerPrinter::print_instr(const BannerInstr& instr) const {
    std::ostringstream out;
    switch (instr.op) {
        case Op::Nop:
            out << "NOP";
            break;
        case Op::ConstNil:
            out << "CONST_NIL " << instr.dest;
            break;
        case Op::ConstNumber:
            out << "CONST_NUMBER " << instr.dest << " " << std::setprecision(17)
                << instr.number_value;
            break;
        case Op::ConstBool:
            out << "CONST_BOOL " << instr.dest << " " << (instr.bool_value ? "true" : "false");
            break;
        case Op::LoadData:
            out << "LOAD_DATA " << instr.dest << " " << instr.src1;
            break;
        case Op::Move:
            out << "MOVE " << instr.dest << " " << instr.src1;
            break;
        case Op::Add:
        case Op::Sub:
        case Op::Mul:
        case Op::Div:
        case Op::Mod:
        case Op::Pow:
        case Op::And:
        case Op::Or:
        case Op::Equal:
        case Op::NotEqual:
        case Op::Less:
        case Op::Greater:
        case Op::LessEqual:
        case Op::GreaterEqual:
        case Op::Concat:
        case Op::ConcatSpace:
            out << op_name(instr.op) << " " << instr.dest << " " << instr.src1 << " " << instr.src2;
            break;
        case Op::Neg:
        case Op::Not:
            out << op_name(instr.op) << " " << instr.dest << " " << instr.src1;
            break;
        case Op::Label:
            out << "LABEL " << instr.label;
            break;
        case Op::Jump:
            out << "JUMP " << instr.label;
            break;
        case Op::JumpIfTrue:
        case Op::JumpIfFalse:
            out << op_name(instr.op) << " " << instr.src1 << " " << instr.label;
            break;
        case Op::Param:
            out << "PARAM " << instr.src1;
            break;
        case Op::Call:
            out << "CALL " << instr.dest << " " << instr.callee;
            break;
        case Op::Return:
            out << "RETURN " << instr.src1;
            break;
        case Op::Allocate:
            out << "ALLOCATE " << instr.dest << " " << instr.type_name;
            break;
        case Op::GetAttr:
            out << "GETATTR " << instr.dest << " " << instr.src1 << " " << instr.field_name;
            break;
        case Op::SetAttr:
            out << "SETATTR " << instr.src1 << " " << instr.field_name << " " << instr.src2;
            break;
        case Op::VCall:
            out << "VCALL " << instr.dest << " " << instr.src1 << " " << instr.method_name;
            break;
        case Op::SCall:
            out << "SCALL " << instr.dest << " " << instr.type_name << " " << instr.method_name;
            break;
        case Op::IsType:
        case Op::AsType:
            out << op_name(instr.op) << " " << instr.dest << " " << instr.src1 << " " << instr.type_name;
            break;
        case Op::Print:
        case Op::Sqrt:
        case Op::Sin:
        case Op::Cos:
        case Op::Exp:
            out << op_name(instr.op) << " " << instr.dest << " " << instr.args.at(0);
            break;
        case Op::Log:
            out << "LOG " << instr.dest << " " << instr.args.at(0) << " " << instr.args.at(1);
            break;
        case Op::Rand:
            out << "RAND " << instr.dest;
            break;
    }
    return out.str();
}

std::string BannerPrinter::escape_string(const std::string& value) const {
    std::ostringstream out;
    out << '"';
    for (unsigned char ch : value) {
        switch (ch) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (ch < 32 || ch > 126) {
                    out << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                        << static_cast<int>(ch) << std::dec;
                } else {
                    out << static_cast<char>(ch);
                }
                break;
        }
    }
    out << '"';
    return out.str();
}

std::string BannerPrinter::join_args(const std::vector<std::string>& args) const {
    std::ostringstream out;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) out << ", ";
        out << args[i];
    }
    return out.str();
}

} 
