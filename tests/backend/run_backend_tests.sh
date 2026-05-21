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
    local name
    name="$(basename "$hulk_file" .hulk)"
    local ir_file="$TMP_DIR/$name.invalid.hir"
    local banner_file="$TMP_DIR/$name.invalid.banner"
    local actual_file="$TMP_DIR/$name.invalid"

    TOTAL=$((TOTAL + 1))

    if "$BACKEND_BIN" "$hulk_file" > "$actual_file.default.out" 2>&1; then
        echo -e "  ${RED}FAIL${RESET} $name"
        echo "       backend executed an invalid semantic program"
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

suite_header "BACKEND SEMANTICOS INVALIDOS"
for f in "$ROOT"/tests/backend/invalid/*.hulk; do
    [[ -f "$f" ]] || continue
    run_invalid_semantic_one "$f"
done

suite_header "BACKEND IR"
run_emit_ir_one "$ROOT/tests/eval/c4_block_let_if.hulk"
run_emit_ir_one "$ROOT/tests/eval/c5_recursion.hulk"
run_emit_ir_one "$ROOT/tests/eval/c6_objects_basic.hulk"

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
