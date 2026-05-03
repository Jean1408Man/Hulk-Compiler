#!/usr/bin/env bash
# run_tests.sh — ejecuta todos los tests, compara con .expected, vuelca AST
# Uso: bash tests/run_tests.sh [suite]   suite = eval | semantic | typecheck | all (default)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR/.."

EVAL_BIN="$ROOT/hulk_eval"
SEM_BIN="$ROOT/hulk_semantic"
PARSER_BIN="$ROOT/hulk_parser_demo"
AST_DIR="$SCRIPT_DIR/ast_dumps"
EXPECTED_DIR="$SCRIPT_DIR/expected"

SUITE="${1:-all}"

# ── contadores globales ──────────────────────────────────────────────────────
TOTAL=0; PASSED=0; FAILED=0

# ── colores ──────────────────────────────────────────────────────────────────
GREEN='\033[0;32m'; RED='\033[0;31m'; YELLOW='\033[1;33m'; RESET='\033[0m'
BOLD='\033[1m'

mkdir -p "$AST_DIR"

# ── dump_ast: parsea el archivo y guarda la estructura del AST ───────────────
dump_ast() {
    local hulk_file="$1"       # tests/eval/c4_arithmetic.hulk
    local suite="$2"           # eval | semantic | typecheck
    local name
    name="$(basename "$hulk_file" .hulk)"
    local out_file="$AST_DIR/${suite}_${name}.ast"

    if [[ -x "$PARSER_BIN" ]]; then
        "$PARSER_BIN" "$hulk_file" > "$out_file" 2>&1 || true
    else
        echo "(hulk_parser_demo no encontrado — AST no disponible)" > "$out_file"
    fi
}

# ── run_one: ejecuta un test y compara con su .expected ──────────────────────
run_one() {
    local bin="$1"
    local hulk_file="$2"
    local expected_file="$3"
    local suite="$4"

    local name
    name="$(basename "$hulk_file" .hulk)"
    TOTAL=$((TOTAL + 1))

    # captura stdout+stderr juntos (igual que el .expected)
    local actual
    actual="$("$bin" "$hulk_file" 2>&1 || true)"

    local expected
    expected="$(cat "$expected_file" 2>/dev/null || echo '')"

    # genera siempre el AST dump (sobreescribe si ya existe)
    dump_ast "$hulk_file" "$suite"

    if [[ "$actual" == "$expected" ]]; then
        echo -e "  ${GREEN}OK${RESET}  $name"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name"
        echo -e "       ${YELLOW}esperado:${RESET}"
        echo "$expected" | sed 's/^/         /'
        echo -e "       ${YELLOW}obtenido:${RESET}"
        echo "$actual"   | sed 's/^/         /'
        FAILED=$((FAILED + 1))
    fi
}

# ── suite_header ─────────────────────────────────────────────────────────────
suite_header() {
    echo ""
    echo -e "${BOLD}══════════════════════════════════════════${RESET}"
    echo -e "${BOLD}  $1${RESET}"
    echo -e "${BOLD}══════════════════════════════════════════${RESET}"
}

# ── EVAL ─────────────────────────────────────────────────────────────────────
run_eval() {
    suite_header "EVAL — Corte 4"
    for f in "$SCRIPT_DIR"/eval/c4_*.hulk; do
        [[ -f "$f" ]] || continue
        run_one "$EVAL_BIN" "$f" "$EXPECTED_DIR/eval/$(basename "$f" .hulk).expected" eval
    done

    suite_header "EVAL — Corte 5"
    for f in "$SCRIPT_DIR"/eval/c5_*.hulk; do
        [[ -f "$f" ]] || continue
        run_one "$EVAL_BIN" "$f" "$EXPECTED_DIR/eval/$(basename "$f" .hulk).expected" eval
    done

    suite_header "EVAL — Corte 6"
    for f in "$SCRIPT_DIR"/eval/c6_*.hulk; do
        [[ -f "$f" ]] || continue
        run_one "$EVAL_BIN" "$f" "$EXPECTED_DIR/eval/$(basename "$f" .hulk).expected" eval
    done

    suite_header "EVAL — Tests de error (deben reportar diagnóstico)"
    for f in "$SCRIPT_DIR"/eval/err_*.hulk; do
        [[ -f "$f" ]] || continue
        run_one "$EVAL_BIN" "$f" "$EXPECTED_DIR/eval/$(basename "$f" .hulk).expected" eval
    done
}

# ── SEMANTIC ─────────────────────────────────────────────────────────────────
run_semantic() {
    suite_header "SEMÁNTICO — Programas válidos"
    for f in "$SCRIPT_DIR"/semantic/ok_*.hulk; do
        [[ -f "$f" ]] || continue
        run_one "$SEM_BIN" "$f" "$EXPECTED_DIR/semantic/$(basename "$f" .hulk).expected" semantic
    done

    suite_header "SEMÁNTICO — Programas con error"
    for f in "$SCRIPT_DIR"/semantic/err_*.hulk; do
        [[ -f "$f" ]] || continue
        run_one "$SEM_BIN" "$f" "$EXPECTED_DIR/semantic/$(basename "$f" .hulk).expected" semantic
    done
}

# ── TYPECHECK ─────────────────────────────────────────────────────────────────
run_typecheck() {
    suite_header "TYPE CHECK"
    for f in "$SCRIPT_DIR"/typecheck/*.hulk; do
        [[ -f "$f" ]] || continue
        run_one "$SEM_BIN" "$f" "$EXPECTED_DIR/typecheck/$(basename "$f" .hulk).expected" typecheck
    done
}

# ── RESUMEN ──────────────────────────────────────────────────────────────────
print_summary() {
    echo ""
    echo -e "${BOLD}══════════════════════════════════════════${RESET}"
    echo -e "${BOLD}  RESUMEN${RESET}"
    echo -e "${BOLD}══════════════════════════════════════════${RESET}"
    echo -e "  Total  : ${BOLD}$TOTAL${RESET}"
    echo -e "  ${GREEN}Passed${RESET} : ${BOLD}$PASSED${RESET}"
    if [[ $FAILED -gt 0 ]]; then
        echo -e "  ${RED}Failed${RESET} : ${BOLD}$FAILED${RESET}"
    else
        echo -e "  Failed : ${BOLD}$FAILED${RESET}"
    fi
    echo ""
    echo -e "  AST dumps guardados en: ${YELLOW}tests/ast_dumps/${RESET}"
    echo ""
    if [[ $FAILED -eq 0 ]]; then
        echo -e "  ${GREEN}${BOLD}Todos los tests pasaron.${RESET}"
    else
        echo -e "  ${RED}${BOLD}$FAILED test(s) fallaron.${RESET}"
    fi
    echo ""
}

# ── DISPATCH ─────────────────────────────────────────────────────────────────
case "$SUITE" in
    eval)      run_eval ;;
    semantic)  run_semantic ;;
    typecheck) run_typecheck ;;
    all)       run_eval; run_semantic; run_typecheck ;;
    *)
        echo "Uso: bash tests/run_tests.sh [eval|semantic|typecheck|all]"
        exit 1
        ;;
esac

print_summary

# exit code refleja si hubo fallos
[[ $FAILED -eq 0 ]]
