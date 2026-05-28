#include "type_utils.h"

namespace {

std::string nominal_type_name(const Hulk::HulkType& type) {
    switch (type.kind()) {
        case Hulk::HulkType::Kind::Number:  return "Number";
        case Hulk::HulkType::Kind::String:  return "String";
        case Hulk::HulkType::Kind::Boolean: return "Boolean";
        case Hulk::HulkType::Kind::Object:  return type.name();
        default: return "";
    }
}

} 

namespace Hulk {

HulkType from_string_type(const std::string& type_name) {
    if (type_name.empty() || type_name == "auto" || type_name == "_")
        return HulkType::make_unknown();
    if (type_name == "Number")  return HulkType::make_number();
    if (type_name == "String")  return HulkType::make_string();
    if (type_name == "Boolean") return HulkType::make_boolean();
    if (type_name == "Void")    return HulkType::make_void();
    return HulkType::make_object(type_name);
}

bool type_conforms_with_inference(const HulkType& found,
                                  const HulkType& expected,
                                  const TypeQueryContext& ctx) {
    if (found.is_error() || expected.is_error()) return true;
    if (found == expected) return true;
    if (expected.kind() == HulkType::Kind::Object && expected.name() == "Object") return true;

    const std::string found_name    = nominal_type_name(found);
    const std::string expected_name = nominal_type_name(expected);

    if (!expected_name.empty() && ctx.tables.lookup_protocol(expected_name)) {
        if (!found_name.empty() && ctx.tables.lookup_protocol(found_name))
            return ctx.tables.protocol_conforms_to_protocol(found_name, expected_name);
        if (!found_name.empty())
            return type_conforms_to_protocol_inferred(found_name, expected_name, ctx);
    }

    return found.conforms_to(expected, ctx.tables);
}

bool type_conforms_to_protocol_inferred(const std::string& type_name,
                                        const std::string& protocol_name,
                                        const TypeQueryContext& ctx,
                                        int depth) {
    if (depth > 256) return false;

    const SemanticTypeInfo*     type     = ctx.tables.lookup_type(type_name);
    const SemanticProtocolInfo* protocol = ctx.tables.lookup_protocol(protocol_name);
    if (!type || !protocol) return false;

    if (!protocol->parent_name.empty() &&
        !type_conforms_to_protocol_inferred(type_name, protocol->parent_name, ctx, depth + 1))
        return false;

    for (const auto& [method_name, required] : protocol->methods) {
        const SemanticMethodInfo* actual = ctx.tables.find_method(type_name, method_name);
        if (!actual || !method_satisfies_protocol_inferred(*actual, required, ctx))
            return false;
    }
    return true;
}

bool method_satisfies_protocol_inferred(const SemanticMethodInfo& actual,
                                        const SemanticProtocolMethodInfo& required,
                                        const TypeQueryContext& ctx) {
    if (actual.params.size() != required.params.size()) return false;

    for (std::size_t i = 0; i < actual.params.size(); ++i) {
        const HulkType actual_param   = resolve_method_param_type(actual, i, ctx);
        const HulkType required_param = from_string_type(required.params[i].typeAnnotation);
        if (actual_param.is_unknown() || required_param.is_unknown()) return false;
        if (!type_conforms_with_inference(required_param, actual_param, ctx)) return false;
    }

    const HulkType actual_return   = resolve_method_return_type(actual, ctx);
    const HulkType required_return = from_string_type(required.return_type_annotation);
    if (actual_return.is_unknown() || required_return.is_unknown()) return false;
    return type_conforms_with_inference(actual_return, required_return, ctx);
}

HulkType resolve_method_param_type(const SemanticMethodInfo& method,
                                   std::size_t index,
                                   const TypeQueryContext& ctx) {
    if (index >= method.params.size()) return HulkType::make_unknown();

    HulkType type = from_string_type(method.params[index].typeAnnotation);
    if (!type.is_unknown()) return type;

    if (index < method.ast_params.size()) {
        auto it = ctx.param_types.find(method.ast_params[index]);
        if (it != ctx.param_types.end()) return it->second;
    }
    return HulkType::make_unknown();
}

HulkType resolve_method_return_type(const SemanticMethodInfo& method,
                                    const TypeQueryContext& ctx) {
    HulkType type = from_string_type(method.return_type_annotation);
    if (!type.is_unknown()) return type;

    if (method.body) {
        auto it = ctx.type_map.find(method.body);
        if (it != ctx.type_map.end()) return it->second;
    }
    return HulkType::make_unknown();
}

} 