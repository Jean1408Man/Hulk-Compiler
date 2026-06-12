SHELL    := bash
export TEMP := /tmp
export TMP  := /tmp
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic -Isrc
CXXFLAGS_GENERATED := -std=c++20 -w -Isrc

OBJDIR := build
PARSERGEN := $(OBJDIR)/tools/parsergen/parsergen

# ─────────────────────────────────────────────────────────────────────────────
# Fuentes compartidas: lexer + todos los nodos del AST + accept() centralizado
# ─────────────────────────────────────────────────────────────────────────────
LEXER_AST_SRCS := \
	src/lexer/lexer.cpp \
	src/ast/literales/number.cpp \
	src/ast/literales/string.cpp \
	src/ast/literales/boolean.cpp \
	src/ast/variables/variableReference.cpp \
	src/ast/variables/variableBinding.cpp \
	src/ast/variables/letIn.cpp \
	src/ast/assignments/desctructiveAssign.cpp \
	src/ast/assignments/destructiveAssignMember.cpp \
	src/ast/binOps/arithmeticBinOp.cpp \
	src/ast/binOps/logicBinOp.cpp \
	src/ast/binOps/stringBinOp.cpp \
	src/ast/conditionals/ifStmt.cpp \
	src/ast/unaryOps/arithmeticUnaryOp.cpp \
	src/ast/unaryOps/logicUnaryOp.cpp \
	src/ast/domainFunctions/print.cpp \
	src/ast/domainFunctions/builtinCall.cpp \
	src/ast/functions/functionCall.cpp \
	src/ast/functions/functionDecl.cpp \
	src/ast/loops/for.cpp \
	src/ast/loops/while.cpp \
	src/ast/others/baseCall.cpp \
	src/ast/others/exprBlock.cpp \
	src/ast/others/group.cpp \
	src/ast/others/program.cpp \
	src/ast/others/selfRef.cpp \
	src/ast/protocols/protocolDecl.cpp \
	src/ast/types/asExpr.cpp \
	src/ast/types/isExpr.cpp \
	src/ast/types/memberAccess.cpp \
	src/ast/types/methodCall.cpp \
	src/ast/types/newExpr.cpp \
	src/ast/types/typeDecl.cpp \
	src/ast/types/typeMemberAttribute.cpp \
	src/ast/types/typeMemberMethod.cpp \
	src/ast/accept_impl.cpp

EVAL_SRCS := \
	src/eval/evaluator.cpp

SEMANTIC_SRCS := \
	src/objects/hulk_value.cpp \
	src/semantic/semantic_tables.cpp \
	src/semantic/analyzer.cpp \
	src/semantic/type_utils.cpp \
	src/binding/symbol_resolver.cpp \
	src/inference/hulk_type.cpp \
	src/inference/type_inferencer.cpp \
	src/typecheck/type_checker.cpp

BACKEND_SRCS := \
	src/banner/banner_ir.cpp \
	src/banner/banner_printer.cpp \
	src/banner/banner_serializer.cpp \
	src/ir/ir.cpp \
	src/ir/ir_printer.cpp \
	src/backend/backend.cpp \
	src/backend/backend_driver.cpp \
	src/backend/codegen_context.cpp \
	src/backend/hulkir_to_banner.cpp \
	src/backend/ir_gen.cpp \
	src/backend/name_mangler.cpp \
	src/backend/output_packager.cpp \
	src/vm/banner_vm.cpp \
	src/vm/vm_heap.cpp \
	src/vm/vm_value.cpp

SEMANTIC_OBJS := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SEMANTIC_SRCS))
BACKEND_OBJS  := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(BACKEND_SRCS))

# Objetos pre-compilados (se reusan entre targets para no recompilar el parser)
LEXER_AST_OBJS := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(LEXER_AST_SRCS))
PARSER_OBJS    := $(OBJDIR)/parser/lr_engine.o \
                  $(OBJDIR)/parser/parser_tables.o \
                  $(OBJDIR)/parser/parser_driver.o \
                  $(OBJDIR)/parser/parser_lexer_adapter.o
EVAL_OBJS      := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(EVAL_SRCS))

# Archivo de entrada por defecto para run-eval / run-vm / emit-banner
FILE ?= examples/example.hulk

.PHONY: all build compile run-eval run-vm emit-banner eval-restricted-tests parser-gen parser-gen-own parser-gen-bison parser-sync-check parser-sync-check-own parser-sync-check-bison lexer parser-demo parser-tests eval eval-tests err-tests semantic semantic-tests extension-tests backend vm-tests backend-tests end-to-end-tests run-tests update-expected clean

all: lexer parser-demo eval semantic

# ─────────────────────────────────────────────────────────────────────────────
# Comandos principales de usuario
# ─────────────────────────────────────────────────────────────────────────────

# Compila todos los binarios principales (evaluador + compilador completo)
compile: eval backend

# Ejecuta un archivo .hulk mediante el evaluador de árbol (sin backend)
#   make run-eval FILE=examples/example.hulk
run-eval: eval
	./hulk_eval $(FILE)

# Ejecuta un archivo .hulk mediante el pipeline completo (IR → BannerVM)
#   make run-vm FILE=examples/example.hulk
run-vm: backend
	./hulk_backend --run-banner $(FILE)

# Emite el Banner IR generado: lo guarda en outputs/ y lo imprime en terminal
#   make emit-banner FILE=examples/example.hulk
emit-banner: backend
	@mkdir -p outputs
	@name=$$(basename $(FILE) .hulk); \
	out=outputs/$${name}.banner; \
	./hulk_backend --emit-banner -o $$out $(FILE) && \
	echo "--- Banner IR: $$out ---" && \
	cat $$out

# Corre los tests de restricted-inference usando el evaluador de árbol
eval-restricted-tests: eval
	@bash tests/eval/run_eval_restricted_tests.sh

# ─────────────────────────────────────────────────────────────────────────────
# Directorios de objetos
# ─────────────────────────────────────────────────────────────────────────────
$(OBJDIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PARSERGEN): tools/parsergen/parsergen.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

src/parser/parser_tables.cpp src/parser/parser_tables.hpp: src/parser/hulk.grammar $(PARSERGEN) doc/parser/expected_conflicts.txt
	$(PARSERGEN) src/parser/hulk.grammar -o src/parser/parser_tables --conflicts doc/parser/expected_conflicts.txt

$(OBJDIR)/parser/lr_engine.o: src/parser/lr_engine.cpp src/parser/parser_tables.hpp
	@mkdir -p $(OBJDIR)/parser
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/parser/parser_tables.o: src/parser/parser_tables.cpp src/parser/parser_tables.hpp
	@mkdir -p $(OBJDIR)/parser
	$(CXX) $(CXXFLAGS_GENERATED) -c $< -o $@

$(OBJDIR)/parser/parser_driver.o: src/parser/parser_driver.cpp
	@mkdir -p $(OBJDIR)/parser
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/parser/parser_lexer_adapter.o: src/parser/parser_lexer_adapter.cpp src/parser/parser_tables.hpp
	@mkdir -p $(OBJDIR)/parser
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ─────────────────────────────────────────────────────────────────────────────
# Targets de binarios
# ─────────────────────────────────────────────────────────────────────────────
parser-gen: parser-gen-own

parser-gen-own: src/parser/parser_tables.cpp src/parser/parser_tables.hpp

parser-sync-check: parser-sync-check-own

parser-sync-check-own: $(PARSERGEN)
	@set -e; \
	tmp_dir=$$(mktemp -d); \
	trap 'rm -rf "$$tmp_dir"' EXIT; \
	mkdir -p "$$tmp_dir/src/parser"; \
	$(PARSERGEN) src/parser/hulk.grammar -o "$$tmp_dir/src/parser/parser_tables" --conflicts doc/parser/expected_conflicts.txt; \
	diff -u src/parser/parser_tables.cpp "$$tmp_dir/src/parser/parser_tables.cpp"; \
	diff -u src/parser/parser_tables.hpp "$$tmp_dir/src/parser/parser_tables.hpp"; \
	echo "parser-sync-check-own: tablas del parser sincronizadas con hulk.grammar"

parser-gen-bison:
	@if [[ "$(USE_BISON)" != "1" ]]; then \
		echo "parser-gen-bison es solo oraculo: ejecuta USE_BISON=1 make parser-gen-bison"; \
		exit 1; \
	fi
	@set -e; \
	tmp_dir=$$(mktemp -d); \
	mkdir -p "$$tmp_dir/src/parser"; \
	cp src/parser/grammar.y "$$tmp_dir/src/parser/grammar.y"; \
	( cd "$$tmp_dir" && bison --report=all -Wcounterexamples -d -o src/parser/parser.cpp src/parser/grammar.y ); \
	echo "parser-gen-bison: oraculo generado en $$tmp_dir/src/parser"

parser-sync-check-bison:
	@if [[ "$(USE_BISON)" != "1" ]]; then \
		echo "parser-sync-check-bison es solo oraculo: ejecuta USE_BISON=1 make parser-sync-check-bison"; \
		exit 1; \
	fi
	@set -e; \
	tmp_dir=$$(mktemp -d); \
	trap 'rm -rf "$$tmp_dir"' EXIT; \
	mkdir -p "$$tmp_dir/src/parser"; \
	cp src/parser/grammar.y "$$tmp_dir/src/parser/grammar.y"; \
	( cd "$$tmp_dir" && bison --report=all -Wcounterexamples -d -o src/parser/parser.cpp src/parser/grammar.y ); \
	grep -E "conflicts:|shift/reduce conflict|reduce/reduce conflict" "$$tmp_dir/src/parser/parser.output" || true; \
	echo "parser-sync-check-bison: oraculo Bison regenerado en temporal"

lexer:
	$(CXX) $(CXXFLAGS) \
		src/lexer/main.cpp \
		src/lexer/lexer.cpp \
		-o hulk_lexer

parser-demo: $(LEXER_AST_OBJS) $(PARSER_OBJS)
	@mkdir -p $(OBJDIR)/parser_main
	$(CXX) $(CXXFLAGS) -c src/parser/main.cpp -o $(OBJDIR)/parser_main/main.o
	$(CXX) $(CXXFLAGS) \
		$(LEXER_AST_OBJS) $(PARSER_OBJS) \
		$(OBJDIR)/parser_main/main.o \
		-o hulk_parser_demo

# ─────────────────────────────────────────────────────────────────────────────
# Evaluador (cortes 4, 5 y 6)
# ─────────────────────────────────────────────────────────────────────────────
$(OBJDIR)/eval_main/main.o: src/eval/main.cpp
	@mkdir -p $(OBJDIR)/eval_main
	$(CXX) $(CXXFLAGS) -c src/eval/main.cpp -o $(OBJDIR)/eval_main/main.o

eval: $(LEXER_AST_OBJS) $(PARSER_OBJS) $(EVAL_OBJS) $(SEMANTIC_OBJS) $(OBJDIR)/eval_main/main.o
	$(CXX) $(CXXFLAGS) \
		$(LEXER_AST_OBJS) $(PARSER_OBJS) $(EVAL_OBJS) $(SEMANTIC_OBJS) \
		$(OBJDIR)/eval_main/main.o \
		-o hulk_eval

# ─────────────────────────────────────────────────────────────────────────────
# Tests
# ─────────────────────────────────────────────────────────────────────────────
parser-tests: parser-demo
	@for f in tests/parser/*.hulk; do \
		echo "===== $$f ====="; \
		./hulk_parser_demo $$f || true; \
		echo; \
	done

eval-tests: eval
	@echo "=== Corte 4: expresiones básicas ==="; \
	for f in tests/eval/c4_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_eval $$f 2>/dev/null || true; \
		echo; \
	done; \
	echo "=== Corte 5: variables y funciones ==="; \
	for f in tests/eval/c5_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_eval $$f 2>/dev/null || true; \
		echo; \
	done; \
	echo "=== Corte 6: objetos y herencia ==="; \
	for f in tests/eval/c6_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_eval $$f 2>/dev/null || true; \
		echo; \
	done

err-tests: eval
	@echo "=== Tests de error (deben fallar con mensaje diagnóstico) ==="; \
	for f in tests/eval/err_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_eval $$f 1>/dev/null; \
		echo; \
	done

semantic: $(LEXER_AST_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS)
	@mkdir -p $(OBJDIR)/semantic_main
	$(CXX) $(CXXFLAGS) -c src/semantic/main_semantic.cpp -o $(OBJDIR)/semantic_main/main.o
	$(CXX) $(CXXFLAGS) \
		$(LEXER_AST_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) \
		$(OBJDIR)/semantic_main/main.o \
		-o hulk_semantic

backend: $(LEXER_AST_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) $(BACKEND_OBJS)
	@mkdir -p $(OBJDIR)/backend_main
	$(CXX) $(CXXFLAGS) -c src/backend/main.cpp -o $(OBJDIR)/backend_main/main.o
	$(CXX) $(CXXFLAGS) \
		$(LEXER_AST_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) $(BACKEND_OBJS) \
		$(OBJDIR)/backend_main/main.o \
		-o hulk_backend

#   ./hulk <archivo.hulk>  → compila a un ejecutable ./output
#   ./output               → ejecuta el programa (IR embebido + VM)
build: $(LEXER_AST_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) $(BACKEND_OBJS)
	@mkdir -p $(OBJDIR)/backend_main
	$(CXX) $(CXXFLAGS) -c src/backend/main.cpp -o $(OBJDIR)/backend_main/main.o
	$(CXX) $(CXXFLAGS) \
		$(LEXER_AST_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) $(BACKEND_OBJS) \
		$(OBJDIR)/backend_main/main.o \
		-o hulk

vm-tests: $(OBJDIR)/vm/banner_vm.o $(OBJDIR)/vm/vm_heap.o $(OBJDIR)/vm/vm_value.o $(OBJDIR)/banner/banner_ir.o
	@mkdir -p $(OBJDIR)/vm_tests
	$(CXX) $(CXXFLAGS) -c tests/vm/vm_value_tests.cpp -o $(OBJDIR)/vm_tests/vm_value_tests.o
	$(CXX) $(CXXFLAGS) -c tests/vm/vm_heap_tests.cpp -o $(OBJDIR)/vm_tests/vm_heap_tests.o
	$(CXX) $(CXXFLAGS) -c tests/vm/banner_vm_limits_tests.cpp -o $(OBJDIR)/vm_tests/banner_vm_limits_tests.o
	$(CXX) $(CXXFLAGS) -c tests/vm/banner_vm_semantics_tests.cpp -o $(OBJDIR)/vm_tests/banner_vm_semantics_tests.o
	$(CXX) $(CXXFLAGS) \
		$(OBJDIR)/vm_tests/vm_value_tests.o \
		$(OBJDIR)/vm/vm_heap.o $(OBJDIR)/vm/vm_value.o \
		-o hulk_vm_value_tests
	$(CXX) $(CXXFLAGS) \
		$(OBJDIR)/vm_tests/vm_heap_tests.o \
		$(OBJDIR)/vm/vm_heap.o $(OBJDIR)/vm/vm_value.o \
		-o hulk_vm_tests
	$(CXX) $(CXXFLAGS) \
		$(OBJDIR)/vm_tests/banner_vm_limits_tests.o \
		$(OBJDIR)/vm/banner_vm.o $(OBJDIR)/vm/vm_heap.o $(OBJDIR)/vm/vm_value.o $(OBJDIR)/banner/banner_ir.o \
		-o hulk_vm_limits_tests
	$(CXX) $(CXXFLAGS) \
		$(OBJDIR)/vm_tests/banner_vm_semantics_tests.o \
		$(OBJDIR)/vm/banner_vm.o $(OBJDIR)/vm/vm_heap.o $(OBJDIR)/vm/vm_value.o $(OBJDIR)/banner/banner_ir.o \
		-o hulk_vm_semantics_tests
	./hulk_vm_value_tests
	./hulk_vm_tests
	./hulk_vm_limits_tests
	./hulk_vm_semantics_tests

backend-tests: backend
	@bash tests/backend/run_backend_tests.sh

end-to-end-tests: build
	@cd tests/end-to-end && HULK=../../hulk bash ./end-to-end_tests.sh $(FOLDER)

hulk-tests: build
	@bash tests/hulk/run_tests.sh

semantic-tests: semantic
	@echo "=== chequeos semánticos — programas válidos ==="; \
	for f in tests/semantic/ok_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_semantic $$f || true; \
		echo; \
	done; \
	echo "=== chequeos semánticos — programas con error ==="; \
	for f in tests/semantic/err_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_semantic $$f 1>/dev/null; \
		echo; \
	done; \
	echo "=== chequeos semánticos de tipo ==="; \
	for f in tests/typecheck/*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_semantic $$f 1>/dev/null; \
		echo; \
	done

extension-tests: semantic
	@echo "=== Tests de extensión — casos válidos ==="; \
	for f in tests/extension/valid_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_semantic $$f || true; \
		echo; \
	done; \
	echo "=== Tests de extensión — casos inválidos (deben fallar) ==="; \
	for f in tests/extension/invalid_*.hulk; do \
		echo "----- $$f -----"; \
		./hulk_semantic $$f 1>/dev/null; \
		echo; \
	done

# ─────────────────────────────────────────────────────────────────────────────
# Test runner unificado con expected y AST dumps
# ─────────────────────────────────────────────────────────────────────────────

# Ejecuta todos los tests (o una suite: eval | semantic | typecheck)
run-tests: eval parser-demo semantic
	@bash tests/run_tests.sh $(SUITE)

# Regenera todos los archivos .expected con la salida actual (usar tras cambios intencionales)
update-expected: eval backend semantic
	@echo "=== Actualizando archivos .expected ==="; \
	mkdir -p tests/expected/eval tests/expected/semantic tests/expected/typecheck tests/expected/backend; \
	for f in tests/eval/c4_*.hulk tests/eval/c5_*.hulk tests/eval/c6_*.hulk; do \
		name=$$(basename $$f .hulk); \
		{ ./hulk_eval $$f 2>&1; } > tests/expected/eval/$${name}.expected || true; \
		echo "  updated eval/$${name}.expected"; \
	done; \
	for f in tests/eval/err_*.hulk; do \
		name=$$(basename $$f .hulk); \
		{ ./hulk_eval $$f 2>&1; } > tests/expected/eval/$${name}.expected || true; \
		echo "  updated eval/$${name}.expected"; \
	done; \
	for f in tests/backend/regression/*.hulk; do \
		name=$$(basename $$f .hulk); \
		{ ./hulk_backend $$f 2>/dev/null && ./output 2>&1; } > tests/expected/backend/$${name}.expected || true; \
		rm -f ./output; \
		echo "  updated backend/$${name}.expected"; \
	done; \
	for f in tests/semantic/ok_*.hulk tests/semantic/err_*.hulk; do \
		name=$$(basename $$f .hulk); \
		{ ./hulk_semantic $$f 2>&1; } > tests/expected/semantic/$${name}.expected || true; \
		echo "  updated semantic/$${name}.expected"; \
	done; \
	for f in tests/typecheck/*.hulk; do \
		name=$$(basename $$f .hulk); \
		{ ./hulk_semantic $$f 2>&1; } > tests/expected/typecheck/$${name}.expected || true; \
		echo "  updated typecheck/$${name}.expected"; \
	done; \
	echo "=== Done ==="

clean:
	rm -rf $(OBJDIR) hulk hulk_lexer hulk_parser_demo hulk_eval hulk_semantic hulk_backend \
		hulk.exe hulk_lexer.exe hulk_parser_demo.exe hulk_eval.exe hulk_semantic.exe hulk_backend.exe \
		output output.exe \
		hulk_vm_value_tests hulk_vm_tests hulk_vm_limits_tests hulk_vm_semantics_tests \
		hulk_vm_value_tests.exe hulk_vm_tests.exe hulk_vm_limits_tests.exe hulk_vm_semantics_tests.exe
