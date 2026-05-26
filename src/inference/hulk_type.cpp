#include "hulk_type.h"
#include "../semantic/semantic_tables.h"

namespace Hulk {
namespace {

std::string nominal_name(const HulkType& type) {
    switch (type.kind()) {
        case HulkType::Kind::Number: return "Number";
        case HulkType::Kind::String: return "String";
        case HulkType::Kind::Boolean: return "Boolean";
        case HulkType::Kind::Object: return type.name();
        default: return "";
    }
}

}

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

    bool HulkType::conforms_to(const HulkType& other, const SemanticTables& tables) const {
        if (is_error() || other.is_error()) return true; // Evitar cascadas de errores
        if (*this == other) return true;

        if (other.kind() == Kind::Object && other.name() == "Object") return true;

        const std::string this_name = nominal_name(*this);
        const std::string other_name = nominal_name(other);

        if (!other_name.empty() && tables.lookup_protocol(other_name)) {
            if (!this_name.empty() && tables.lookup_protocol(this_name)) {
                return tables.protocol_conforms_to_protocol(this_name, other_name);
            }
            if (!this_name.empty()) {
                return tables.type_conforms_to_protocol(this_name, other_name);
            }
        }

        if (kind_ == Kind::Object && other.kind() == Kind::Object &&
            !tables.lookup_protocol(name_) && !tables.lookup_protocol(other.name())) {
            return tables.is_subtype(name_, other.name());
        }

        return false;
    }

    bool HulkType::can_unify(const HulkType& other, const SemanticTables& tables) const {
        if (is_error() || other.is_error()) return true;
        if (is_unknown() || other.is_unknown()) return true;
        return conforms_to(other, tables);
    }

}
