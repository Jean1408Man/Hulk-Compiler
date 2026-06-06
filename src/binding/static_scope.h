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

    enum class SyntheticKind {
        ForVariable, 
        Self         
    };

    struct SyntheticSymbol {
        std::string   name;
        SyntheticKind kind;
        std::string   type_name; 
    };

    // StaticScope — tabla de símbolos para análisis estático (sin valores).
    //   - VariableBinding* → para variables declaradas via let/in
    //   - Param* (const)   → para parámetros de funciones y métodos
    //   - SyntheticSymbol* → para variables sintéticas (for, self, etc.)
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
            auto it_b = bindings_.find(name);
            if (it_b != bindings_.end()) return { .binding = it_b->second };

            auto it_p = params_.find(name);
            if (it_p != params_.end()) return { .param = it_p->second };

            auto it_s = synthetics_.find(name);
            if (it_s != synthetics_.end()) return { .synthetic = it_s->second };

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