#include "typeMemberAttribute.h"

namespace Hulk {

    TypeMemberAttribute::TypeMemberAttribute(const std::string& name,
                                             std::unique_ptr<Expr> initializer)
        : name(name), typeAnnotation(""), isTypeHole(false), initializer(std::move(initializer)) {}

    TypeMemberAttribute::TypeMemberAttribute(const std::string& name,
                                             const std::string& typeAnnotation,
                                             std::unique_ptr<Expr> initializer)
        : name(name), typeAnnotation(typeAnnotation),
          isTypeHole(typeAnnotation == "_" || typeAnnotation == "auto"),
          initializer(std::move(initializer)) {}

    const std::string& TypeMemberAttribute::GetName() const { return name; }
    const std::string& TypeMemberAttribute::GetTypeAnnotation() const { return typeAnnotation; }
    bool TypeMemberAttribute::HasTypeAnnotation() const { return !typeAnnotation.empty() && !isTypeHole; }
    bool TypeMemberAttribute::IsTypeHole() const { return isTypeHole; }
    Expr* TypeMemberAttribute::GetInitializer() const { return initializer.get(); }

    std::string TypeMemberAttribute::ToString() const {
        std::string result = name;
        if (!typeAnnotation.empty())
            result += " : " + typeAnnotation;
        result += " = " + initializer->ToString();
        return result;
    }

}
