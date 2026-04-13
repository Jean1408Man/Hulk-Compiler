#ifndef DECL_H
#define DECL_H

#include "ast.h"
#include "../../common/span.hpp"

namespace Hulk {

    class Decl : public ASTnode {
    public:
        hulk::common::Span span {};
        virtual ~Decl() = default;
    };

}

#endif
