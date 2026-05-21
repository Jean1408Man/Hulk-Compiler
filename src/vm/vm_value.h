#ifndef HULK_VM_VALUE_H
#define HULK_VM_VALUE_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace Hulk::VM {

class VMHeap;

using Word = std::uint64_t;

enum class WordKind : std::uint8_t {
    Number,
    Nil,
    Bool,
    String,
    Object
};

Word make_nil();
Word make_number(double value);
Word make_bool(bool value);
Word make_string_ref(std::size_t index, std::uint32_t generation = 0);
Word make_object_ref(std::size_t index, std::uint32_t generation = 0);

bool is_nil(Word value);
bool is_number(Word value);
bool is_bool(Word value);
bool is_string(Word value);
bool is_object(Word value);

double as_number(Word value);
bool as_bool(Word value);
std::size_t as_string_index(Word value);
std::size_t as_object_index(Word value);
std::uint32_t as_string_generation(Word value);
std::uint32_t as_object_generation(Word value);

bool truthy(Word value);
std::string to_string(Word value, const VMHeap& heap);

}

#endif
