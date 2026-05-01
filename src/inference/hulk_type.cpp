#include "hulk_type.h"

namespace Hulk {

    HulkType::HulkType() : kind_(Kind::Unknown), name_("") {}

    HulkType::HulkType(Kind kind) : kind_(kind), name_("") {}

    HulkType::HulkType(Kind kind, std::string name) : kind_(kind), name_(std::move(name)) {}

    bool HulkType::operator==(const HulkType& other) const {
        if (kind_ != other.kind_) return false;
        if (kind_ == Kind::Object && name_ != other.name_) return false;
        return true;
    }

    bool HulkType::operator!=(const HulkType& other) const {
        return !(*this == other);
    }

    std::string HulkType::to_string() const {
        switch (kind_) {
            case Kind::Number:  return "Number";
            case Kind::String:  return "String";
            case Kind::Boolean: return "Boolean";
            case Kind::Void:    return "Void";
            case Kind::Unknown: return "Unknown";
            case Kind::Error:   return "Error";
            case Kind::Object:  return name_;
            default:            return "???";
        }
    }

} // namespace Hulk
