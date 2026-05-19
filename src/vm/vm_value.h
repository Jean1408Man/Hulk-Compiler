#ifndef HULK_VM_VALUE_H
#define HULK_VM_VALUE_H

#include <memory>
#include <string>
#include <variant>

namespace Hulk::VM {

struct VMNil {};

struct VMObject;
using VMObjectRef = std::shared_ptr<VMObject>;

struct VMValue {
    using Inner = std::variant<VMNil, double, bool, std::string, VMObjectRef>;
    Inner inner;

    VMValue() : inner(VMNil{}) {}
    explicit VMValue(double value) : inner(value) {}
    explicit VMValue(bool value) : inner(value) {}
    explicit VMValue(std::string value) : inner(std::move(value)) {}
    explicit VMValue(VMObjectRef value) : inner(std::move(value)) {}
};

bool is_nil(const VMValue& value);
bool is_number(const VMValue& value);
bool is_bool(const VMValue& value);
bool is_string(const VMValue& value);
bool is_object(const VMValue& value);

double as_number(const VMValue& value);
bool as_bool(const VMValue& value);
const std::string& as_string(const VMValue& value);
bool truthy(const VMValue& value);
std::string to_string(const VMValue& value);

}

#endif
