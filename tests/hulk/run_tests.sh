set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# ── Rutas ─────────────────────────────────────────────────────────────────────

PROJECT_ROOT="${1:-$(cd "$SCRIPT_DIR/../.." && pwd)}"
TESTS_DIR="${2:-$SCRIPT_DIR}"
TIMEOUT_SECS="${TIMEOUT_SECS:-15}"

# Detectar binario: argumento/env > ./hulk > ./hulk_backend
if [ -n "${HULK:-}" ]; then
    HULK_BIN="$HULK"
elif [ -f "$PROJECT_ROOT/hulk" ]; then
    HULK_BIN="$PROJECT_ROOT/hulk"
elif [ -f "$PROJECT_ROOT/hulk_backend" ]; then
    HULK_BIN="$PROJECT_ROOT/hulk_backend"
else
    HULK_BIN="$PROJECT_ROOT/hulk"
fi

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

declare -A CAT_PASS CAT_FAIL
TOTAL_PASS=0
TOTAL_FAIL=0
FAILURES=""

log_pass() {
    local cat="$1" name="$2"
    printf "${GREEN}PASS${NC} %s/%s\n" "$cat" "$name"
    CAT_PASS[$cat]=$(( ${CAT_PASS[$cat]:-0} + 1 ))
    TOTAL_PASS=$(( TOTAL_PASS + 1 ))
}

log_fail() {
    local cat="$1" name="$2" reason="$3"
    printf "${RED}FAIL${NC} %s/%s: %s\n" "$cat" "$name" "$reason"
    CAT_FAIL[$cat]=$(( ${CAT_FAIL[$cat]:-0} + 1 ))
    TOTAL_FAIL=$(( TOTAL_FAIL + 1 ))
    FAILURES="${FAILURES}"$'\n'"- \`${cat}/${name}\`: ${reason}"
}

run_with_timeout() {
    if command -v timeout >/dev/null 2>&1; then
        timeout "$TIMEOUT_SECS" "$@"
    else
        "$@"
    fi
}

run_ok_test() {
    local hulk_file="$1" expected_file="$2" cat="$3" name="$4"
    local tmperr; tmperr=$(mktemp)

    # Limpiar cualquier ./output anterior para evitar falsos positivos
    rm -f "$PROJECT_ROOT/output"

    # Compilar: ./hulk <archivo> → produce ./output en PROJECT_ROOT
    local compile_exit=0
    (cd "$PROJECT_ROOT" && run_with_timeout "$HULK_BIN" "$hulk_file" > /dev/null 2> "$tmperr") || compile_exit=$?

    if [ "$compile_exit" -ne 0 ]; then
        local err_preview; err_preview=$(head -1 "$tmperr" | cut -c1-80)
        rm -f "$tmperr" "$PROJECT_ROOT/output"
        log_fail "$cat" "$name" "compilation failed (exit $compile_exit): $err_preview"
        return
    fi
    rm -f "$tmperr"

    if [ ! -x "$PROJECT_ROOT/output" ]; then
        log_fail "$cat" "$name" "./output no existe o no es ejecutable tras compilar"
        return
    fi

    # Ejecutar el binario generado
    local actual runtime_exit=0
    actual=$(cd "$PROJECT_ROOT" && run_with_timeout ./output 2>/dev/null) || runtime_exit=$?

    rm -f "$PROJECT_ROOT/output"

    if [ "$runtime_exit" -ne 0 ]; then
        log_fail "$cat" "$name" "./output crashed (exit $runtime_exit)"
        return
    fi

    # Comparar salida (ignorar espacios al final de línea)
    local expected; expected=$(cat "$expected_file")
    local actual_norm; actual_norm=$(echo "$actual" | sed 's/[[:space:]]*$//')
    local expected_norm; expected_norm=$(echo "$expected" | sed 's/[[:space:]]*$//')

    if [ "$actual_norm" = "$expected_norm" ]; then
        log_pass "$cat" "$name"
    else
        local got; got=$(echo "$actual" | head -3 | tr '\n' '|')
        local exp; exp=$(echo "$expected" | head -3 | tr '\n' '|')
        log_fail "$cat" "$name" "expected [${exp}] got [${got}]"
    fi
}

run_error_test() {
    local hulk_file="$1" exit_file="$2" cat="$3" name="$4"
    local expected_exit; expected_exit=$(tr -d '[:space:]' < "$exit_file")

    local expected_type
    case "$cat" in
        errors/lexical)   expected_type="LEXICAL" ;;
        errors/syntactic) expected_type="SYNTACTIC" ;;
        errors/semantic)  expected_type="SEMANTIC" ;;
        *) expected_type="" ;;
    esac

    local tmperr; tmperr=$(mktemp)
    local actual_exit=0
    (cd "$PROJECT_ROOT" && run_with_timeout "$HULK_BIN" "$hulk_file" > /dev/null 2> "$tmperr") || actual_exit=$?
    local stderr_out; stderr_out=$(cat "$tmperr"); rm -f "$tmperr"

    if [ "$actual_exit" != "$expected_exit" ]; then
        log_fail "$cat" "$name" "expected exit $expected_exit, got $actual_exit"
        return
    fi
    if [ -n "$expected_type" ] && ! echo "$stderr_out" | grep -q "$expected_type"; then
        log_fail "$cat" "$name" "exit $actual_exit OK but $expected_type missing from stderr"
        return
    fi
    log_pass "$cat" "$name"
}

# ── Pre-flight ────────────────────────────────────────────────────────────────

if [ ! -f "$HULK_BIN" ]; then
    echo "ERROR: binario no encontrado en '$HULK_BIN'." >&2
    echo "       Ejecuta 'make build' o 'make backend' primero." >&2
    exit 2
fi
chmod +x "$HULK_BIN"

echo "Binario:    $HULK_BIN"
echo "Tests dir:  $TESTS_DIR"
echo "Timeout:    ${TIMEOUT_SECS}s"
echo ""

# ── Run tests ─────────────────────────────────────────────────────────────────

for hulk_file in "$TESTS_DIR/ok/minimal/"*.hulk; do
    [ -f "$hulk_file" ] || continue
    name=$(basename "$hulk_file" .hulk)
    run_ok_test "$hulk_file" "$TESTS_DIR/ok/minimal/$name.expected" "ok/minimal" "$name"
done

for hulk_file in "$TESTS_DIR/ok/types/"*.hulk; do
    [ -f "$hulk_file" ] || continue
    name=$(basename "$hulk_file" .hulk)
    run_ok_test "$hulk_file" "$TESTS_DIR/ok/types/$name.expected" "ok/types" "$name"
done

for hulk_file in "$TESTS_DIR/ok/oop/"*.hulk; do
    [ -f "$hulk_file" ] || continue
    name=$(basename "$hulk_file" .hulk)
    run_ok_test "$hulk_file" "$TESTS_DIR/ok/oop/$name.expected" "ok/oop" "$name"
done

for hulk_file in "$TESTS_DIR/ok/extras/"*.hulk; do
    [ -f "$hulk_file" ] || continue
    name=$(basename "$hulk_file" .hulk)
    run_ok_test "$hulk_file" "$TESTS_DIR/ok/extras/$name.expected" "ok/extras" "$name"
done

for hulk_file in "$TESTS_DIR/errors/lexical/"*.hulk; do
    [ -f "$hulk_file" ] || continue
    name=$(basename "$hulk_file" .hulk)
    run_error_test "$hulk_file" "$TESTS_DIR/errors/lexical/$name.exit" "errors/lexical" "$name"
done

for hulk_file in "$TESTS_DIR/errors/syntactic/"*.hulk; do
    [ -f "$hulk_file" ] || continue
    name=$(basename "$hulk_file" .hulk)
    run_error_test "$hulk_file" "$TESTS_DIR/errors/syntactic/$name.exit" "errors/syntactic" "$name"
done

for hulk_file in "$TESTS_DIR/errors/semantic/"*.hulk; do
    [ -f "$hulk_file" ] || continue
    name=$(basename "$hulk_file" .hulk)
    run_error_test "$hulk_file" "$TESTS_DIR/errors/semantic/$name.exit" "errors/semantic" "$name"
done

# ── Summary ───────────────────────────────────────────────────────────────────

echo ""
echo "=============================="
echo "       GRADING SUMMARY"
echo "=============================="

REQUIRED=("ok/minimal" "ok/types" "ok/oop" "errors/lexical" "errors/syntactic" "errors/semantic")
ALL_GREEN=true

printf "%-28s %s\n" "Category" "Result"
printf "%-28s %s\n" "--------" "------"
for cat in "${REQUIRED[@]}" "ok/extras"; do
    p=${CAT_PASS[$cat]:-0}
    f=${CAT_FAIL[$cat]:-0}
    total=$(( p + f ))
    if [ "$cat" = "ok/extras" ]; then
        label="bonus"
    elif [ "$f" -eq 0 ] && [ "$total" -gt 0 ]; then
        label="PASS"
    else
        label="FAIL"
        ALL_GREEN=false
    fi
    printf "%-28s %d/%d [%s]\n" "$cat" "$p" "$total" "$label"
done

echo ""
if $ALL_GREEN; then
    echo "RESULT: ALL_PASS"
    exit 0
else
    echo "RESULT: FAIL"
    if [ -n "$FAILURES" ]; then
        echo "FAILURES:${FAILURES}"
    fi
    exit 1
fi
