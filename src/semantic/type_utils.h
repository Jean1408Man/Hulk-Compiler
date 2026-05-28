#pragma once

#include "../inference/hulk_type.h"
#include "../semantic/semantic_tables.h"
#include "../semantic/semantic_type_info.h"
#include "../semantic/semantic_protocol_info.h"
#include "../ast/functions/param.h"
#include <string>
#include <unordered_map>
#include <cstddef>

namespace Hulk {

class Expr;

struct TypeQueryContext {
    const SemanticTables&                             tables;
    const std::unordered_map<const Param*, HulkType>& param_types;
    const std::unordered_map<Expr*, HulkType>&        type_map;
};

HulkType from_string_type(const std::string& type_name);

bool type_conforms_with_inference(const HulkType& found,
                                  const HulkType& expected,
                                  const TypeQueryContext& ctx);

bool type_conforms_to_protocol_inferred(const std::string& type_name,
                                        const std::string& protocol_name,
                                        const TypeQueryContext& ctx,
                                        int depth = 0);

bool method_satisfies_protocol_inferred(const SemanticMethodInfo& actual,
                                        const SemanticProtocolMethodInfo& required,
                                        const TypeQueryContext& ctx);

HulkType resolve_method_param_type(const SemanticMethodInfo& method,
                                   std::size_t index,
                                   const TypeQueryContext& ctx);

HulkType resolve_method_return_type(const SemanticMethodInfo& method,
                                    const TypeQueryContext& ctx);

} 