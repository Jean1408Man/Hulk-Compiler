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

        bool contains(const std::string& name) const {
            if (bindings_.count(name) || params_.count(name) || synthetics_.count(name))
                return true;
            if (parent_) return parent_->contains(name);
            return false;
        }

        // Devuelve el VariableBinding* si el nombre fue declarado via let.
        // nullptr si no existe en ningún scope o si fue declarado como param.
        VariableBinding* get_binding(const std::string& name) const {
            auto it = bindings_.find(name);
            if (it != bindings_.end()) return it->second;
            if (parent_) return parent_->get_binding(name);
            return nullptr;
        }

        // Devuelve el Param* si el nombre fue declarado como parámetro.
        // nullptr si no existe en ningún scope o si fue declarado como binding.
        const Param* get_param(const std::string& name) const {
            auto it = params_.find(name);
            if (it != params_.end()) return it->second;
            if (parent_) return parent_->get_param(name);
            return nullptr;
        }

        // Devuelve el SyntheticSymbol* si el nombre fue declarado sintéticamente.
        SyntheticSymbol* get_synthetic(const std::string& name) const {
            auto it = synthetics_.find(name);
            if (it != synthetics_.end()) return it->second;
            if (parent_) return parent_->get_synthetic(name);
            return nullptr;
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