#ifndef LETIN_H
#define LETIN_H
#include "../abs_nodes/ast.h"
#include <memory>

namespace Hulk {

    class LetIn : public ASTnode {
    private:
        std::unique_ptr<ASTnode> binding;
        std::unique_ptr<ASTnode> body;

    public:
        LetIn(std::unique_ptr<ASTnode> binding, std::unique_ptr<ASTnode> body);

        ASTnode* GetBinding() const;
        ASTnode* GetBody() const;

        std::string ToString() const override;
    };

}

#endif