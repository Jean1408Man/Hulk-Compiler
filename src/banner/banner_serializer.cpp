#include "banner_serializer.h"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Hulk::Banner {
namespace {

void put_u8(std::ostream& o, uint8_t v)  { o.write(reinterpret_cast<const char*>(&v), 1); }
void put_u32(std::ostream& o, uint32_t v){ o.write(reinterpret_cast<const char*>(&v), 4); }
void put_i32(std::ostream& o, int32_t v) { o.write(reinterpret_cast<const char*>(&v), 4); }
void put_u64(std::ostream& o, uint64_t v){ o.write(reinterpret_cast<const char*>(&v), 8); }
void put_f64(std::ostream& o, double v)  { o.write(reinterpret_cast<const char*>(&v), 8); }

void put_str(std::ostream& o, const std::string& s) {
    put_u32(o, static_cast<uint32_t>(s.size()));
    if (!s.empty()) o.write(s.data(), static_cast<std::streamsize>(s.size()));
}

void put_strvec(std::ostream& o, const std::vector<std::string>& v) {
    put_u32(o, static_cast<uint32_t>(v.size()));
    for (const auto& s : v) put_str(o, s);
}

//  Read primitives

uint8_t get_u8(std::istream& i) {
    uint8_t v = 0; i.read(reinterpret_cast<char*>(&v), 1);
    if (!i) throw std::runtime_error("banner_serializer: EOF reading u8");
    return v;
}
uint32_t get_u32(std::istream& i) {
    uint32_t v = 0; i.read(reinterpret_cast<char*>(&v), 4);
    if (!i) throw std::runtime_error("banner_serializer: EOF reading u32");
    return v;
}
int32_t get_i32(std::istream& i) {
    int32_t v = 0; i.read(reinterpret_cast<char*>(&v), 4);
    if (!i) throw std::runtime_error("banner_serializer: EOF reading i32");
    return v;
}
uint64_t get_u64(std::istream& i) {
    uint64_t v = 0; i.read(reinterpret_cast<char*>(&v), 8);
    if (!i) throw std::runtime_error("banner_serializer: EOF reading u64");
    return v;
}
double get_f64(std::istream& i) {
    double v = 0.0; i.read(reinterpret_cast<char*>(&v), 8);
    if (!i) throw std::runtime_error("banner_serializer: EOF reading f64");
    return v;
}
std::string get_str(std::istream& i) {
    const uint32_t len = get_u32(i);
    if (len == 0) return {};
    std::string s(len, '\0');
    i.read(s.data(), len);
    if (!i) throw std::runtime_error("banner_serializer: EOF reading string");
    return s;
}
std::vector<std::string> get_strvec(std::istream& i) {
    const uint32_t n = get_u32(i);
    std::vector<std::string> v;
    v.reserve(n);
    for (uint32_t k = 0; k < n; ++k) v.push_back(get_str(i));
    return v;
}

//  Instructions

void put_instr(std::ostream& o, const BannerInstr& in) {
    put_u8(o, static_cast<uint8_t>(in.op));
    put_str(o, in.dest);
    put_str(o, in.src1);
    put_str(o, in.src2);
    put_str(o, in.label);
    put_str(o, in.callee);
    put_str(o, in.type_name);
    put_str(o, in.field_name);
    put_str(o, in.method_name);
    put_strvec(o, in.args);
    put_f64(o, in.number_value);
    put_u8(o, in.bool_value ? 1 : 0);
    // BannerInstr::source is debug info, not needed by the VM at runtime.
}

BannerInstr get_instr(std::istream& i) {
    BannerInstr in;
    in.op           = static_cast<Op>(get_u8(i));
    in.dest         = get_str(i);
    in.src1         = get_str(i);
    in.src2         = get_str(i);
    in.label        = get_str(i);
    in.callee       = get_str(i);
    in.type_name    = get_str(i);
    in.field_name   = get_str(i);
    in.method_name  = get_str(i);
    in.args         = get_strvec(i);
    in.number_value = get_f64(i);
    in.bool_value   = get_u8(i) != 0;
    return in;
}

} 

//  Public API 

void write_binary(const BannerProgram& prog, std::ostream& out) {
    put_u64(out, SERIALIZER_MAGIC);
    put_u32(out, SERIALIZER_VERSION);
    put_str(out, prog.entry_function);

    // Types
    put_u32(out, static_cast<uint32_t>(prog.types.size()));
    for (const auto& t : prog.types) {
        put_str(out, t.name);
        put_str(out, t.parent);
        put_str(out, t.init_name);
        put_str(out, t.ctor_name);

        put_u32(out, static_cast<uint32_t>(t.fields.size()));
        for (const auto& f : t.fields) {
            put_str(out, f.owner_type);
            put_str(out, f.name);
            put_str(out, f.lowered_name);
            put_str(out, f.type_name);
            put_i32(out, f.slot);
        }

        put_u32(out, static_cast<uint32_t>(t.methods.size()));
        for (const auto& m : t.methods) {
            put_str(out, m.owner_type);
            put_str(out, m.name);
            put_str(out, m.function_name);
            put_u32(out, static_cast<uint32_t>(m.arity));
            put_i32(out, m.slot);
        }
    }

    // Data
    put_u32(out, static_cast<uint32_t>(prog.data.size()));
    for (const auto& d : prog.data) {
        put_str(out, d.label);
        put_str(out, d.value);
    }

    // Functions
    put_u32(out, static_cast<uint32_t>(prog.functions.size()));
    for (const auto& fn : prog.functions) {
        put_str(out, fn.name);
        put_str(out, fn.source_name);
        put_u8(out, static_cast<uint8_t>(fn.kind));
        put_strvec(out, fn.params);
        put_strvec(out, fn.locals);
        put_u32(out, static_cast<uint32_t>(fn.code.size()));
        for (const auto& in : fn.code) put_instr(out, in);
    }

    if (!out) throw std::runtime_error("banner_serializer: I/O error during write");
}

BannerProgram read_binary(std::istream& in) {
    if (get_u64(in) != SERIALIZER_MAGIC)
        throw std::runtime_error("banner_serializer: bad magic");
    const uint32_t version = get_u32(in);
    if (version != SERIALIZER_VERSION)
        throw std::runtime_error("banner_serializer: unsupported version " + std::to_string(version));

    BannerProgram prog;
    prog.entry_function = get_str(in);

    const uint32_t type_count = get_u32(in);
    prog.types.reserve(type_count);
    for (uint32_t ti = 0; ti < type_count; ++ti) {
        BannerType t;
        t.name      = get_str(in);
        t.parent    = get_str(in);
        t.init_name = get_str(in);
        t.ctor_name = get_str(in);

        const uint32_t fc = get_u32(in);
        t.fields.reserve(fc);
        for (uint32_t fi = 0; fi < fc; ++fi) {
            BannerField f;
            f.owner_type   = get_str(in);
            f.name         = get_str(in);
            f.lowered_name = get_str(in);
            f.type_name    = get_str(in);
            f.slot         = get_i32(in);
            t.fields.push_back(std::move(f));
        }

        const uint32_t mc = get_u32(in);
        t.methods.reserve(mc);
        for (uint32_t mi = 0; mi < mc; ++mi) {
            BannerMethod m;
            m.owner_type    = get_str(in);
            m.name          = get_str(in);
            m.function_name = get_str(in);
            m.arity         = get_u32(in);
            m.slot          = get_i32(in);
            t.methods.push_back(std::move(m));
        }
        prog.types.push_back(std::move(t));
    }

    const uint32_t data_count = get_u32(in);
    prog.data.reserve(data_count);
    for (uint32_t di = 0; di < data_count; ++di) {
        BannerData d;
        d.label = get_str(in);
        d.value = get_str(in);
        prog.data.push_back(std::move(d));
    }

    const uint32_t fn_count = get_u32(in);
    prog.functions.reserve(fn_count);
    for (uint32_t fi = 0; fi < fn_count; ++fi) {
        BannerFunction fn;
        fn.name        = get_str(in);
        fn.source_name = get_str(in);
        fn.kind        = static_cast<IR::IRFunctionKind>(get_u8(in));
        fn.params      = get_strvec(in);
        fn.locals      = get_strvec(in);
        const uint32_t cc = get_u32(in);
        fn.code.reserve(cc);
        for (uint32_t ci = 0; ci < cc; ++ci) fn.code.push_back(get_instr(in));
        prog.functions.push_back(std::move(fn));
    }

    return prog;
}

}