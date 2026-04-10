#include "letIn.h"

namespace Hulk {

    LetIn::LetIn(std::unique_ptr<ASTnode> binding, std::unique_ptr<ASTnode> body)
        : binding(std::move(binding)), body(std::move(body)) {}

    ASTnode* LetIn::GetBinding() const {
        return binding.get();
    }

    ASTnode* LetIn::GetBody() const {
        return body.get();
    }

    std::string LetIn::ToString() const {
        return "let " + binding->ToString() + " in " + body->ToString();
    }
}