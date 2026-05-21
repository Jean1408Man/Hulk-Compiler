#include "vm/vm_heap.h"
#include "vm/vm_value.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Hulk::VM;

namespace {

bool throws_runtime_error(auto&& fn) {
    try {
        fn();
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

void string_without_root_is_collected() {
    VMHeap heap;
    const Word value = heap.allocate_string("temp");
    assert(heap.live_string_count() == 1);

    heap.collect({});

    assert(heap.live_string_count() == 0);
    assert(throws_runtime_error([&] { (void)heap.string_value(value); }));
}

void string_root_survives() {
    VMHeap heap;
    const Word value = heap.allocate_string("rooted");

    heap.collect({value});

    assert(heap.live_string_count() == 1);
    assert(heap.string_value(value) == "rooted");
}

void object_root_marks_fields() {
    VMHeap heap;
    const Word child = heap.allocate_string("child");
    const Word parent = heap.allocate_object(7, 1);
    heap.object(parent).fields.at(0) = child;

    heap.collect({parent});

    assert(heap.live_object_count() == 1);
    assert(heap.live_string_count() == 1);
    assert(heap.object(parent).type_id == 7);
    assert(heap.string_value(child) == "child");
}

void unreachable_object_cycle_is_collected() {
    VMHeap heap;
    const Word a = heap.allocate_object(1, 1);
    const Word b = heap.allocate_object(2, 1);
    heap.object(a).fields.at(0) = b;
    heap.object(b).fields.at(0) = a;

    heap.collect({});

    assert(heap.live_object_count() == 0);
    assert(throws_runtime_error([&] { (void)heap.object(a); }));
    assert(throws_runtime_error([&] { (void)heap.object(b); }));
}

void freed_slot_is_reused_with_new_generation() {
    VMHeap heap;
    const Word old_value = heap.allocate_string("old");
    const std::size_t old_index = as_string_index(old_value);
    const std::uint32_t old_generation = as_string_generation(old_value);

    heap.collect({});

    const Word new_value = heap.allocate_string("new");
    assert(as_string_index(new_value) == old_index);
    assert(as_string_generation(new_value) == old_generation + 1);
    assert(heap.string_value(new_value) == "new");
    assert(throws_runtime_error([&] { (void)heap.string_value(old_value); }));
}

void old_object_handle_is_rejected_after_reuse() {
    VMHeap heap;
    const Word old_object = heap.allocate_object(1, 0);
    const std::size_t old_index = as_object_index(old_object);
    const std::uint32_t old_generation = as_object_generation(old_object);

    heap.collect({});

    const Word new_object = heap.allocate_object(2, 0);
    assert(as_object_index(new_object) == old_index);
    assert(as_object_generation(new_object) == old_generation + 1);
    assert(heap.object(new_object).type_id == 2);
    assert(throws_runtime_error([&] { (void)heap.object(old_object); }));
}

}

int main() {
    string_without_root_is_collected();
    string_root_survives();
    object_root_marks_fields();
    unreachable_object_cycle_is_collected();
    freed_slot_is_reused_with_new_generation();
    old_object_handle_is_rejected_after_reuse();
    return 0;
}
