#ifndef HULK_VM_HEAP_H
#define HULK_VM_HEAP_H

#include "vm_value.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Hulk::VM {

struct VMObject {
    int type_id = -1;
    std::vector<Word> fields;
};

struct VMString {
    std::string value;
};

class VMHeap {
public:
    Word allocate_object(int type_id, std::size_t field_count);
    Word allocate_string(std::string value);

    VMObject& object(Word value);
    const VMObject& object(Word value) const;
    const std::string& string_value(Word value) const;
    void collect(const std::vector<Word>& roots);

    [[nodiscard]] bool should_collect() const;
    [[nodiscard]] std::size_t live_object_count() const;
    [[nodiscard]] std::size_t live_string_count() const;

private:
    template <typename T>
    struct HeapSlot {
        T value {};
        bool occupied = false;
        bool marked = false;
        std::uint32_t generation = 0;
    };

    VMObject& checked_object_slot(Word value);
    const VMObject& checked_object_slot(Word value) const;
    const VMString& checked_string_slot(Word value) const;
    void mark_word(Word value);
    void release_object_slot(std::size_t index);
    void release_string_slot(std::size_t index);

    std::vector<HeapSlot<VMObject>> objects_;
    std::vector<HeapSlot<VMString>> strings_;
    std::vector<std::size_t> free_objects_;
    std::vector<std::size_t> free_strings_;
    std::size_t allocations_since_gc_ = 0;
};

}

#endif
