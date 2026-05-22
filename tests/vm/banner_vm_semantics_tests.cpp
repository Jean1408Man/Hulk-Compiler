#include "banner/banner_ir.h"
#include "vm/banner_vm.h"
#include "vm/vm_value.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <utility>

using namespace Hulk;

namespace {

bool contains(const std::string& text, const std::string& fragment) {
    return text.find(fragment) != std::string::npos;
}

bool throws_with_message(auto&& fn, const std::string& fragment) {
    try {
        fn();
    } catch (const std::runtime_error& err) {
        return contains(err.what(), fragment);
    }
    return false;
}

Banner::BannerFunction make_function(std::string name) {
    Banner::BannerFunction fn;
    fn.name = std::move(name);
    fn.source_name = fn.name;
    return fn;
}

Banner::BannerType make_type(std::string name) {
    Banner::BannerType type;
    type.name = std::move(name);
    return type;
}

Banner::BannerField field(std::string name) {
    Banner::BannerField out;
    out.name = std::move(name);
    return out;
}

Banner::BannerMethod method(std::string name, std::string function_name) {
    Banner::BannerMethod out;
    out.name = std::move(name);
    out.function_name = std::move(function_name);
    return out;
}

Banner::BannerInstr instr(Banner::Op op) {
    Banner::BannerInstr out;
    out.op = op;
    return out;
}

Banner::BannerInstr const_number(std::string dest, double value) {
    auto out = instr(Banner::Op::ConstNumber);
    out.dest = std::move(dest);
    out.number_value = value;
    return out;
}

Banner::BannerInstr load_data(std::string dest, std::string label) {
    auto out = instr(Banner::Op::LoadData);
    out.dest = std::move(dest);
    out.src1 = std::move(label);
    return out;
}

Banner::BannerInstr binary(Banner::Op op, std::string dest, std::string left, std::string right) {
    auto out = instr(op);
    out.dest = std::move(dest);
    out.src1 = std::move(left);
    out.src2 = std::move(right);
    return out;
}

Banner::BannerInstr label(std::string name) {
    auto out = instr(Banner::Op::Label);
    out.label = std::move(name);
    return out;
}

Banner::BannerInstr jump_if_true(std::string cond, std::string label_name) {
    auto out = instr(Banner::Op::JumpIfTrue);
    out.src1 = std::move(cond);
    out.label = std::move(label_name);
    return out;
}

Banner::BannerInstr param(std::string src) {
    auto out = instr(Banner::Op::Param);
    out.src1 = std::move(src);
    return out;
}

Banner::BannerInstr call(std::string dest, std::string callee) {
    auto out = instr(Banner::Op::Call);
    out.dest = std::move(dest);
    out.callee = std::move(callee);
    return out;
}

Banner::BannerInstr ret(std::string src) {
    auto out = instr(Banner::Op::Return);
    out.src1 = std::move(src);
    return out;
}

Banner::BannerInstr allocate(std::string dest, std::string type_name) {
    auto out = instr(Banner::Op::Allocate);
    out.dest = std::move(dest);
    out.type_name = std::move(type_name);
    return out;
}

Banner::BannerInstr get_attr(std::string dest, std::string receiver, std::string field_name) {
    auto out = instr(Banner::Op::GetAttr);
    out.dest = std::move(dest);
    out.src1 = std::move(receiver);
    out.field_name = std::move(field_name);
    return out;
}

Banner::BannerInstr set_attr(std::string dest,
                             std::string receiver,
                             std::string value,
                             std::string field_name) {
    auto out = instr(Banner::Op::SetAttr);
    out.dest = std::move(dest);
    out.src1 = std::move(receiver);
    out.src2 = std::move(value);
    out.field_name = std::move(field_name);
    return out;
}

Banner::BannerInstr vcall(std::string dest, std::string receiver, std::string method_name) {
    auto out = instr(Banner::Op::VCall);
    out.dest = std::move(dest);
    out.src1 = std::move(receiver);
    out.method_name = std::move(method_name);
    return out;
}

void minimal_program_returns_number() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto main = make_function("hulk_main");
    main.locals = {"n"};
    main.code.push_back(const_number("n", 7.0));
    main.code.push_back(ret("n"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 7.0);
}

void direct_call_uses_param_buffer_and_return_slot() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto sum = make_function("sum");
    sum.params = {"x", "y"};
    sum.locals = {"r"};
    sum.code.push_back(binary(Banner::Op::Add, "r", "x", "y"));
    sum.code.push_back(ret("r"));

    auto main = make_function("hulk_main");
    main.locals = {"a", "b", "r"};
    main.code.push_back(const_number("a", 4.0));
    main.code.push_back(const_number("b", 5.0));
    main.code.push_back(param("a"));
    main.code.push_back(param("b"));
    main.code.push_back(call("r", "sum"));
    main.code.push_back(ret("r"));

    program.functions.push_back(std::move(sum));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 9.0);
}

void finite_recursion_uses_vm_frames_correctly() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto countdown = make_function("countdown");
    countdown.params = {"n"};
    countdown.locals = {"zero", "cond", "one", "next", "r"};
    countdown.code.push_back(const_number("zero", 0.0));
    countdown.code.push_back(binary(Banner::Op::LessEqual, "cond", "n", "zero"));
    countdown.code.push_back(jump_if_true("cond", "base"));
    countdown.code.push_back(const_number("one", 1.0));
    countdown.code.push_back(binary(Banner::Op::Sub, "next", "n", "one"));
    countdown.code.push_back(param("next"));
    countdown.code.push_back(call("r", "countdown"));
    countdown.code.push_back(ret("r"));
    countdown.code.push_back(label("base"));
    countdown.code.push_back(ret("zero"));

    auto main = make_function("hulk_main");
    main.locals = {"n", "r"};
    main.code.push_back(const_number("n", 3.0));
    main.code.push_back(param("n"));
    main.code.push_back(call("r", "countdown"));
    main.code.push_back(ret("r"));

    program.functions.push_back(std::move(countdown));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 0.0);
}

void object_field_storage_round_trips_through_vm() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";
    auto box = make_type("Box");
    box.fields.push_back(field("value"));
    program.types.push_back(std::move(box));

    auto main = make_function("hulk_main");
    main.locals = {"obj", "value", "ignored", "got"};
    main.code.push_back(allocate("obj", "Box"));
    main.code.push_back(const_number("value", 9.0));
    main.code.push_back(set_attr("ignored", "obj", "value", "value"));
    main.code.push_back(get_attr("got", "obj", "value"));
    main.code.push_back(ret("got"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 9.0);
}

void dynamic_string_concat_can_be_compared() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";
    program.data.push_back(Banner::BannerData{"a", "hola"});
    program.data.push_back(Banner::BannerData{"b", "mundo"});
    program.data.push_back(Banner::BannerData{"expected", "hola mundo"});

    auto main = make_function("hulk_main");
    main.locals = {"a", "b", "joined", "expected", "ok"};
    main.code.push_back(load_data("a", "a"));
    main.code.push_back(load_data("b", "b"));
    main.code.push_back(binary(Banner::Op::ConcatSpace, "joined", "a", "b"));
    main.code.push_back(load_data("expected", "expected"));
    main.code.push_back(binary(Banner::Op::Equal, "ok", "joined", "expected"));
    main.code.push_back(ret("ok"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const VM::Word result = vm.run(program);
    assert(VM::is_bool(result));
    assert(VM::as_bool(result));
}

void vcall_dispatches_through_type_vtable() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto box = make_type("Box");
    box.methods.push_back(method("get", "Box_get"));
    program.types.push_back(std::move(box));

    auto get = make_function("Box_get");
    get.params = {"self"};
    get.locals = {"n"};
    get.code.push_back(const_number("n", 21.0));
    get.code.push_back(ret("n"));

    auto main = make_function("hulk_main");
    main.locals = {"obj", "r"};
    main.code.push_back(allocate("obj", "Box"));
    main.code.push_back(vcall("r", "obj", "get"));
    main.code.push_back(ret("r"));

    program.functions.push_back(std::move(get));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 21.0);
}

void ambiguous_field_name_falls_back_to_runtime_type_lookup() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto a = make_type("A");
    a.fields.push_back(field("other"));
    a.fields.push_back(field("shared"));
    auto b = make_type("B");
    b.fields.push_back(field("shared"));
    program.types.push_back(std::move(a));
    program.types.push_back(std::move(b));

    auto main = make_function("hulk_main");
    main.locals = {"obj", "value", "ignored", "got"};
    main.code.push_back(allocate("obj", "B"));
    main.code.push_back(const_number("value", 33.0));
    main.code.push_back(set_attr("ignored", "obj", "value", "shared"));
    main.code.push_back(get_attr("got", "obj", "shared"));
    main.code.push_back(ret("got"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const std::string view = vm.compiled_view(program);
    assert(contains(view, "SET_ATTR s0.shared"));
    assert(contains(view, "GET_ATTR s0.shared"));

    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 33.0);
}

void ambiguous_method_name_falls_back_to_runtime_type_lookup() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto a = make_type("A");
    a.methods.push_back(method("other", "A_other"));
    a.methods.push_back(method("speak", "A_speak"));
    auto b = make_type("B");
    b.methods.push_back(method("speak", "B_speak"));
    program.types.push_back(std::move(a));
    program.types.push_back(std::move(b));

    auto a_other = make_function("A_other");
    a_other.params = {"self"};
    a_other.locals = {"n"};
    a_other.code.push_back(const_number("n", 1.0));
    a_other.code.push_back(ret("n"));

    auto a_speak = make_function("A_speak");
    a_speak.params = {"self"};
    a_speak.locals = {"n"};
    a_speak.code.push_back(const_number("n", 2.0));
    a_speak.code.push_back(ret("n"));

    auto b_speak = make_function("B_speak");
    b_speak.params = {"self"};
    b_speak.locals = {"n"};
    b_speak.code.push_back(const_number("n", 44.0));
    b_speak.code.push_back(ret("n"));

    auto main = make_function("hulk_main");
    main.locals = {"obj", "r"};
    main.code.push_back(allocate("obj", "B"));
    main.code.push_back(vcall("r", "obj", "speak"));
    main.code.push_back(ret("r"));

    program.functions.push_back(std::move(a_other));
    program.functions.push_back(std::move(a_speak));
    program.functions.push_back(std::move(b_speak));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const std::string view = vm.compiled_view(program);
    assert(contains(view, "VCALL s0.speak"));

    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 44.0);
}

void arity_errors_are_reported_as_runtime_errors() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto identity = make_function("identity");
    identity.params = {"x"};
    identity.code.push_back(ret("x"));

    auto main = make_function("hulk_main");
    main.locals = {"r"};
    main.code.push_back(call("r", "identity"));
    main.code.push_back(ret("r"));

    program.functions.push_back(std::move(identity));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    assert(throws_with_message([&] { (void)vm.run(program); }, "aridad incorrecta"));
}

}

int main() {
    minimal_program_returns_number();
    direct_call_uses_param_buffer_and_return_slot();
    finite_recursion_uses_vm_frames_correctly();
    object_field_storage_round_trips_through_vm();
    dynamic_string_concat_can_be_compared();
    vcall_dispatches_through_type_vtable();
    ambiguous_field_name_falls_back_to_runtime_type_lookup();
    ambiguous_method_name_falls_back_to_runtime_type_lookup();
    arity_errors_are_reported_as_runtime_errors();
    return 0;
}
