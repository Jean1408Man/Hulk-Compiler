#ifndef HULK_STATIC_SCOPE_H
#define HULK_STATIC_SCOPE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>

namespace Hulk {
    class VariableBinding;
    struct Param;
}

namespace Hulk {

    // -----------------------------------------------------------------------
    // SyntheticSymbol — símbolo introducido implícitamente por el compilador,
    // sin un nodo VariableBinding ni Param en el AST.
    //
    // Ejemplos: variable del for, variable del VectorGenerator, self.
    // -----------------------------------------------------------------------
    enum class SyntheticKind {
        ForVariable,             // variable del for (x in ...)
        VectorGeneratorVariable, // variable del [e | x in ...]
        Self                     // self dentro de un método
    };

    struct SyntheticSymbol {
        std::string   name;
        SyntheticKind kind;
        std::string   type_name; // para Self: nombre del tipo actual; vacío para otros
    };

    // -----------------------------------------------------------------------
    // StaticScope — tabla de símbolos para análisis estático (sin valores).
    //
    // Análogo a Environment (src/eval/environment.h), pero almacena
    // punteros a declaraciones en lugar de valores en runtime:
    //
    //   - VariableBinding* → para variables declaradas via let/in
    //   - Param* (const)   → para parámetros de funciones y métodos
    //   - SyntheticSymbol* → para variables sintéticas (for, self, etc.)
    //
    // Se encadena igual que Environment: cada scope apunta a su padre.
    // La búsqueda sube la cadena hasta el scope global (parent == nullptr).
    // -----------------------------------------------------------------------
    class StaticScope {
    public:
        explicit StaticScope(std::shared_ptr<StaticScope> parent = nullptr)
            : parent_(std::move(parent)) {}

        void define_binding(const std::string& name, VariableBinding* binding) {
            bindings_[name] = binding;
        }
        void define_param(const std::string& name, const Param* param) {
            params_[name] = param;
        }
        void define_synthetic(const std::string& name, SyntheticSymbol* symbol) {
            synthetics_[name] = symbol;
        }

        struct ResolvedSymbol {
            VariableBinding* binding   = nullptr;
            const Param*     param     = nullptr;
            SyntheticSymbol* synthetic = nullptr;

            bool is_resolved() const { return binding || param || synthetic; }
        };

        ResolvedSymbol lookup(const std::string& name) const {
            // Buscar en el scope actual (todas las categorías)
            auto it_b = bindings_.find(name);
            if (it_b != bindings_.end()) return { .binding = it_b->second };

            auto it_p = params_.find(name);
            if (it_p != params_.end()) return { .param = it_p->second };

            auto it_s = synthetics_.find(name);
            if (it_s != synthetics_.end()) return { .synthetic = it_s->second };

            // Si no está, buscar en el padre
            if (parent_) return parent_->lookup(name);
            return {};
        }

        std::shared_ptr<StaticScope> parent() const { return parent_; }

    private:
        std::unordered_map<std::string, VariableBinding*>  bindings_;
        std::unordered_map<std::string, const Param*>      params_;
        std::unordered_map<std::string, SyntheticSymbol*>  synthetics_;
        std::shared_ptr<StaticScope>                       parent_;
    };

}

#endif