# Suite de tests end-to-end para HULK

Tests de caja negra que ejercitan el compilador completo (lexer → parser →
semántica/inferencia → ejecución) feature por feature, desde A.2 hasta A.9 de
la especificación de HULK, más el `restricted mode` de la extensión de
inferencia explícita (`_` / `auto`).

## Uso

```bash
# El compilador debe invocarse como:  ./hulk archivo.hulk
./end-to-end_tests.sh                      # corre toda la suite
./end-to-end_tests.sh 08_inference         # corre solo una carpeta
HULK=./build/hulk ./end-to-end_tests.sh    # usa otro binario
```

Si no se encuentra el binario, el runner entra en *dry-run* y lista los casos
que ejecutaría (útil para revisar la suite sin compilador).

## Convención de archivos

Cada caso vive en `cases/<feature>/` y consta de un `.hulk` más UNA expectativa:

| Archivo        | Significado                                                        |
|----------------|--------------------------------------------------------------------|
| `nombre.hulk`  | Programa de entrada.                                               |
| `nombre.out`   | Caso **válido**: se espera `exit 0` y este stdout **exacto**.      |
| `nombre.err`   | Caso **inválido**: se espera `exit != 0`. Si el archivo tiene texto, cada línea es un *substring* que **debe** aparecer en el mensaje de error (comparación case-insensitive). Si está vacío, basta con que falle. |

Los casos de `09_restricted/` se ejecutan automáticamente con la bandera
`--type-inference=restricted` (configurable en el runner mediante la variable
`RESTRICTED_FLAG`).

## Cobertura

| Carpeta                | Sección | Qué cubre                                                       |
|------------------------|---------|-----------------------------------------------------------------|
| `01_arithmetic`        | A.2.1   | Operadores, precedencia, errores de sintaxis y de tipo.         |
| `02_strings_builtins`  | A.2.2-4 | Strings, escapes, `@`/`@@`, builtins math, bloques.             |
| `03_variables`         | A.4     | `let`, multi-binding, scoping, shadowing, `:=`, identificadores.|
| `04_control_flow`      | A.5-A.6 | `if/elif/else`, booleanos, `while`, `for`, `range`.             |
| `05_functions`         | A.3     | Inline, full-form, recursión, forward reference, aridad.        |
| `06_objects`           | A.7     | Tipos, atributos, herencia, `base`, dispatch virtual.           |
| `07_type_check`        | A.8     | Anotaciones, conformidad, `is`, `as`, mismatches.               |
| `08_inference`         | A.9     | Inferencia de expresiones y símbolos; holes en modo normal; fallos por ambigüedad/contradicción. |
| `09_restricted`        | ext.    | Restricted mode: holes obligatorios, rechazo de omisión implícita. |

## Ajustes que quizá necesites

Las salidas esperadas (`.out`) asumen una semántica concreta. Revisa y ajusta
si tu implementación difiere en:

- **Formato numérico**: los `.out` usan `5`, `2.5`, `21.6`, `1024` (enteros sin
  `.0`). Si tu `print` emite `5.0`, regenera los `.out` afectados.
- **Substrings de error en español**: los `.err` con texto buscan palabras como
  `no se pudo inferir`, `incompatibles`, `restricted`. Si tus mensajes están en
  inglés u otra redacción, edita esos `.err` (o déjalos vacíos para sólo exigir
  que el caso falle).
- **`private_attribute`**: asume que los atributos son privados (acceso externo
  = error), según A.7.
