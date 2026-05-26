#ifndef HULK_SEMANTIC_PROTOCOL_INFO_H
#define HULK_SEMANTIC_PROTOCOL_INFO_H

#include "../ast/functions/param.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Hulk { class ProtocolDecl; }

namespace Hulk {

    struct SemanticProtocolMethodInfo {
        std::string name;
        std::vector<Param> params;
        std::string return_type_annotation;
    };

    struct SemanticProtocolInfo {
        std::string name;
        std::string parent_name;
        std::unordered_map<std::string, SemanticProtocolMethodInfo> methods;
        ProtocolDecl* decl = nullptr;
        bool is_builtin = false;
    };

}

#endif
