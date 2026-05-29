#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
EVAL="$ROOT/hulk_eval"
PASS=0
FAIL=0

if [ ! -x "$EVAL" ]; then
    echo "Error: no se encontró el binario $EVAL"
    echo "Ejecuta 'make eval' primero."
    exit 1
fi

echo "=== eval restricted-inference — casos válidos (deben pasar) ==="
for f in "$ROOT"/tests/extension/restricted_valid_*.hulk; do
    name=$(basename "$f" .hulk)
    printf "  %-48s" "$name"
    if "$EVAL" --restricted-inference "$f" > /dev/null 2>&1; then
        echo "PASS"
        PASS=$((PASS+1))
    else
        echo "FAIL  (rechazó programa válido)"
        FAIL=$((FAIL+1))
    fi
done

echo ""
echo "=== eval restricted-inference — casos inválidos (deben ser rechazados) ==="
for f in "$ROOT"/tests/extension/restricted_invalid_*.hulk; do
    name=$(basename "$f" .hulk)
    printf "  %-48s" "$name"
    if "$EVAL" --restricted-inference "$f" > /dev/null 2>&1; then
        echo "FAIL  (aceptó programa inválido)"
        FAIL=$((FAIL+1))
    else
        echo "PASS  (rechazado correctamente)"
        PASS=$((PASS+1))
    fi
done

echo ""
echo "=== Resultado: $PASS PASS, $FAIL FAIL ==="
[ "$FAIL" -eq 0 ]
