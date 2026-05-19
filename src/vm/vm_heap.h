#ifndef HULK_VM_HEAP_H
#define HULK_VM_HEAP_H

#include "vm_value.h"

#include <cstddef>
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

private:
    std::vector<VMObject> objects_;
    std::vector<VMString> strings_;
};

}

#endif
