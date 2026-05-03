#include "hulk_type.h"
#include "../semantic/semantic_tables.h"

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

    bool HulkType::conforms_to(const HulkType& other, const SemanticTables& tables) const {
        if (is_error() || other.is_error()) return true; // Evitar cascadas de errores
        if (*this == other) return true;

        if (other.kind() == Kind::Object && other.name() == "Object") return true;

        if (kind_ == Kind::Object && other.kind() == Kind::Object) {
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
