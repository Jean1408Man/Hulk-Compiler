# Informe de vulnerabilidades resueltas en el flujo end-to-end

Fecha: 2026-05-24

Este informe resume el trabajo realizado durante el chat para cerrar las
vulnerabilidades 1 a 15 del documento
`doc/vulnerabilidades-flujo-end-to-end.md`. El foco fue endurecer el flujo:

```text
HULK fuente
  -> Lexer / Parser
  -> AST
  -> SemanticAnalyzer
  -> HulkIR
  -> BannerIR
  -> BannerVM
  -> salida
```

## Resumen ejecutivo

| Vulnerabilidad | Estado | Resultado principal |
| --- | --- | --- |
| 1. Tolerancias semanticas en BackendDriver | Resuelta | El backend ya no ignora errores semanticos por texto de mensaje. |
| 2. Acceso privado a atributos tolerado | Resuelta | Los accesos privados invalidos bloquean ejecucion y emision de IR/BannerIR. |
| 3. `is` no plausible tolerado | Resuelta | La politica estricta del type checker se respeta en backend. |
| 4. Falta GC en BannerVM | Resuelta | Se agrego mark-and-sweep exacto, no compactante, con free lists y generaciones. |
| 5. Crecimiento sin limite de stack/ejecucion/heap | Resuelta | Se agregaron limites configurables de frames, steps y heap vivo. |
| 6. BannerIR textual demasiado simbolica | Resuelta | Se agrego `--emit-banner-compiled` con vista indexada por slots/PC/IDs. |
| 7. Errores runtime sin contexto | Resuelta | Los errores runtime incluyen funcion, pc, source span, instruccion y stack trace. |
| 8. Tests sin invariantes internas de VM | Resuelta | Se ampliaron `vm-tests` con checks de `Word`, heap y BannerVM construido en C++. |
| 9. Falta `restricted-inference` | Resuelta | Se agrego modo opt-in que bloquea inferencia implicita y permite solo tipos concretos, `_` o `auto`. |
| 10. Features parciales en frontend/backend | Resuelta | `lambda`, `range()`/`Iterable` y `protocol` fallan temprano con diagnostico semantico estable; el caso bloqueado de `for` es `for` sobre `Iterable/range`. |
| 11. Literales numericos invalidos convertidos a `0` | Resuelta | Los numeros no representables reportan error sintactico y no se transforman en `Number(0)`. |
| 12. Escapes de strings aceptados pero no interpretados | Resuelta | Los literales decodifican `\n`, `\r`, `\t`, `\"` y `\\`; escapes desconocidos bloquean el frontend. |
| 13. `grammar.y` y parser generado desincronizados | Resuelta | `auto` y `_` quedan como pseudo-tipos via `IDENTIFIER`, el parser fue regenerado y se agrego `make parser-sync-check`. |
| 14. Multiples expresiones globales | Resuelta | Se formalizo como extension local: `decl* expr+` se ejecuta como bloque implicito y queda documentado/probado. |
| 15. Concatenacion `@` acepta valores fuera del contrato | Resuelta | `@`/`@@` ahora admiten solo `String`/`Number`, exigen al menos un `String` y la VM aplica la misma defensa. |

## 1. Tolerancias semanticas en BackendDriver

### Problema

El backend contenia excepciones por texto para ignorar diagnosticos
semanticos especificos. Eso permitia que un programa marcado como invalido por
semantica continuara hacia `IRGen`, `HulkIRToBanner` o `BannerVM`.

### Solucion aplicada

Se elimino la logica de tolerancia por string y se dejo una regla unica:

```cpp
if (!sem_ok || engine.has_blocking_errors()) {
    engine.print_all();
    return result;
}
```

Con esto, si semantica falla o si existen errores bloqueantes, el backend no
genera IR, no genera BannerIR y no ejecuta VM.

### Archivos relevantes

- `src/backend/backend_driver.cpp`
- `tests/backend/run_backend_tests.sh`
- `tests/backend/invalid/private_attribute_access.hulk`
- `tests/backend/invalid/non_plausible_is.hulk`

### Validacion

La suite de backend verifica que un programa semanticamente invalido:

- no se ejecuta en modo default;
- no emite `--emit-ir`;
- no emite `--emit-banner`;
- no emite `--emit-banner-compiled`;
- no deja archivos de salida creados accidentalmente.

## 2. Acceso privado a atributos tolerado

### Problema

El backend ignoraba el diagnostico:

```text
Los atributos son privados. Solo se pueden acceder mediante 'self'.
```

La VM no conoce la regla de privacidad del lenguaje. Para la VM, un atributo es
solo un slot en `fields[]`. Por eso la decision tenia que cerrarse antes de
llegar a BannerVM.

### Solucion aplicada

Al retirar la tolerancia semantica del `BackendDriver`, el acceso privado quedo
bloqueado antes de `IRGen`. Tambien se agrego/uso un caso negativo dedicado:

```text
tests/backend/invalid/private_attribute_access.hulk
```

### Resultado

La privacidad vuelve a ser una regla del lenguaje fuente y no una
responsabilidad de la VM. Los tests validos C6 siguen cubriendo acceso correcto
a estado de objetos por las rutas aceptadas por semantica.

## 3. `is` no plausible tolerado parcialmente

### Problema

El backend ignoraba diagnosticos cuyo texto contenia:

```text
'is' no plausible:
```

Esto dejaba al lenguaje en una politica mixta: el type checker reportaba error,
pero el backend lo ejecutaba como una comprobacion runtime que devolvia `false`.

### Solucion aplicada

Se mantuvo la politica estricta ya expresada por semantica/typecheck: si el
chequeo `is` no es plausible y semantica lo reporta como error bloqueante, el
backend no continua.

### Validacion

Se agrego/uso el caso:

```text
tests/backend/invalid/non_plausible_is.hulk
```

El runner de backend comprueba que tampoco se emitan salidas intermedias para
este programa invalido.

## 4. Falta GC: heap con vida de programa completa

### Problema

Antes, `VMHeap` funcionaba como una arena de vida completa: strings y objetos
se acumulaban hasta el final de `BannerVM::run`. Programas con concatenaciones
o asignaciones largas podian crecer sin liberar valores no alcanzables.

### Solucion aplicada

Se implemento un GC mark-and-sweep exacto, no compactante:

- `VMHeap` ahora usa slots con `occupied`, `marked`, `generation` y `value`.
- Los handles `Word` de strings y objetos codifican `index + generation`.
- Los slots liberados se reutilizan mediante free lists.
- `VMHeap::object()` y `VMHeap::string_value()` validan:
  - tipo del `Word`;
  - indice dentro de rango;
  - slot ocupado;
  - generacion del handle igual a la generacion del slot.
- `VMHeap::collect(roots)` marca strings/objetos alcanzables y libera lo no
  marcado.
- La VM construye raices desde:
  - `Frame::slots`;
  - `Frame::param_buffer`;
  - `CompiledProgram::data`.
- El GC se dispara despues de allocaciones runtime en:
  - `Concat`;
  - `ConcatSpace`;
  - `Allocate`.

### Archivos relevantes

- `src/vm/vm_heap.h`
- `src/vm/vm_heap.cpp`
- `src/vm/vm_value.h`
- `src/vm/vm_value.cpp`
- `src/vm/banner_vm.cpp`
- `tests/vm/vm_heap_tests.cpp`
- `Makefile`

### Validacion

Los tests de heap cubren:

- string sin raiz se libera;
- string con raiz sobrevive;
- objeto con raiz mantiene vivos sus campos;
- ciclo de objetos sin raiz se libera;
- slot liberado se reutiliza con nueva generacion;
- handle viejo a slot reutilizado falla con error controlado.

## 5. Sin proteccion contra crecimiento de stack VM

### Problema

La VM podia crecer indefinidamente por recursion, loops infinitos o heap vivo
sin limite externo. El resultado podia ser consumo de memoria del proceso host
en lugar de un error controlado.

### Solucion aplicada

Se agrego `VMOptions`:

```cpp
struct VMOptions {
    std::size_t max_frames = 100000;
    std::uint64_t max_steps = 10000000;
    std::size_t max_heap_values = 1000000;
};
```

`BannerVM::run` ahora acepta opciones:

```cpp
Word run(const Banner::BannerProgram& program, const VMOptions& options = {});
```

La VM valida:

- limite de instrucciones ejecutadas en cada ciclo de fetch/execute;
- limite de frames antes de `CALL`, `VCALL` y `SCALL`;
- limite de heap vivo despues de cargar `.DATA` y despues de allocaciones
  runtime.

### Archivos relevantes

- `src/vm/banner_vm.h`
- `src/vm/banner_vm.cpp`
- `tests/vm/banner_vm_limits_tests.cpp`

### Validacion

Los tests unitarios cubren:

- loop infinito detenido por `max_steps`;
- recursion detenida por `max_frames`;
- crecimiento de heap detenido por `max_heap_values`;
- programa pequeno aceptado con limites por defecto.

## 6. BannerIR textual conserva demasiados simbolos

### Problema

`--emit-banner` imprimia una vista legible pero simbolica. Esa vista es util
para humanos, pero no demostraba la forma baja que ejecuta la VM, donde locales,
labels, funciones, campos y metodos se convierten a indices.

### Solucion aplicada

Se mantuvo `--emit-banner` como vista simbolica y se agrego:

```text
--emit-banner-compiled
```

La nueva salida usa `BannerVM::compiled_view(...)` y muestra:

- encabezado `.COMPILED_BANNER`;
- funcion de entrada por ID;
- funciones como `function #N nombre`;
- slots locales como `sN = nombre_original`;
- instrucciones con PC numerico;
- operaciones ya expresadas con slots;
- tipos por `type_id`;
- data strings como entradas de `.DATA`;
- `source=` por instruccion cuando existe metadata de origen.

Ejemplo de forma generada:

```text
.COMPILED_BANNER
entry function #0 hulk_main

function #0 hulk_main ; source=program
  slots:
    s0 = hulk_tmp_number_0
  code:
    0: s0 = CONST_NUMBER 1 ; source=tests/eval/foo.hulk:1:7
```

### Archivos relevantes

- `src/backend/main.cpp`
- `src/backend/backend_driver.h`
- `src/backend/backend_driver.cpp`
- `src/vm/banner_vm.h`
- `src/vm/banner_vm.cpp`
- `tests/backend/run_backend_tests.sh`
- `tests/vm/banner_vm_limits_tests.cpp`

### Validacion

La suite de backend incluye casos para `--emit-banner-compiled` en C4, C5 y C6.
El test unitario de VM valida que la vista contenga `.COMPILED_BANNER`, funcion,
slots y PCs.

## 7. Errores runtime sin span ni contexto de instruccion

### Problema

Los errores runtime solo decian la causa inmediata, por ejemplo:

```text
Runtime error: division por cero.
```

Eso obligaba a inspeccionar manualmente la IR o la VM para encontrar la
instruccion exacta y el punto del programa fuente.

### Solucion aplicada

Se agrego metadata de origen y propagacion completa:

- `IR::SourceSpan` en `src/ir/ir.h`.
- `std::optional<SourceSpan>` en `IRInstr`.
- `std::optional<IR::SourceSpan>` en `BannerInstr`.
- `std::optional<IR::SourceSpan>` en `CompiledInstr`.
- `IRGen` recibe `input_path` y usa una pila de spans mientras baja
  expresiones AST.
- `HulkIRToBanner` copia el span al bajar instrucciones.
- `BannerVM` envuelve la ejecucion de cada instruccion y enriquece excepciones.

El nuevo formato de error incluye:

```text
Runtime error en hulk_main pc=2
  source: tests/eval/err_div_zero.hulk:3:7
  instr: s2 = DIV s0, s1
  causa: Runtime error: division por cero.
  stack:
    at hulk_main pc=2
```

Ademas, el `BackendDriver` trata `std::runtime_error` como diagnostico
controlado y lo imprime sin prefijarlo como "error inesperado".

### Ajuste colateral importante

Durante la propagacion de metadata se detecto y corrigio un bug latente en
`IRGen`: `start_function()` dejaba `current_function_` apuntando a una variable
local devuelta por valor. Se movio la asignacion de `current_function_` al
objeto real usado por cada emisor de funcion (`emit_global_function`,
`emit_type_initializer`, `emit_method`, `emit_entry`).

### Validacion

Se agregaron pruebas:

- test unitario de VM que fuerza division por cero y verifica funcion, PC,
  source, instruccion y stack;
- test end-to-end con `tests/eval/err_div_zero.hulk` en
  `tests/backend/run_backend_tests.sh`.

## 8. Tests no cubren invariantes internas de VM

### Problema

El flujo end-to-end ya verificaba muchos programas HULK, pero faltaban pruebas
unitarias directas sobre piezas internas de la VM. Eso dejaba fragiles zonas
como tags de `Word`, handles invalidos, almacenamiento de campos, dispatch y
errores runtime construidos directamente desde BannerIR.

### Solucion aplicada

Se amplio el target `vm-tests` para compilar y ejecutar cuatro binarios:

```text
hulk_vm_value_tests
hulk_vm_tests
hulk_vm_limits_tests
hulk_vm_semantics_tests
```

Las nuevas pruebas cubren:

- `Word` tags:
  - round-trip de numeros;
  - booleanos y `nil`;
  - referencias a strings/objetos con indice y generacion;
  - rango invalido de payload;
  - patron NaN que colisiona con tags, canonicalizado como numero.
- `VMHeap`:
  - strings y objetos alcanzables/no alcanzables;
  - ciclos no alcanzables;
  - reutilizacion de slots con nueva generacion;
  - rechazo de handles viejos o de tipo incorrecto;
  - almacenamiento de distintos tipos de `Word` en campos.
- `BannerVM` con `BannerProgram` construido en C++:
  - programa minimo;
  - llamadas directas con `param_buffer`;
  - recursion finita;
  - objeto con campo;
  - strings dinamicos por `ConcatSpace`;
  - `VCALL` por vtable;
  - fallback por nombre para campos ambiguos;
  - fallback por nombre para metodos ambiguos;
  - error runtime por aridad incorrecta.

### Archivos relevantes

- `Makefile`
- `tests/vm/vm_value_tests.cpp`
- `tests/vm/vm_heap_tests.cpp`
- `tests/vm/banner_vm_limits_tests.cpp`
- `tests/vm/banner_vm_semantics_tests.cpp`

### Validacion

El target:

```bash
make vm-tests
```

compila y ejecuta todos los binarios unitarios de VM. Con esto, los cambios
futuros en representacion de `Word`, GC, slots, dispatch o errores runtime
tienen pruebas mas cercanas al punto de fallo.

## 9. Falta implementar `restricted-inference`

### Problema

La extension de inferencia ya aceptaba type holes con `_` y `auto`, pero no
existia una forma de activar el modo estricto descrito en el reporte: permitir
inferencia solo cuando el programador la pide explicitamente.

Eso hacia indistinguibles estos dos casos desde el flujo end-to-end:

```hulk
let x = 42 in print(x);
let y: _ = 42 in print(y);
```

En modo normal ambos deben seguir siendo validos. En modo restringido, el
primero debe fallar porque omitio la anotacion.

### Solucion aplicada

Se agrego `SemanticOptions`:

```cpp
struct SemanticOptions {
    bool restricted_inference = false;
};
```

`SemanticAnalyzer` recibe esas opciones y, si `restricted_inference` esta
activo, ejecuta un pase previo a inferencia que recorre el AST y reporta error
cuando encuentra anotaciones omitidas en:

- bindings de `let`;
- parametros y retornos de funciones;
- parametros y retornos de metodos;
- parametros de constructores;
- atributos de tipos;
- lambdas, por consistencia con el visitor existente.

La regla se basa en el dato que ya conserva el AST: anotacion omitida es cadena
vacia. Por eso `_` y `auto` quedan permitidos como solicitudes explicitas de
inferencia.

El diagnostico nuevo es:

```text
Inferencia implicita no permitida en modo restringido. Use ': _', ': auto' o escriba un tipo concreto.
```

Tambien se conecto la opcion en:

```bash
./hulk_backend archivo.hulk --restricted-inference
./hulk_semantic archivo.hulk --restricted-inference
```

El backend pasa las opciones semanticas al analyzer y conserva la proteccion
de la vulnerabilidad 1: si aparece este error, no genera IR, no genera
BannerIR y no ejecuta VM.

### Archivos relevantes

- `src/semantic/analyzer.h`
- `src/semantic/analyzer.cpp`
- `src/semantic/main_semantic.cpp`
- `src/backend/backend_driver.h`
- `src/backend/backend_driver.cpp`
- `src/backend/main.cpp`
- `tests/backend/run_backend_tests.sh`
- `tests/extension/restricted_valid_*.hulk`
- `tests/extension/restricted_invalid_*.hulk`
- `tests/expected/backend/restricted_valid_*.expected`

### Validacion

Se agregaron secciones nuevas al runner:

```text
BACKEND RESTRICTED-INFERENCE VALIDOS
BACKEND RESTRICTED-INFERENCE INVALIDOS
```

Los validos demuestran que `: _`, `: auto`/tipo concreto y anotaciones
explicitas siguen funcionando con la bandera. Los invalidos demuestran que:

- el mismo programa pasa en modo normal;
- falla con `--restricted-inference`;
- el error contiene el diagnostico esperado;
- no se emite `--emit-ir`;
- no se emite `--emit-banner`;
- no se emite `--emit-banner-compiled`;
- no quedan archivos de salida creados.

## 10. Limpiar del flujo end-to-end `lambda`, `range`/`Iterable` y `protocol`

### Problema

El frontend conservaba piezas para features que no forman parte del flujo
soportado por el backend final:

- `lambda`;
- `for` sobre `Iterable/range`;
- llamada `range(...)`;
- `protocol`.

Antes, algunos de estos casos llegaban tarde hasta `IRGen`, que los rechazaba
con `unsupported(...)`. Eso evitaba ejecuciones incorrectas, pero dejaba una
promesa falsa: parser/AST/semantica parecian aceptar construcciones que el
pipeline end-to-end no podia bajar a HulkIR/BannerIR.

### Solucion aplicada

Se formalizo la politica conservadora: mantener nodos y parser donde ya aportan
estructura, pero bloquear el feature en `SemanticAnalyzer` antes de resolver,
inferir, type-checkear o generar IR.

El nuevo pase de politica semantica reporta:

```text
Feature no soportado en el flujo end-to-end: <feature>.
```

Se bloquean explicitamente:

- `lambda`;
- `for` sobre `Iterable/range`;
- `range`;
- `protocol`.

Despues de revisar `hulk-docs.pdf`, el `for` queda tratado como una feature del
lenguaje HULK. Lo que no se anuncia como soportado en este pipeline es el caso
end-to-end que depende de `Iterable`/`range`, que todavia no tiene bajada segura
hacia el backend.

Tambien se retiro `range` de las funciones builtin registradas por
`SemanticTables`, para que no quede anunciado como builtin soportado por
semantica. La ruta de `FunctionCall("range")` se detecta por nombre antes de
resolver simbolos, por lo que el usuario recibe el diagnostico estable de
feature no soportado en vez de un error tardio de IR.

Para que el rechazo sea consistente:

- se agrego token `protocol`;
- se agrego parser minimo para declaraciones `protocol`;
- se agrego parser minimo para lambdas de la forma `(x: T): R => expr`;
- se enlazo `ProtocolDecl` en el build compartido;
- `IRGen` conserva sus defensas `unsupported(...)`, pero ya no son la primera
  barrera esperada.

### Archivos relevantes

- `src/semantic/analyzer.cpp`
- `src/semantic/semantic_tables.cpp`
- `src/lexer/token_kind.hpp`
- `src/lexer/keywords.hpp`
- `src/lexer/lexer.cpp`
- `src/lexer/main.cpp`
- `src/parser/grammar.y`
- `src/parser/parser.cpp`
- `src/parser/parser.hpp`
- `src/parser/parser_lexer_adapter.cpp`
- `src/ast/protocols/protocolMethodSig.h`
- `Makefile`
- `tests/backend/run_backend_tests.sh`
- `tests/backend/unsupported/*.hulk`

### Validacion

Se agrego la seccion:

```text
BACKEND FEATURES NO SOPORTADOS
```

Los tests cubren:

- lambda;
- `for (x in range(...))`;
- llamada directa `range(...)`;
- declaracion `protocol`.

Cada caso verifica que:

- el backend falla en modo default;
- el mensaje contiene `Feature no soportado en el flujo end-to-end`;
- no se emite `--emit-ir`;
- no se emite `--emit-banner`;
- no se emite `--emit-banner-compiled`;
- no quedan archivos de salida creados.

## 11. Literales numericos invalidos no se convierten silenciosamente en `0`

### Problema

`src/parser/parser_lexer_adapter.cpp` convertia lexemas numericos con
`std::stod`. Si la conversion fallaba, el helper capturaba la excepcion y
devolvia `0.0`. Eso permitia que un literal fuera de rango entrara al AST como
un numero valido distinto del programa fuente.

### Solucion aplicada

La conversion ahora valida el resultado de `std::stod` y reporta un diagnostico
sintactico con el span del token cuando el literal no se puede representar.
Para un numero fuera de rango se emite:

```text
Literal numerico fuera de rango
```

En ese caso el adaptador devuelve el token especial de error de Bison
`YYerror`, asi que no se construye `NUMBER_LITERAL(0)` como fallback. El
`BackendDriver` ya bloquea el flujo si el parser o el motor de diagnosticos
reportan errores, por lo que tampoco se genera IR, BannerIR ni BannerIR
compilado.

### Archivos relevantes

- `src/parser/parser_lexer_adapter.cpp`
- `tests/backend/run_backend_tests.sh`
- `tests/backend/frontend_invalid/out_of_range_number.hulk`

### Validacion

Se agrego la seccion:

```text
BACKEND FRONTEND INVALIDOS
```

El test `out_of_range_number.hulk` verifica que:

- el backend rechaza el programa en modo default;
- el mensaje contiene `Literal numerico fuera de rango`;
- no se emite `--emit-ir`;
- no se emite `--emit-banner`;
- no se emite `--emit-banner-compiled`;
- no quedan archivos de salida creados.

## 12. Escapes de strings se interpretan como valor semantico

### Problema

El lexer ya aceptaba barras invertidas dentro de strings, pero el parser solo
quitaba las comillas externas con un `substr`. Por eso literales como
`"a\nb"` llegaban al AST como los dos caracteres `\` y `n`, no como un salto
de linea. La documentacion de HULK describe strings con comillas escapadas,
line endings y tabs, asi que el valor ejecutado no coincidia con el programa
escrito.

### Solucion aplicada

La decodificacion se movio al adaptador lexer-parser. Cuando llega un token
`String`, `decode_string_lexeme` valida la forma del literal y traduce:

```text
\n -> newline
\r -> carriage return
\t -> tab
\" -> "
\\ -> \
```

Si encuentra un escape desconocido, reporta un diagnostico sintactico con el
span del literal y devuelve `YYerror`; asi no se construye un nodo
`String` con contenido corrupto. En `grammar.y`, `STRING_LITERAL` ahora usa el
valor ya decodificado directamente, y `parser.cpp`/`parser.hpp` fueron
regenerados con Bison.

### Archivos relevantes

- `src/parser/parser_lexer_adapter.cpp`
- `src/parser/grammar.y`
- `src/parser/parser.cpp`
- `src/parser/parser.hpp`
- `tests/backend/run_backend_tests.sh`
- `tests/backend/regression/string_escapes.hulk`
- `tests/backend/frontend_invalid/invalid_string_escape.hulk`
- `tests/expected/backend/string_escapes.expected`

### Validacion

Se agrego una regresion positiva que imprime strings con salto de linea, tab,
comillas escapadas y barra invertida. Tambien se agrego un caso negativo:

```hulk
print("bad\q");
```

El test invalido verifica que:

- el backend rechaza el programa;
- el mensaje contiene `Escape de string no soportado`;
- no se emite `--emit-ir`;
- no se emite `--emit-banner`;
- no se emite `--emit-banner-compiled`;
- no quedan archivos de salida creados.

## 13. Parser generado sincronizado con `grammar.y`

### Problema

El informe marcaba una inconsistencia entre la gramatica fuente y el parser
generado alrededor de los tokens `AUTO` y `UNDERSCORE_TYPE`. El lexer no
producía esos tokens; en la practica `auto` y `_` viajan como identificadores,
y la semantica los interpreta como type holes explicitos.

### Solucion aplicada

Se cerro la decision por la opcion conservadora ya compatible con el resto del
compilador: mantener `auto` y `_` como pseudo-tipos expresados mediante
`IDENTIFIER`. La regla de tipos queda en:

```yacc
type_expr
    : IDENTIFIER
```

El parser generado fue regenerado desde `src/parser/grammar.y`, por lo que
`src/parser/parser.cpp` y `src/parser/parser.hpp` reflejan esa fuente. Ademas,
se agrego el target:

```bash
make parser-sync-check
```

Este target:

- falla si reaparecen `AUTO` o `UNDERSCORE_TYPE` en lexer/parser;
- regenera Bison en un arbol temporal con las mismas rutas relativas;
- compara `parser.cpp`, `parser.hpp` y `location.hh` contra los archivos del repo;
- falla si hay drift entre la gramatica y lo generado.

### Archivos relevantes

- `Makefile`
- `src/parser/grammar.y`
- `src/parser/parser.cpp`
- `src/parser/parser.hpp`
- `src/parser/location.hh`

### Validacion

Se ejecuto `make parser-sync-check`. El target confirma:

```text
parser-sync-check: parser generado sincronizado con grammar.y
```

Tambien se mantienen verdes las pruebas que usan `auto` y `_` como pseudo-tipos
en `tests/extension`, incluyendo los casos de inferencia restringida.

## 14. Multiples expresiones globales como extension local

### Problema

La referencia academica describe un programa como cero o mas declaraciones
globales y una unica expresion final. El parser del repositorio aceptaba varias
expresiones globales y las empaquetaba en un `ExprBlock`, pero esa extension no
estaba documentada como parte del dialecto end-to-end.

### Solucion aplicada

Se formalizo la decision de mantener la extension porque buena parte de la
suite y de los ejemplos operativos del proyecto ya dependen de ella. La regla
del dialecto implementado queda asi:

```text
decl* expr+
```

Si hay una sola expresion global, se conserva como entrypoint directo. Si hay
mas de una, el parser construye un bloque implicito equivalente a:

```hulk
{
    expr1;
    expr2;
    exprN;
}
```

Las expresiones se ejecutan en orden de aparicion y el valor del programa es el
valor de la ultima expresion, igual que en un bloque explicito.

Para que no sea un comportamiento escondido, se agrego el helper
`make_global_entrypoint` en `grammar.y` y un comentario junto a la construccion
del `ExprBlock` implicito. Tambien se documento la diferencia entre HULK base y
el dialecto end-to-end en `doc/analisis_gramatica_hulk_base.md`.

### Archivos relevantes

- `src/parser/grammar.y`
- `src/parser/parser.cpp`
- `src/parser/parser.hpp`
- `doc/analisis_gramatica_hulk_base.md`
- `tests/backend/regression/multiple_global_exprs.hulk`
- `tests/expected/backend/multiple_global_exprs.expected`

### Validacion

Se agrego la regresion `multiple_global_exprs.hulk`, que verifica ejecucion en
orden de varias expresiones globales:

```hulk
print("global 1");
print("global 2");
print("global 3");
```

La suite de backend tambien conserva cubiertos los programas historicos que
dependen de esta extension, como `c4_literals`, `c4_strings`, `c5_recursion` y
`c6_inheritance`.

## 15. Concatenacion restringida a `String` y `Number`

### Problema

El type checker visitaba `StringBinOp` sin validar los tipos de los operandos.
Luego BannerVM implementaba `Concat` y `ConcatSpace` con `to_string(...)`, lo
que convertia cualquier `Word` a texto. Eso hacia publicas conversiones no
documentadas para booleanos, objetos o `nil`.

### Solucion aplicada

Se adopto la politica estricta alineada con la documentacion:

- `@` y `@@` solo aceptan operandos `String` o `Number`;
- al menos uno de los dos operandos debe ser `String`;
- `Boolean`, objetos y `nil` quedan fuera del contrato de concatenacion.

En `TypeChecker::visit(StringBinOp&)` se reportan diagnosticos semanticos
bloqueantes cuando se incumple esa regla. Ejemplos:

```text
Operador de concatenacion '@' solo admite operandos String o Number; se encontro 'Boolean'.
Operador de concatenacion '@' requiere al menos un operando String.
```

Tambien se agrego una defensa en BannerVM para BannerIR manual o malicioso:
`Concat` y `ConcatSpace` ya no llaman directamente a `to_string` sobre cualquier
`Word`, sino a una ruta que valida `String`/`Number` y exige un `String` antes
de asignar el string resultante.

### Archivos relevantes

- `src/typecheck/type_checker.cpp`
- `src/vm/banner_vm.cpp`
- `tests/backend/run_backend_tests.sh`
- `tests/backend/invalid_concat/concat_bool.hulk`
- `tests/backend/invalid_concat/concat_number_number.hulk`
- `tests/backend/invalid_concat/concat_object.hulk`
- `tests/vm/banner_vm_semantics_tests.cpp`

### Validacion

Se agrego la seccion:

```text
BACKEND CONCAT INVALIDOS
```

Los casos nuevos verifican que el backend rechaza:

- `"x" @ true`;
- `1 @ 2`;
- `"box: " @ b` con `b` objeto.

Cada caso exige el diagnostico `Operador de concatenacion`, falla en modo
default y comprueba que no se emitan IR, BannerIR ni BannerIR compilado.

En `vm-tests` se agregaron pruebas directas de BannerVM para confirmar que:

- `String @ Number` funciona;
- `String @ Boolean` falla en runtime;
- `Number @ Number` falla en runtime.

## Comandos de verificacion ejecutados

Durante el cierre de estas vulnerabilidades se ejecutaron:

```bash
make parser-gen
make parser-sync-check
make parser-demo
make -B backend
make vm-tests
make backend-tests
make semantic
./hulk_semantic tests/extension/restricted_invalid_implicit_let.hulk --restricted-inference
./hulk_semantic tests/extension/restricted_valid_let.hulk --restricted-inference
./hulk_backend tests/extension/restricted_invalid_implicit_let.hulk
./hulk_backend tests/backend/unsupported/unsupported_lambda.hulk
./hulk_backend tests/backend/unsupported/unsupported_protocol.hulk --emit-ir -o /tmp/unsupported_protocol.hir
./hulk_backend tests/backend/unsupported/unsupported_range_call.hulk --emit-banner-compiled -o /tmp/unsupported_range.compiled.banner
./hulk_backend tests/backend/frontend_invalid/out_of_range_number.hulk
./hulk_backend tests/backend/regression/string_escapes.hulk
./hulk_backend tests/backend/frontend_invalid/invalid_string_escape.hulk
./hulk_backend tests/backend/regression/multiple_global_exprs.hulk
./hulk_backend tests/backend/invalid_concat/concat_bool.hulk
./hulk_backend tests/backend/invalid_concat/concat_number_number.hulk
./hulk_backend tests/backend/invalid_concat/concat_object.hulk
```

Resultado final relevante:

```text
BACKEND RESUMEN
  Total  : 110
  Passed : 110
  Failed : 0
```

Tambien se ejecuto una prueba manual:

```bash
./hulk_backend tests/eval/err_div_zero.hulk
```

que produjo un error runtime con `source`, `pc`, instruccion compilada y stack
trace.

## Estado final

Las vulnerabilidades 1 a 15 quedaron cerradas con cambios de implementacion y
tests. El pipeline ahora respeta errores semanticos bloqueantes, libera heap no
alcanzable, limita recursos de VM, permite inspeccionar la forma baja de
BannerIR, reporta errores runtime con contexto suficiente para depurar desde
el programa fuente, cuenta con pruebas unitarias para invariantes internas de
la VM y ofrece un modo opt-in de inferencia restringida para exigir tipos
concretos o type holes explicitos. Ademas, los features fuera de alcance
end-to-end quedan bloqueados temprano con diagnosticos probados y no llegan a
la generacion de IR ni a BannerVM, y los literales numericos no representables
ya no pueden convertirse silenciosamente en `0`. Los strings tambien llegan al
AST y al backend con sus escapes decodificados, y los escapes desconocidos
fallan temprano. Finalmente, el parser generado queda protegido contra drift
respecto a `grammar.y` mediante un chequeo reproducible. El soporte de multiples
expresiones globales ya no queda implicito: es una extension local documentada,
probada y definida como bloque global implicito. La concatenacion tambien queda
acotada al contrato documentado, sin conversion publica de booleanos, objetos o
`nil`.
