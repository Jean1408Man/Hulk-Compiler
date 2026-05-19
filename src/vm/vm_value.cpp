#include "vm_value.h"

#include <stdexcept>

namespace Hulk::VM {

bool is_nil(const VMValue& value) { return std::holds_alternative<VMNil>(value.inner); }
bool is_number(const VMValue& value) { return std::holds_alternative<double>(value.inner); }
bool is_bool(const VMValue& value) { return std::holds_alternative<bool>(value.inner); }
bool is_string(const VMValue& value) { return std::holds_alternative<std::string>(value.inner); }
bool is_object(const VMValue& value) { return std::holds_alternative<VMObjectRef>(value.inner); }

double as_number(const VMValue& value) {
    if (!is_number(value)) throw std::runtime_error("Runtime error: se esperaba Number.");
    return std::get<double>(value.inner);
}

bool as_bool(const VMValue& value) {
    if (!is_bool(value)) throw std::runtime_error("Runtime error: se esperaba Boolean.");
    return std::get<bool>(value.inner);
}

const std::string& as_string(const VMValue& value) {
    if (!is_string(value)) throw std::runtime_error("Runtime error: se esperaba String.");
    return std::get<std::string>(value.inner);
}

bool truthy(const VMValue& value) {
    if (is_nil(value)) return false;
    if (is_bool(value)) return std::get<bool>(value.inner);
    return true;
}

std::string to_string(const VMValue& value) {
    struct Visitor {
        std::string operator()(VMNil) const { return "nil"; }
        std::string operator()(double number) const {
            if (number == static_cast<long long>(number)) {
                return std::to_string(static_cast<long long>(number));
            }
            return std::to_string(number);
        }
        std::string operator()(bool value) const { return value ? "true" : "false"; }
        std::string operator()(const std::string& value) const { return value; }
        std::string operator()(const VMObjectRef&) const { return "<object>"; }
    };
    return std::visit(Visitor{}, value.inner);
}

}
