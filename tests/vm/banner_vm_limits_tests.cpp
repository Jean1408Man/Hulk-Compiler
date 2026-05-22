#include "banner/banner_ir.h"
#include "vm/banner_vm.h"
#include "vm/vm_value.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <utility>

using namespace Hulk;

namespace {

bool throws_with_message(auto&& fn, const std::string& fragment) {
    try {
        fn();
    } catch (const std::runtime_error& err) {
        return std::string(err.what()).find(fragment) != std::string::npos;
    }
    return false;
}

std::string runtime_error_from(auto&& fn) {
    try {
        fn();
    } catch (const std::runtime_error& err) {
        return err.what();
    }
    return {};
}

bool contains(const std::string& text, const std::string& fragment) {
    return text.find(fragment) != std::string::npos;
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

Banner::BannerInstr instr(Banner::Op op) {
    Banner::BannerInstr out;
    out.op = op;
    return out;
}

Banner::BannerInstr label(std::string name) {
    auto out = instr(Banner::Op::Label);
    out.label = std::move(name);
    return out;
}

Banner::BannerInstr jump(std::string name) {
    auto out = instr(Banner::Op::Jump);
    out.label = std::move(name);
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

Banner::BannerInstr const_number(std::string dest, double value) {
    auto out = instr(Banner::Op::ConstNumber);
    out.dest = std::move(dest);
    out.number_value = value;
    return out;
}

Banner::BannerInstr div(std::string dest, std::string lhs, std::string rhs) {
    auto out = instr(Banner::Op::Div);
    out.dest = std::move(dest);
    out.src1 = std::move(lhs);
    out.src2 = std::move(rhs);
    return out;
}

Banner::BannerInstr allocate(std::string dest, std::string type_name) {
    auto out = instr(Banner::Op::Allocate);
    out.dest = std::move(dest);
    out.type_name = std::move(type_name);
    return out;
}

void step_limit_stops_infinite_loop() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto main = make_function("hulk_main");
    main.code.push_back(label("loop"));
    main.code.push_back(jump("loop"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    VM::VMOptions options;
    options.max_steps = 3;

    assert(throws_with_message([&] { (void)vm.run(program, options); },
                               "limite de instrucciones"));
}

void frame_limit_stops_recursion() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto recursive = make_function("f");
    recursive.params.push_back("x");
    recursive.locals.push_back("r");
    recursive.code.push_back(param("x"));
    recursive.code.push_back(call("r", "f"));
    recursive.code.push_back(ret("r"));

    auto main = make_function("hulk_main");
    main.locals.push_back("x");
    main.locals.push_back("r");
    main.code.push_back(const_number("x", 0.0));
    main.code.push_back(param("x"));
    main.code.push_back(call("r", "f"));
    main.code.push_back(ret("r"));

    program.functions.push_back(std::move(recursive));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    VM::VMOptions options;
    options.max_frames = 2;

    assert(throws_with_message([&] { (void)vm.run(program, options); },
                               "limite de frames"));
}

void heap_limit_stops_allocation_growth() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";
    program.types.push_back(make_type("A"));

    auto main = make_function("hulk_main");
    main.locals.push_back("a");
    main.locals.push_back("b");
    main.code.push_back(allocate("a", "A"));
    main.code.push_back(allocate("b", "A"));
    main.code.push_back(ret("b"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    VM::VMOptions options;
    options.max_heap_values = 1;

    assert(throws_with_message([&] { (void)vm.run(program, options); },
                               "limite de heap"));
}

void default_limits_allow_small_program() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto main = make_function("hulk_main");
    main.locals.push_back("n");
    main.code.push_back(const_number("n", 42.0));
    main.code.push_back(ret("n"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const VM::Word result = vm.run(program);
    assert(VM::is_number(result));
    assert(VM::as_number(result) == 42.0);
}

void compiled_view_uses_slots_and_pcs() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto main = make_function("hulk_main");
    main.locals.push_back("x");
    main.locals.push_back("y");
    main.code.push_back(const_number("x", 42.0));
    main.code.push_back(ret("x"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const std::string view = vm.compiled_view(program);
    assert(contains(view, ".COMPILED_BANNER"));
    assert(contains(view, "function #0 hulk_main"));
    assert(contains(view, "s0 = x"));
    assert(contains(view, "0: s0 = CONST_NUMBER 42"));
    assert(contains(view, "1: RETURN s0"));
}

void runtime_error_includes_instruction_context() {
    Banner::BannerProgram program;
    program.entry_function = "hulk_main";

    auto main = make_function("hulk_main");
    main.locals.push_back("x");
    main.locals.push_back("y");
    main.locals.push_back("z");
    main.code.push_back(const_number("x", 1.0));
    main.code.push_back(const_number("y", 0.0));
    auto bad_div = div("z", "x", "y");
    bad_div.source = IR::SourceSpan{
        "tests/vm/runtime_error.hulk",
        hulk::common::Span{hulk::common::Position{.index = 0, .line = 3, .column = 12},
                           hulk::common::Position{.index = 0, .line = 3, .column = 17}},
    };
    main.code.push_back(std::move(bad_div));
    main.code.push_back(ret("z"));
    program.functions.push_back(std::move(main));

    VM::BannerVM vm;
    const std::string message = runtime_error_from([&] { (void)vm.run(program); });
    assert(contains(message, "Runtime error en hulk_main pc=2"));
    assert(contains(message, "source: tests/vm/runtime_error.hulk:3:12"));
    assert(contains(message, "instr: s2 = DIV s0, s1"));
    assert(contains(message, "causa: Runtime error: division por cero."));
    assert(contains(message, "stack:"));
    assert(contains(message, "at hulk_main pc=2"));
}

}

int main() {
    step_limit_stops_infinite_loop();
    frame_limit_stops_recursion();
    heap_limit_stops_allocation_growth();
    default_limits_allow_small_program();
    compiled_view_uses_slots_and_pcs();
    runtime_error_includes_instruction_context();
    return 0;
}
