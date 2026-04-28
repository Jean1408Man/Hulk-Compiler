#ifndef HULK_SYMBOL_RESOLVER_H
#define HULK_SYMBOL_RESOLVER_H

#include "static_scope.h"
#include "../semantic/semantic_tables.h"
#include "../eval/visitor.h"
#include "../common/diagnosticEngine.hpp"
#include "../common/span.hpp"

#include <memory>
#include <unordered_map>
#include <string>

namespace Hulk {
    class Program;
    class Decl;
    class FunctionDecl;
    class TypeDecl;
    class TypeMemberAttribute;
    class TypeMemberMethod;
    class ProtocolDecl;
    class VariableReference;
    class VariableBinding;
    class LetIn;
    class DestructiveAssign;
    class DestructiveAssignMember;
    class FunctionCall;
    class Lambda;
    class IfStmt;
    class WhileStmt;
    class For;
    class Print;
    class BuiltinCall;
    class ExprBlock;
    class Group;
    class SelfRef;
    class BaseCall;
    class NewExpr;
    class MemberAccess;
    class MethodCall;
    class IsExpr;
    class AsExpr;
    class VectorLiteral;
    class VectorIndex;
    class VectorGenerator;
    class Number;
    class String;
    class Boolean;
    class ArithmeticBinOp;
    class LogicBinOp;
    class StringBinOp;
    class ArithmeticUnaryOp;
    class LogicUnaryOp;
}

namespace Hulk {

    // -----------------------------------------------------------------------
    // ResolutionResult — lo que el SymbolResolver sabe de un nombre concreto.
    //
    // Se guarda en el mapa externo: Expr* → ResolutionResult.
    // -----------------------------------------------------------------------
    enum class ResolutionKind {
        Variable,    // VariableBinding*
        Param,       // const Param*
        Function,    // FunctionDecl*
        Type,        // SemanticTypeInfo*
        Method,      // SemanticMethodInfo*
        Attribute,   // SemanticAttrInfo*
        Unresolved   // error — no se encontró
    };

    struct ResolutionResult {
        ResolutionKind kind = ResolutionKind::Unresolved;
        union {
            VariableBinding*         binding;
            const Param*             param;
            FunctionDecl*            func_decl;
            const SemanticTypeInfo*  type_info;
            const SemanticMethodInfo* method_info;
            const SemanticAttrInfo*  attr_info;
        };
        // Constructor por defecto — Unresolved
        ResolutionResult() : kind(ResolutionKind::Unresolved), binding(nullptr) {}

        static ResolutionResult from_binding(VariableBinding* b) {
            ResolutionResult r; r.kind = ResolutionKind::Variable; r.binding = b; return r;
        }
        static ResolutionResult from_param(const Param* p) {
            ResolutionResult r; r.kind = ResolutionKind::Param; r.param = p; return r;
        }
        static ResolutionResult from_func(FunctionDecl* f) {
            ResolutionResult r; r.kind = ResolutionKind::Function; r.func_decl = f; return r;
        }
        static ResolutionResult from_type(const SemanticTypeInfo* t) {
            ResolutionResult r; r.kind = ResolutionKind::Type; r.type_info = t; return r;
        }
        static ResolutionResult from_method(const SemanticMethodInfo* m) {
            ResolutionResult r; r.kind = ResolutionKind::Method; r.method_info = m; return r;
        }
        static ResolutionResult from_attr(const SemanticAttrInfo* a) {
            ResolutionResult r; r.kind = ResolutionKind::Attribute; r.attr_info = a; return r;
        }
    };

    // -----------------------------------------------------------------------
    // SymbolResolver — recorre el AST en tres pases:
    //
    //   Pase 1 (register_decls): recorre las declaraciones de nivel superior
    //           y llena SemanticTables con tipos y funciones.
    //
    //   Pase 2 (resolve_expr): recorre todas las expresiones del programa,
    //           mantiene scopes estáticos, y anota cada referencia en
    //           resolution_map_.
    //
    //   Pase 3 (run_checks): sobre las tablas ya construidas, verifica
    //           - padre declarado
    //           - sin ciclos de herencia
    //           - sin métodos duplicados en el mismo tipo
    //           - override solo si el método existe en el padre
    //           - aridades correctas en FunctionCall y NewExpr
    //           - VariableReference a variable declarada
    //
    // Uso:
    //   SymbolResolver resolver(tables, engine);
    //   resolver.run(program);
    //   auto& map = resolver.resolution_map();
    // -----------------------------------------------------------------------
    class SymbolResolver : public ExprVisitor, public DeclVisitor {
    public:
        SymbolResolver(SemanticTables& tables,
                       hulk::common::DiagnosticEngine& engine);

        // Punto de entrada: ejecuta los tres pases sobre el programa.
        // Devuelve false si se encontraron errores semánticos.
        bool run(Program& program);

        // Acceso al mapa de resolución
        const std::unordered_map<Expr*, ResolutionResult>& resolution_map() const {
            return resolution_map_;
        }

        bool has_errors() const { return has_errors_; }

    private:
        // Estado
        SemanticTables&                  tables_;
        hulk::common::DiagnosticEngine&  engine_;
        std::shared_ptr<StaticScope>     scope_;
        std::unordered_map<Expr*, ResolutionResult> resolution_map_;
        bool has_errors_ = false;

        // Nombre del tipo en cuyo cuerpo estamos (para self, override, etc.)
        std::string current_type_name_;
        // Nombre de la función/método actual (para errores contextuales)
        std::string current_func_name_;

        // Helpers de scope
        void push_scope();
        void pop_scope();
        std::shared_ptr<StaticScope> make_child_scope();

        // Reporte de errores — no lanza excepción, continúa el recorrido
        void report(const hulk::common::Span& span,
                    const std::string& error_id);
        void report(const hulk::common::Span& span,
                    const std::string& error_id,
                    const std::string& arg1);
        void report(const hulk::common::Span& span,
                    const std::string& error_id,
                    const std::string& arg1,
                    const std::string& arg2);
        void report_raw(const hulk::common::Span& span,
                        const std::string& msg);

        // Pase 1 — Registrar declaraciones en las tablas
        // (implementado como DeclVisitor sobre program.GetDeclarations())
        void visit(FunctionDecl& n) override;
        void visit(TypeDecl& n)     override;
        void visit(TypeMemberAttribute& n) override;  // no-op (procesado en TypeDecl)
        void visit(TypeMemberMethod& n)    override;  // no-op (procesado en TypeDecl)
        void visit(ProtocolDecl& n)        override;  // no-op por ahora

        // Pase 3 — Chequeos globales sobre las tablas ya construidas
        void run_checks();
        void check_inheritance();   // padre declarado + sin ciclos
        void check_methods();       // duplicados + override válido
        void check_arities();       // aridades de FunctionCall y NewExpr (via tablas)

        // Pase 2 — Resolver referencias (ExprVisitor)

        // Literales — no resuelven nada, son hojas
        void visit(Number& n)             override;
        void visit(String& n)             override;
        void visit(Boolean& n)            override;

        // Operaciones — recorren subexpresiones
        void visit(ArithmeticBinOp& n)    override;
        void visit(LogicBinOp& n)         override;
        void visit(StringBinOp& n)        override;
        void visit(ArithmeticUnaryOp& n)  override;
        void visit(LogicUnaryOp& n)       override;

        // Builtins
        void visit(Print& n)              override;
        void visit(BuiltinCall& n)        override;

        // Bloques
        void visit(ExprBlock& n)          override;
        void visit(Group& n)              override;

        // Variables — anotan en resolution_map_
        void visit(VariableReference& n)  override;
        void visit(VariableBinding& n)    override;
        void visit(LetIn& n)              override;

        // Asignaciones
        void visit(DestructiveAssign& n)        override;
        void visit(DestructiveAssignMember& n)  override;

        // Control de flujo
        void visit(IfStmt& n)             override;
        void visit(WhileStmt& n)          override;
        void visit(For& n)                override;

        // Funciones — anotan en resolution_map_
        void visit(FunctionCall& n)       override;
        void visit(Lambda& n)             override;

        // OOP — anotan en resolution_map_
        void visit(NewExpr& n)            override;
        void visit(MemberAccess& n)       override;
        void visit(MethodCall& n)         override;
        void visit(SelfRef& n)            override;
        void visit(BaseCall& n)           override;
        void visit(IsExpr& n)             override;
        void visit(AsExpr& n)             override;

        // Vectores
        void visit(VectorLiteral& n)      override;
        void visit(VectorIndex& n)        override;
        void visit(VectorGenerator& n)    override;

        // Helper: resolver una expresión (como eval() en el Evaluator)
        void resolve(Expr* node);
    };

}

#endif