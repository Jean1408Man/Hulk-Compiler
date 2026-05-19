#ifndef HULK_VM_HEAP_H
#define HULK_VM_HEAP_H

#include "vm_value.h"
#include <vector>

namespace Hulk::VM {

struct VMObject {
    int type_id = -1;
    std::vector<VMValue> fields;
};

class VMHeap {
public:
    VMObjectRef allocate_object(int type_id, std::size_t field_count);

private:
    std::vector<VMObjectRef> objects_;
};

}

#endif
