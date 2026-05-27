#include "lambda.h"

namespace Hulk {

    Lambda::Lambda(std::vector<Param> params,
                   std::unique_ptr<Expr> body)
        : params(std::move(params)), returnTypeAnnotation(""),
          returnIsTypeHole(false), body(std::move(body)) {}

    Lambda::Lambda(std::vector<Param> params,
                   const std::string& returnTypeAnnotation,
                   std::unique_ptr<Expr> body)
        : params(std::move(params)), returnTypeAnnotation(returnTypeAnnotation),
          returnIsTypeHole(returnTypeAnnotation == "_" || returnTypeAnnotation == "auto"),
          body(std::move(body)) {}

    const std::vector<Param>& Lambda::GetParams() const { return params; }
    const std::string& Lambda::GetReturnTypeAnnotation() const { return returnTypeAnnotation; }
    bool Lambda::HasReturnTypeAnnotation() const { return !returnTypeAnnotation.empty() && !returnIsTypeHole; }
    bool Lambda::IsReturnTypeHole() const { return returnIsTypeHole; }
    Expr* Lambda::GetBody() const { return body.get(); }

    std::string Lambda::ToString() const {
        std::string result = "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) result += ", ";
            result += params[i].name;
            if (params[i].HasTypeAnnotation())
                result += " : " + params[i].typeAnnotation;
        }
        result += ")";
        if (!returnTypeAnnotation.empty())
            result += " : " + returnTypeAnnotation;
        result += " => " + body->ToString();
        return result;
    }

}
