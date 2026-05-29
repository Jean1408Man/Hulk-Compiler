#ifndef PARAM_H
#define PARAM_H

#include <string>

namespace Hulk {

    struct Param {
        std::string name;
        std::string typeAnnotation;
        bool isTypeHole = false;

        Param()
            : name(""), typeAnnotation(""), isTypeHole(false) {}

        explicit Param(const std::string& name)
            : name(name), typeAnnotation(""), isTypeHole(false) {}

        Param(const std::string& name, const std::string& typeAnnotation)
            : name(name), typeAnnotation(typeAnnotation),
              isTypeHole(typeAnnotation == "_" || typeAnnotation == "auto") {}

        bool HasTypeAnnotation() const { return !typeAnnotation.empty() && !isTypeHole; }
        bool IsTypeHole() const { return isTypeHole; }
    };

}

#endif
