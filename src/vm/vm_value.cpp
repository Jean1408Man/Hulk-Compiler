#include "vm_value.h"

#include "vm_heap.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace Hulk::VM {
namespace {

constexpr Word kTagMask = 0xffff000000000000ULL;
constexpr Word kTagBase = 0x7ffc000000000000ULL;
constexpr Word kKindShift = 40;
constexpr Word kPayloadMask = 0x000000ffffffffffULL;
constexpr Word kHandleIndexBits = 24;
constexpr Word kHandleIndexMask = (Word{1} << kHandleIndexBits) - 1;
constexpr Word kHandleGenerationShift = kHandleIndexBits;
constexpr Word kHandleGenerationMask = 0xffffULL;

Word make_tagged(WordKind kind, std::size_t payload) {
    if ((static_cast<Word>(payload) & ~kPayloadMask) != 0) {
        throw std::runtime_error("Runtime error: handle de VM fuera de rango.");
    }
    return kTagBase | (static_cast<Word>(kind) << kKindShift) |
           (static_cast<Word>(payload) & kPayloadMask);
}

WordKind kind_of(Word value) {
    if ((value & kTagMask) != kTagBase) return WordKind::Number;
    return static_cast<WordKind>((value >> kKindShift) & 0xffU);
}

std::size_t payload_of(Word value) {
    return static_cast<std::size_t>(value & kPayloadMask);
}

std::size_t handle_index(Word value) {
    return static_cast<std::size_t>(payload_of(value) & kHandleIndexMask);
}

std::uint32_t handle_generation(Word value) {
    return static_cast<std::uint32_t>((payload_of(value) >> kHandleGenerationShift) &
                                      kHandleGenerationMask);
}

std::size_t make_handle_payload(std::size_t index, std::uint32_t generation) {
    if ((static_cast<Word>(index) & ~kHandleIndexMask) != 0 ||
        (static_cast<Word>(generation) & ~kHandleGenerationMask) != 0) {
        throw std::runtime_error("Runtime error: handle de VM fuera de rango.");
    }
    return static_cast<std::size_t>((static_cast<Word>(generation) << kHandleGenerationShift) |
                                    static_cast<Word>(index));
}

}

Word make_nil() {
    return make_tagged(WordKind::Nil, 0);
}

Word make_number(double value) {
    Word bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & kTagMask) == kTagBase) {
        const double canonical_nan = std::numeric_limits<double>::quiet_NaN();
        std::memcpy(&bits, &canonical_nan, sizeof(bits));
    }
    return bits;
}

Word make_bool(bool value) {
    return make_tagged(WordKind::Bool, value ? 1 : 0);
}

Word make_string_ref(std::size_t index, std::uint32_t generation) {
    return make_tagged(WordKind::String, make_handle_payload(index, generation));
}

Word make_object_ref(std::size_t index, std::uint32_t generation) {
    return make_tagged(WordKind::Object, make_handle_payload(index, generation));
}

bool is_nil(Word value) { return kind_of(value) == WordKind::Nil; }
bool is_number(Word value) { return kind_of(value) == WordKind::Number; }
bool is_bool(Word value) { return kind_of(value) == WordKind::Bool; }
bool is_string(Word value) { return kind_of(value) == WordKind::String; }
bool is_object(Word value) { return kind_of(value) == WordKind::Object; }

double as_number(Word value) {
    if (!is_number(value)) throw std::runtime_error("Runtime error: se esperaba Number.");
    double number = 0.0;
    std::memcpy(&number, &value, sizeof(number));
    return number;
}

bool as_bool(Word value) {
    if (!is_bool(value)) throw std::runtime_error("Runtime error: se esperaba Boolean.");
    return payload_of(value) != 0;
}

std::size_t as_string_index(Word value) {
    if (!is_string(value)) throw std::runtime_error("Runtime error: se esperaba String.");
    return handle_index(value);
}

std::size_t as_object_index(Word value) {
    if (!is_object(value)) throw std::runtime_error("Runtime error: se esperaba Object.");
    return handle_index(value);
}

std::uint32_t as_string_generation(Word value) {
    if (!is_string(value)) throw std::runtime_error("Runtime error: se esperaba String.");
    return handle_generation(value);
}

std::uint32_t as_object_generation(Word value) {
    if (!is_object(value)) throw std::runtime_error("Runtime error: se esperaba Object.");
    return handle_generation(value);
}

bool truthy(Word value) {
    if (is_nil(value)) return false;
    if (is_bool(value)) return as_bool(value);
    return true;
}

std::string to_string(Word value, const VMHeap& heap) {
    if (is_nil(value)) return "nil";
    if (is_bool(value)) return as_bool(value) ? "true" : "false";
    if (is_string(value)) return heap.string_value(value);
    if (is_object(value)) return "<object>";

    const double number = as_number(value);
    if (!std::isnan(number) && number == static_cast<long long>(number)) {
        return std::to_string(static_cast<long long>(number));
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.14g", number);
    return buf;
}

}
