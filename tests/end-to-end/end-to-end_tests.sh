#!/usr/bin/env bash
# ============================================================================
#  run_tests.sh — Suite de tests end-to-end para el compilador de HULK
# ============================================================================
#
#  Convención de archivos (en cases/<feature>/):
#
#    nombre.hulk          -> programa de entrada
#    nombre.out           -> (caso VÁLIDO) stdout exacto esperado del programa
#    nombre.err           -> (caso INVÁLIDO) el compilador DEBE fallar.
#                            Si el archivo .err tiene contenido, además se
#                            exige que la salida (stdout+stderr) contenga ese
#                            texto como substring (una línea = un substring
#                            que debe aparecer). Si está vacío, basta con que
#                            el compilador retorne código de salida != 0.
#
#  Un .hulk con .out  => se espera EXIT 0 y stdout idéntico.
#  Un .hulk con .err  => se espera EXIT != 0 (y substrings, si los hay).
#  Un .hulk sin ninguno => error de configuración del test.
#
#  Uso:
#    ./run_tests.sh                 # corre todo
#    ./run_tests.sh 08_inference    # corre solo una carpeta
#    HULK=./build/hulk ./run_tests.sh   # usa otro binario
#
# ============================================================================

set -u

# --- Configuración ----------------------------------------------------------
HULK="${HULK:-./hulk}"              # binario del compilador (override con env)
CASES_DIR="${CASES_DIR:-cases}"
TIMEOUT_SECS="${TIMEOUT_SECS:-15}"

# Flag que activa el restricted mode de la extensión. Ajusta si tu CLI usa otro.
RESTRICTED_FLAG="${RESTRICTED_FLAG:---restricted-inference}"
# Carpeta cuyos casos se ejecutan en restricted mode:
RESTRICTED_DIR="09_restricted"

# --- Colores -----------------------------------------------------------------
if [ -t 1 ]; then
  RED=$'\e[31m'; GRN=$'\e[32m'; YEL=$'\e[33m'; BLU=$'\e[34m'; DIM=$'\e[2m'; RST=$'\e[0m'
else
  RED=""; GRN=""; YEL=""; BLU=""; DIM=""; RST=""
fi

PASS=0; FAIL=0; SKIP=0
FAILED_LIST=()

# --- Localizar el binario ----------------------------------------------------
if ! command -v "$HULK" >/dev/null 2>&1 && [ ! -x "$HULK" ]; then
  echo "${YEL}AVISO:${RST} no se encontró el compilador en '$HULK'."
  echo "       Define la variable HULK, p. ej.:  HULK=./build/hulk ./run_tests.sh"
  echo "       (Se continúa para mostrar qué casos se ejecutarían.)"
  echo
  DRYRUN=1
else
  DRYRUN=0
fi

# --- Helper: ejecuta el compilador con timeout, captura salida y exit ---------
run_hulk() {
  # $1 = archivo .hulk ; $2... = flags extra
  local file="$1"; shift
  if command -v timeout >/dev/null 2>&1; then
    timeout "$TIMEOUT_SECS" "$HULK" "$@" "$file" 2>&1
  else
    "$HULK" "$@" "$file" 2>&1
  fi
}

# --- Ejecuta un único caso ---------------------------------------------------
run_case() {
  local hulk_file="$1"
  local base="${hulk_file%.hulk}"
  local name; name="$(basename "$base")"
  local dir; dir="$(basename "$(dirname "$hulk_file")")"

  # ¿restricted mode?
  local extra_flags=()
  if [ "$dir" = "$RESTRICTED_DIR" ]; then
    extra_flags+=("$RESTRICTED_FLAG")
  fi

  # Determinar tipo de caso
  local mode=""
  [ -f "${base}.out" ] && mode="out"
  [ -f "${base}.err" ] && mode="err"

  if [ -z "$mode" ]; then
    printf "  ${YEL}SKIP${RST} %-34s ${DIM}(sin .out ni .err)${RST}\n" "$name"
    SKIP=$((SKIP+1)); return
  fi

  if [ "$DRYRUN" = "1" ]; then
    printf "  ${BLU}WOULD${RST} %-33s ${DIM}[%s%s]${RST}\n" "$name" "$mode" \
      "$( [ "${#extra_flags[@]}" -gt 0 ] && echo " ${extra_flags[*]}" )"
    return
  fi

  local output exit_code
  output="$(run_hulk "$hulk_file" "${extra_flags[@]+"${extra_flags[@]}"}")"
  exit_code=$?

  if [ "$mode" = "out" ]; then
    # Caso válido: exit 0 + stdout idéntico
    local expected; expected="$(cat "${base}.out")"
    if [ "$exit_code" -ne 0 ]; then
      printf "  ${RED}FAIL${RST} %-34s ${DIM}(exit=%s, se esperaba 0)${RST}\n" "$name" "$exit_code"
      FAIL=$((FAIL+1)); FAILED_LIST+=("$dir/$name")
      echo "${DIM}        salida: $(echo "$output" | head -3 | tr '\n' '|')${RST}"
    elif [ "$output" = "$expected" ]; then
      printf "  ${GRN}PASS${RST} %-34s ${DIM}(valido)${RST}\n" "$name"
      PASS=$((PASS+1))
    else
      printf "  ${RED}FAIL${RST} %-34s ${DIM}(stdout no coincide)${RST}\n" "$name"
      FAIL=$((FAIL+1)); FAILED_LIST+=("$dir/$name")
      diff <(printf '%s' "$expected") <(printf '%s' "$output") \
        | sed 's/^/        /' | head -8
    fi
  else
    # Caso inválido: exit != 0, y substrings opcionales
    if [ "$exit_code" -eq 0 ]; then
      printf "  ${RED}FAIL${RST} %-34s ${DIM}(compiló; se esperaba error)${RST}\n" "$name"
      FAIL=$((FAIL+1)); FAILED_LIST+=("$dir/$name")
      return
    fi
    # Verificar substrings esperados (si los hay)
    local missing=0
    if [ -s "${base}.err" ]; then
      while IFS= read -r needle; do
        needle="${needle%$'\r'}"   # strip trailing CR (Windows CRLF .err files)
        [ -z "$needle" ] && continue
        if ! grep -qiF -- "$needle" <<<"$output"; then
          missing=1
          echo "${DIM}        falta en el error: \"$needle\"${RST}"
        fi
      done < "${base}.err"
    fi
    if [ "$missing" -eq 0 ]; then
      printf "  ${GRN}PASS${RST} %-34s ${DIM}(error esperado)${RST}\n" "$name"
      PASS=$((PASS+1))
    else
      printf "  ${RED}FAIL${RST} %-34s ${DIM}(error, pero mensaje no coincide)${RST}\n" "$name"
      FAIL=$((FAIL+1)); FAILED_LIST+=("$dir/$name")
    fi
  fi
}

# --- Main --------------------------------------------------------------------
echo "${BLU}========================================================${RST}"
echo "${BLU} HULK — Suite de tests end-to-end${RST}"
echo "${BLU}========================================================${RST}"
echo " Compilador : $HULK $( [ "$DRYRUN" = 1 ] && echo "${YEL}(no encontrado: dry-run)${RST}")"
echo " Casos      : $CASES_DIR"
echo

FILTER="${1:-}"
for feat_dir in "$CASES_DIR"/*/; do
  feat="$(basename "$feat_dir")"
  if [ -n "$FILTER" ] && [ "$feat" != "$FILTER" ]; then continue; fi
  # ¿hay casos?
  shopt -s nullglob
  files=("$feat_dir"*.hulk)
  shopt -u nullglob
  [ "${#files[@]}" -eq 0 ] && continue

  echo "${YEL}▶ $feat${RST}"
  for f in "${files[@]}"; do
    run_case "$f"
  done
  echo
done

# --- Resumen -----------------------------------------------------------------
echo "${BLU}--------------------------------------------------------${RST}"
if [ "$DRYRUN" = "1" ]; then
  echo " ${YEL}Dry-run: define HULK=<ruta-al-binario> para ejecutar.${RST}"
else
  TOTAL=$((PASS+FAIL))
  echo " Resultado: ${GRN}${PASS} PASS${RST} / ${RED}${FAIL} FAIL${RST} / ${YEL}${SKIP} SKIP${RST}  (de $TOTAL)"
  if [ "$FAIL" -gt 0 ]; then
    echo " ${RED}Fallaron:${RST}"
    for t in "${FAILED_LIST[@]}"; do echo "   - $t"; done
  fi
fi
echo "${BLU}--------------------------------------------------------${RST}"

[ "${FAIL:-0}" -eq 0 ]
