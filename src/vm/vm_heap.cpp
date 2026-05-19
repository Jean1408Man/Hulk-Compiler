#include "vm_heap.h"

#include <stdexcept>
#include <utility>

namespace Hulk::VM {

Word VMHeap::allocate_object(int type_id, std::size_t field_count) {
    VMObject object;
    object.type_id = type_id;
    object.fields.resize(field_count, make_nil());
    objects_.push_back(std::move(object));
    return make_object_ref(objects_.size() - 1);
}

Word VMHeap::allocate_string(std::string value) {
    strings_.push_back(VMString{std::move(value)});
    return make_string_ref(strings_.size() - 1);
}

VMObject& VMHeap::object(Word value) {
    const std::size_t index = as_object_index(value);
    if (index >= objects_.size()) {
        throw std::runtime_error("Runtime error: referencia de objeto invalida.");
    }
    return objects_.at(index);
}

const VMObject& VMHeap::object(Word value) const {
    const std::size_t index = as_object_index(value);
    if (index >= objects_.size()) {
        throw std::runtime_error("Runtime error: referencia de objeto invalida.");
    }
    return objects_.at(index);
}

const std::string& VMHeap::string_value(Word value) const {
    const std::size_t index = as_string_index(value);
    if (index >= strings_.size()) {
        throw std::runtime_error("Runtime error: referencia de string invalida.");
    }
    return strings_.at(index).value;
}

}
