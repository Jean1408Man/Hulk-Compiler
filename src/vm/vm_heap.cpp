#include "vm_heap.h"

#include <stdexcept>
#include <utility>

namespace Hulk::VM {
namespace {

constexpr std::size_t kCollectionThreshold = 1024;
constexpr std::uint32_t kMaxGeneration = 0xffffU;

}

Word VMHeap::allocate_object(int type_id, std::size_t field_count) {
    VMObject object;
    object.type_id = type_id;
    object.fields.resize(field_count, make_nil());
    ++allocations_since_gc_;

    if (!free_objects_.empty()) {
        const std::size_t index = free_objects_.back();
        free_objects_.pop_back();
        auto& slot = objects_.at(index);
        slot.value = std::move(object);
        slot.occupied = true;
        slot.marked = false;
        return make_object_ref(index, slot.generation);
    }

    objects_.push_back(HeapSlot<VMObject>{
        .value = std::move(object),
        .occupied = true,
        .marked = false,
        .generation = 0,
    });
    return make_object_ref(objects_.size() - 1, objects_.back().generation);
}

Word VMHeap::allocate_string(std::string value) {
    ++allocations_since_gc_;

    if (!free_strings_.empty()) {
        const std::size_t index = free_strings_.back();
        free_strings_.pop_back();
        auto& slot = strings_.at(index);
        slot.value = VMString{std::move(value)};
        slot.occupied = true;
        slot.marked = false;
        return make_string_ref(index, slot.generation);
    }

    strings_.push_back(HeapSlot<VMString>{
        .value = VMString{std::move(value)},
        .occupied = true,
        .marked = false,
        .generation = 0,
    });
    return make_string_ref(strings_.size() - 1, strings_.back().generation);
}

VMObject& VMHeap::object(Word value) {
    return checked_object_slot(value);
}

const VMObject& VMHeap::object(Word value) const {
    return checked_object_slot(value);
}

const std::string& VMHeap::string_value(Word value) const {
    return checked_string_slot(value).value;
}

void VMHeap::collect(const std::vector<Word>& roots) {
    for (const Word root : roots) {
        mark_word(root);
    }

    for (std::size_t i = 0; i < objects_.size(); ++i) {
        auto& slot = objects_.at(i);
        if (!slot.occupied) continue;
        if (slot.marked) {
            slot.marked = false;
        } else {
            release_object_slot(i);
        }
    }

    for (std::size_t i = 0; i < strings_.size(); ++i) {
        auto& slot = strings_.at(i);
        if (!slot.occupied) continue;
        if (slot.marked) {
            slot.marked = false;
        } else {
            release_string_slot(i);
        }
    }

    allocations_since_gc_ = 0;
}

bool VMHeap::should_collect() const {
    return allocations_since_gc_ >= kCollectionThreshold;
}

std::size_t VMHeap::live_object_count() const {
    std::size_t count = 0;
    for (const auto& slot : objects_) {
        if (slot.occupied) ++count;
    }
    return count;
}

std::size_t VMHeap::live_string_count() const {
    std::size_t count = 0;
    for (const auto& slot : strings_) {
        if (slot.occupied) ++count;
    }
    return count;
}

VMObject& VMHeap::checked_object_slot(Word value) {
    const std::size_t index = as_object_index(value);
    const std::uint32_t generation = as_object_generation(value);
    if (index >= objects_.size()) {
        throw std::runtime_error("Runtime error: referencia de objeto invalida.");
    }

    auto& slot = objects_.at(index);
    if (!slot.occupied || slot.generation != generation) {
        throw std::runtime_error("Runtime error: referencia de objeto invalida.");
    }
    return slot.value;
}

const VMObject& VMHeap::checked_object_slot(Word value) const {
    const std::size_t index = as_object_index(value);
    const std::uint32_t generation = as_object_generation(value);
    if (index >= objects_.size()) {
        throw std::runtime_error("Runtime error: referencia de objeto invalida.");
    }

    const auto& slot = objects_.at(index);
    if (!slot.occupied || slot.generation != generation) {
        throw std::runtime_error("Runtime error: referencia de objeto invalida.");
    }
    return slot.value;
}

const VMString& VMHeap::checked_string_slot(Word value) const {
    const std::size_t index = as_string_index(value);
    const std::uint32_t generation = as_string_generation(value);
    if (index >= strings_.size()) {
        throw std::runtime_error("Runtime error: referencia de string invalida.");
    }

    const auto& slot = strings_.at(index);
    if (!slot.occupied || slot.generation != generation) {
        throw std::runtime_error("Runtime error: referencia de string invalida.");
    }
    return slot.value;
}

void VMHeap::mark_word(Word value) {
    if (is_string(value)) {
        const std::size_t index = as_string_index(value);
        if (index >= strings_.size()) return;
        auto& slot = strings_.at(index);
        if (!slot.occupied || slot.generation != as_string_generation(value)) return;
        slot.marked = true;
        return;
    }

    if (!is_object(value)) return;

    const std::size_t index = as_object_index(value);
    if (index >= objects_.size()) return;
    auto& slot = objects_.at(index);
    if (!slot.occupied || slot.generation != as_object_generation(value) || slot.marked) return;

    slot.marked = true;
    for (const Word field : slot.value.fields) {
        mark_word(field);
    }
}

void VMHeap::release_object_slot(std::size_t index) {
    auto& slot = objects_.at(index);
    slot.value = VMObject{};
    slot.marked = false;
    slot.occupied = false;
    if (slot.generation < kMaxGeneration) {
        ++slot.generation;
        free_objects_.push_back(index);
    }
}

void VMHeap::release_string_slot(std::size_t index) {
    auto& slot = strings_.at(index);
    slot.value = VMString{};
    slot.marked = false;
    slot.occupied = false;
    if (slot.generation < kMaxGeneration) {
        ++slot.generation;
        free_strings_.push_back(index);
    }
}

}
