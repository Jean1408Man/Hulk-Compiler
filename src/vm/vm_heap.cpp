#include "vm_heap.h"

#include <memory>

namespace Hulk::VM {

VMObjectRef VMHeap::allocate_object(int type_id, std::size_t field_count) {
    auto object = std::make_shared<VMObject>();
    object->type_id = type_id;
    object->fields.resize(field_count);
    objects_.push_back(object);
    return object;
}

} 
