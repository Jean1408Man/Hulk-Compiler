#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR/../.."
BACKEND_BIN="$ROOT/hulk_backend"
EXPECTED_DIR="$ROOT/tests/expected/eval"
BACKEND_EXPECTED_DIR="$ROOT/tests/expected/backend"
BACKEND_IR_EXPECTED_DIR="$ROOT/tests/expected/backend_ir"

TOTAL=0
PASSED=0
FAILED=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
RESET='\033[0m'
BOLD='\033[1m'

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

compare_files() {
    local expected_file="$1"
    local actual_file="$2"
    local diff_file="$3"
    local expected_norm="$diff_file.expected.norm"
    local actual_norm="$diff_file.actual.norm"

    sed 's/\r$//' "$expected_file" > "$expected_norm"
    sed 's/\r$//' "$actual_file" > "$actual_norm"
    diff -u "$expected_norm" "$actual_norm" > "$diff_file"
}

run_one() {
    local hulk_file="$1"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local actual_file="$TMP_DIR/$name.actual"
    local expected_file="$EXPECTED_DIR/$name.expected"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" > "$actual_file" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo -e "       ${YELLOW}backend:${RESET}"
        sed 's/^/         /' "$actual_file"
        FAILED=$((FAILED + 1))
        return
    fi

    if compare_files "$expected_file" "$actual_file" "$TMP_DIR/$name.diff"; then
        echo -e "  ${GREEN}OK${RESET}  $name"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name"
        sed 's/^/       /' "$TMP_DIR/$name.diff"
        FAILED=$((FAILED + 1))
    fi
}

run_expected_one() {
    local hulk_file="$1"
    local expected_file="$2"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local actual_file="$TMP_DIR/$name.actual"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" > "$actual_file" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo -e "       ${YELLOW}backend:${RESET}"
        sed 's/^/         /' "$actual_file"
        FAILED=$((FAILED + 1))
        return
    fi

    if compare_files "$expected_file" "$actual_file" "$TMP_DIR/$name.diff"; then
        echo -e "  ${GREEN}OK${RESET}  $name"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name"
        sed 's/^/       /' "$TMP_DIR/$name.diff"
        FAILED=$((FAILED + 1))
    fi
}

run_banner_one() {
    local hulk_file="$1"
    local mode="$2"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local actual_file="$TMP_DIR/$name.banner.$mode.actual"
    local expected_file="$EXPECTED_DIR/$name.expected"

    TOTAL=$((TOTAL + 1))

    if [[ "$mode" == "default" ]]; then
        "$BACKEND_BIN" "$hulk_file" > "$actual_file" 2>&1 || true
    else
        "$BACKEND_BIN" "$hulk_file" --run-banner > "$actual_file" 2>&1 || true
    fi

    if compare_files "$expected_file" "$actual_file" "$TMP_DIR/$name.banner.$mode.diff"; then
        echo -e "  ${GREEN}OK${RESET}  $name Banner $mode"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name Banner $mode"
        sed 's/^/       /' "$TMP_DIR/$name.banner.$mode.diff"
        FAILED=$((FAILED + 1))
    fi
}

run_invalid_one() {
    local hulk_file="$1"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local exe="$TMP_DIR/$name"

    TOTAL=$((TOTAL + 1))

    if "$BACKEND_BIN" "$hulk_file" -o "$exe" > "$TMP_DIR/$name.invalid.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend accepted an invalid extension program"
        FAILED=$((FAILED + 1))
    else
        echo -e "  ${GREEN}OK${RESET}  $name"
        PASSED=$((PASSED + 1))
    fi
}

run_invalid_semantic_one() {
    local hulk_file="$1"
    local expected_diagnostic="${2:-}"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local ir_file="$TMP_DIR/$name.invalid.hir"
    local banner_file="$TMP_DIR/$name.invalid.banner"
    local compiled_banner_file="$TMP_DIR/$name.invalid.compiled.banner"
    local actual_file="$TMP_DIR/$name.invalid"

    TOTAL=$((TOTAL + 1))

    if "$BACKEND_BIN" "$hulk_file" > "$actual_file.default.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend executed an invalid semantic program"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -n "$expected_diagnostic" ]] &&
       ! grep -q "$expected_diagnostic" "$actual_file.default.out"; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       expected semantic diagnostic was not reported"
        sed 's/^/         /' "$actual_file.default.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-ir -o "$ir_file" > "$actual_file.emit-ir.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted IR for an invalid semantic program"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$ir_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       IR file was created despite semantic errors"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-banner -o "$banner_file" > "$actual_file.emit-banner.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted BannerIR for an invalid semantic program"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       BannerIR file was created despite semantic errors"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-banner-compiled -o "$compiled_banner_file" > "$actual_file.emit-banner-compiled.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted compiled BannerIR for an invalid semantic program"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$compiled_banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       compiled BannerIR file was created despite semantic errors"
        FAILED=$((FAILED + 1))
        return
    fi

    echo -e "  ${GREEN}OK${RESET}  $name"
    PASSED=$((PASSED + 1))
}

run_restricted_valid_one() {
    local hulk_file="$1"
    local expected_file="$2"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local actual_file="$TMP_DIR/$name.restricted.actual"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" --restricted-inference > "$actual_file" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo -e "       ${YELLOW}backend restricted:${RESET}"
        sed 's/^/         /' "$actual_file"
        FAILED=$((FAILED + 1))
        return
    fi

    if compare_files "$expected_file" "$actual_file" "$TMP_DIR/$name.restricted.diff"; then
        echo -e "  ${GREEN}OK${RESET}  $name"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name"
        sed 's/^/       /' "$TMP_DIR/$name.restricted.diff"
        FAILED=$((FAILED + 1))
    fi
}

run_restricted_invalid_one() {
    local hulk_file="$1"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local ir_file="$TMP_DIR/$name.restricted.hir"
    local banner_file="$TMP_DIR/$name.restricted.banner"
    local compiled_banner_file="$TMP_DIR/$name.restricted.compiled.banner"
    local actual_file="$TMP_DIR/$name.restricted"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" > "$actual_file.normal.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend rejected implicit inference in normal mode"
        sed 's/^/         /' "$actual_file.normal.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --restricted-inference > "$actual_file.default.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend accepted implicit inference in restricted mode"
        FAILED=$((FAILED + 1))
        return
    fi

    if ! grep -q "Inferencia implicita no permitida" "$actual_file.default.out"; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       restricted inference diagnostic was not reported"
        sed 's/^/         /' "$actual_file.default.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --restricted-inference --emit-ir -o "$ir_file" > "$actual_file.emit-ir.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted IR with implicit inference in restricted mode"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$ir_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       IR file was created despite restricted inference errors"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --restricted-inference --emit-banner -o "$banner_file" > "$actual_file.emit-banner.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted BannerIR with implicit inference in restricted mode"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       BannerIR file was created despite restricted inference errors"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --restricted-inference --emit-banner-compiled -o "$compiled_banner_file" > "$actual_file.emit-banner-compiled.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted compiled BannerIR with implicit inference in restricted mode"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$compiled_banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       compiled BannerIR file was created despite restricted inference errors"
        FAILED=$((FAILED + 1))
        return
    fi

    echo -e "  ${GREEN}OK${RESET}  $name"
    PASSED=$((PASSED + 1))
}

run_unsupported_feature_one() {
    local hulk_file="$1"
    local feature="$2"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local ir_file="$TMP_DIR/$name.unsupported.hir"
    local banner_file="$TMP_DIR/$name.unsupported.banner"
    local compiled_banner_file="$TMP_DIR/$name.unsupported.compiled.banner"
    local actual_file="$TMP_DIR/$name.unsupported"

    TOTAL=$((TOTAL + 1))

    if "$BACKEND_BIN" "$hulk_file" > "$actual_file.default.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend accepted unsupported feature '$feature'"
        FAILED=$((FAILED + 1))
        return
    fi

    if ! grep -q "Feature no soportado en el flujo end-to-end: $feature" "$actual_file.default.out"; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       unsupported feature diagnostic was not reported for '$feature'"
        sed 's/^/         /' "$actual_file.default.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-ir -o "$ir_file" > "$actual_file.emit-ir.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted IR for unsupported feature '$feature'"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$ir_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       IR file was created despite unsupported feature '$feature'"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-banner -o "$banner_file" > "$actual_file.emit-banner.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted BannerIR for unsupported feature '$feature'"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       BannerIR file was created despite unsupported feature '$feature'"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-banner-compiled -o "$compiled_banner_file" > "$actual_file.emit-banner-compiled.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted compiled BannerIR for unsupported feature '$feature'"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$compiled_banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       compiled BannerIR file was created despite unsupported feature '$feature'"
        FAILED=$((FAILED + 1))
        return
    fi

    echo -e "  ${GREEN}OK${RESET}  $name"
    PASSED=$((PASSED + 1))
}

run_invalid_frontend_one() {
    local hulk_file="$1"
    local expected_diagnostic="$2"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local ir_file="$TMP_DIR/$name.frontend-invalid.hir"
    local banner_file="$TMP_DIR/$name.frontend-invalid.banner"
    local compiled_banner_file="$TMP_DIR/$name.frontend-invalid.compiled.banner"
    local actual_file="$TMP_DIR/$name.frontend-invalid"

    TOTAL=$((TOTAL + 1))

    if "$BACKEND_BIN" "$hulk_file" > "$actual_file.default.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend accepted an invalid frontend program"
        FAILED=$((FAILED + 1))
        return
    fi

    if ! grep -q "$expected_diagnostic" "$actual_file.default.out"; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       expected frontend diagnostic was not reported"
        sed 's/^/         /' "$actual_file.default.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-ir -o "$ir_file" > "$actual_file.emit-ir.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted IR for an invalid frontend program"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$ir_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       IR file was created despite frontend errors"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-banner -o "$banner_file" > "$actual_file.emit-banner.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted BannerIR for an invalid frontend program"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       BannerIR file was created despite frontend errors"
        FAILED=$((FAILED + 1))
        return
    fi

    if "$BACKEND_BIN" "$hulk_file" --emit-banner-compiled -o "$compiled_banner_file" > "$actual_file.emit-banner-compiled.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend emitted compiled BannerIR for an invalid frontend program"
        FAILED=$((FAILED + 1))
        return
    fi

    if [[ -e "$compiled_banner_file" ]]; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       compiled BannerIR file was created despite frontend errors"
        FAILED=$((FAILED + 1))
        return
    fi

    echo -e "  ${GREEN}OK${RESET}  $name"
    PASSED=$((PASSED + 1))
}

run_emit_ir_one() {
    local hulk_file="$1"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local ir_file="$TMP_DIR/$name.hir"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" --emit-ir -o "$ir_file" > "$TMP_DIR/$name.emit-ir.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name --emit-ir"
        sed 's/^/         /' "$TMP_DIR/$name.emit-ir.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if grep -q '^\.TYPES$' "$ir_file" &&
       grep -q '^\.DATA$' "$ir_file" &&
       grep -q '^\.CODE$' "$ir_file"; then
        echo -e "  ${GREEN}OK${RESET}  $name --emit-ir"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name --emit-ir"
        echo "       IR output missing .TYPES/.DATA/.CODE sections"
        FAILED=$((FAILED + 1))
    fi
}

run_emit_banner_one() {
    local hulk_file="$1"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local banner_file="$TMP_DIR/$name.banner"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" --emit-banner -o "$banner_file" > "$TMP_DIR/$name.emit-banner.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name --emit-banner"
        sed 's/^/         /' "$TMP_DIR/$name.emit-banner.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if grep -q '^\.TYPES$' "$banner_file" &&
       grep -q '^\.DATA$' "$banner_file" &&
       grep -q '^\.CODE$' "$banner_file"; then
        echo -e "  ${GREEN}OK${RESET}  $name --emit-banner"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name --emit-banner"
        echo "       Banner output missing .TYPES/.DATA/.CODE sections"
        FAILED=$((FAILED + 1))
    fi
}

run_emit_compiled_banner_one() {
    local hulk_file="$1"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local banner_file="$TMP_DIR/$name.compiled.banner"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" --emit-banner-compiled -o "$banner_file" > "$TMP_DIR/$name.emit-banner-compiled.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name --emit-banner-compiled"
        sed 's/^/         /' "$TMP_DIR/$name.emit-banner-compiled.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if grep -q '^\.COMPILED_BANNER$' "$banner_file" &&
       grep -q '^function #' "$banner_file" &&
       grep -q '^[[:space:]]*[0-9][0-9]*:' "$banner_file"; then
        echo -e "  ${GREEN}OK${RESET}  $name --emit-banner-compiled"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name --emit-banner-compiled"
        echo "       compiled Banner output missing header/functions/pc-indexed code"
        FAILED=$((FAILED + 1))
    fi
}

run_runtime_error_context_one() {
    local hulk_file="$1"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local actual_file="$TMP_DIR/$name.runtime-error.out"

    TOTAL=$((TOTAL + 1))

    if "$BACKEND_BIN" "$hulk_file" > "$actual_file" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name runtime error context"
        echo "       backend accepted a runtime-error program"
        FAILED=$((FAILED + 1))
        return
    fi

    if grep -q '^Runtime error en hulk_main pc=' "$actual_file" &&
       grep -q "source: $hulk_file:3:7" "$actual_file" &&
       grep -q 'instr: s2 = DIV s0, s1' "$actual_file" &&
       grep -q '^  stack:$' "$actual_file"; then
        echo -e "  ${GREEN}OK${RESET}  $name runtime error context"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name runtime error context"
        sed 's/^/       /' "$actual_file"
        FAILED=$((FAILED + 1))
    fi
}

run_runtime_error_contains_one() {
    local hulk_file="$1"
    local expected_diagnostic="$2"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local actual_file="$TMP_DIR/$name.runtime-error.out"

    TOTAL=$((TOTAL + 1))

    if "$BACKEND_BIN" "$hulk_file" > "$actual_file" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name runtime error"
        echo "       backend accepted a runtime-error program"
        FAILED=$((FAILED + 1))
        return
    fi

    if grep -q "$expected_diagnostic" "$actual_file"; then
        echo -e "  ${GREEN}OK${RESET}  $name runtime error"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name runtime error"
        echo "       expected runtime diagnostic was not reported"
        sed 's/^/       /' "$actual_file"
        FAILED=$((FAILED + 1))
    fi
}

run_ir_snapshot_one() {
    local hulk_file="$1"
    local expected_file="$2"
    local name
    name="$(basename "$hulk_file" .hulk)"
    local ir_file="$TMP_DIR/$name.snapshot.hir"

    TOTAL=$((TOTAL + 1))

    if ! "$BACKEND_BIN" "$hulk_file" --emit-ir -o "$ir_file" > "$TMP_DIR/$name.snapshot.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name IR snapshot"
        sed 's/^/         /' "$TMP_DIR/$name.snapshot.out"
        FAILED=$((FAILED + 1))
        return
    fi

    if compare_files "$expected_file" "$ir_file" "$TMP_DIR/$name.snapshot.diff"; then
        echo -e "  ${GREEN}OK${RESET}  $name IR snapshot"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${RESET} $name IR snapshot"
        sed 's/^/       /' "$TMP_DIR/$name.snapshot.diff"
        FAILED=$((FAILED + 1))
    fi
}

suite_header() {
    echo ""
    echo -e "${BOLD}$1${RESET}"
}

suite_header "BACKEND C4"
for f in "$ROOT"/tests/eval/c4_*.hulk; do
    [[ -f "$f" ]] || continue
    run_one "$f"
done

suite_header "BACKEND C5"
for f in "$ROOT"/tests/eval/c5_*.hulk; do
    [[ -f "$f" ]] || continue
    run_one "$f"
done

suite_header "BACKEND C6"
for f in "$ROOT"/tests/eval/c6_*.hulk; do
    [[ -f "$f" ]] || continue
    run_one "$f"
done

suite_header "BACKEND TYPE-HOLES VALIDOS"
for f in "$ROOT"/tests/extension/valid_*.hulk; do
    [[ -f "$f" ]] || continue
    run_expected_one "$f" "$BACKEND_EXPECTED_DIR/$(basename "$f" .hulk).expected"
done

suite_header "BACKEND RESTRICTED-INFERENCE VALIDOS"
for f in "$ROOT"/tests/extension/restricted_valid_*.hulk; do
    [[ -f "$f" ]] || continue
    run_restricted_valid_one "$f" "$BACKEND_EXPECTED_DIR/$(basename "$f" .hulk).expected"
done

suite_header "BACKEND REGRESIONES"
for f in "$ROOT"/tests/backend/regression/*.hulk; do
    [[ -f "$f" ]] || continue
    run_expected_one "$f" "$BACKEND_EXPECTED_DIR/$(basename "$f" .hulk).expected"
done

suite_header "BACKEND TYPE-HOLES INVALIDOS"
for name in \
    invalid_ambiguous_id \
    invalid_conflict \
    invalid_double_constraint \
    invalid_op_conflict \
    invalid_recursive_type \
    invalid_type_mismatch \
    invalid_unknown_method
do
    run_invalid_one "$ROOT/tests/extension/$name.hulk"
done

suite_header "BACKEND RESTRICTED-INFERENCE INVALIDOS"
for f in "$ROOT"/tests/extension/restricted_invalid_*.hulk; do
    [[ -f "$f" ]] || continue
    run_restricted_invalid_one "$f"
done

suite_header "BACKEND FEATURES NO SOPORTADOS"
run_unsupported_feature_one "$ROOT/tests/backend/unsupported/unsupported_lambda.hulk" "lambda"

suite_header "BACKEND FRONTEND INVALIDOS"
run_invalid_frontend_one "$ROOT/tests/backend/frontend_invalid/out_of_range_number.hulk" "Literal numerico fuera de rango"
run_invalid_frontend_one "$ROOT/tests/backend/frontend_invalid/invalid_string_escape.hulk" "Escape de string no soportado"
run_invalid_frontend_one "$ROOT/tests/backend/frontend_invalid/multiple_global_exprs.hulk" "Solo se permite una expresion global final"

suite_header "BACKEND SEMANTICOS INVALIDOS"
for f in "$ROOT"/tests/backend/invalid/*.hulk; do
    [[ -f "$f" ]] || continue
    run_invalid_semantic_one "$f"
done

suite_header "BACKEND CONCAT INVALIDOS"
for f in "$ROOT"/tests/backend/invalid_concat/*.hulk; do
    [[ -f "$f" ]] || continue
    run_invalid_semantic_one "$f" "Operador de concatenacion"
done

suite_header "BACKEND PRINT INVALIDOS"
for f in "$ROOT"/tests/backend/invalid_print/*.hulk; do
    [[ -f "$f" ]] || continue
    run_invalid_semantic_one "$f" "retorno de función 'f'"
done

suite_header "BACKEND IR"
run_emit_ir_one "$ROOT/tests/eval/c4_block_let_if.hulk"
run_emit_ir_one "$ROOT/tests/eval/c5_recursion.hulk"
run_emit_ir_one "$ROOT/tests/eval/c6_objects_basic.hulk"

suite_header "BACKEND COMPILED BANNER"
run_emit_compiled_banner_one "$ROOT/tests/eval/c4_block_let_if.hulk"
run_emit_compiled_banner_one "$ROOT/tests/eval/c5_recursion.hulk"
run_emit_compiled_banner_one "$ROOT/tests/eval/c6_objects_basic.hulk"

suite_header "BACKEND RUNTIME ERRORS"
run_runtime_error_context_one "$ROOT/tests/eval/err_div_zero.hulk"
run_runtime_error_contains_one "$ROOT/tests/backend/runtime_errors/math_sqrt_domain.hulk" "dominio invalido para sqrt"
run_runtime_error_contains_one "$ROOT/tests/backend/runtime_errors/math_log_domain.hulk" "dominio invalido para log"
run_runtime_error_contains_one "$ROOT/tests/backend/runtime_errors/math_exp_nonfinite.hulk" "resultado numerico no finito en exp"

suite_header "BACKEND BANNER C4"
for f in "$ROOT"/tests/eval/c4_*.hulk; do
    [[ -f "$f" ]] || continue
    run_emit_banner_one "$f"
    run_banner_one "$f" "default"
    run_banner_one "$f" "run"
done

suite_header "BACKEND BANNER C5"
for f in "$ROOT"/tests/eval/c5_*.hulk; do
    [[ -f "$f" ]] || continue
    run_emit_banner_one "$f"
    run_banner_one "$f" "default"
    run_banner_one "$f" "run"
done

suite_header "BACKEND BANNER C6"
for f in "$ROOT"/tests/eval/c6_*.hulk; do
    [[ -f "$f" ]] || continue
    run_emit_banner_one "$f"
    run_banner_one "$f" "default"
    run_banner_one "$f" "run"
done

suite_header "BACKEND IR SNAPSHOTS"
for f in "$ROOT"/tests/backend/ir_cases/*.hulk; do
    [[ -f "$f" ]] || continue
    run_ir_snapshot_one "$f" "$BACKEND_IR_EXPECTED_DIR/$(basename "$f" .hulk).hir"
done

echo ""
echo -e "${BOLD}BACKEND RESUMEN${RESET}"
echo "  Total  : $TOTAL"
echo -e "  ${GREEN}Passed${RESET} : $PASSED"
echo "  Failed : $FAILED"

[[ $FAILED -eq 0 ]]
