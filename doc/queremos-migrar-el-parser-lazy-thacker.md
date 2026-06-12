# Migración del parser HULK: de Bison a un generador LALR(1) propio

## Contexto

El parser actual de HULK está generado por **Bison** (`%skeleton "lalr1.cc"`, LALR(1))
a partir de [src/parser/grammar.y](src/parser/grammar.y). Queremos **eliminar la
dependencia de Bison** pero **mantener el mismo tipo de parser (LALR(1))**, escribiendo
un **generador de tablas propio** que preserve la gramática, las acciones semánticas y,
sobre todo, el **comportamiento sintáctico y el AST resultante idénticos**.

No se migra a recursive-descent / LL(1) / Pratt / packrat / combinators. No se "limpia"
la gramática (eso queda como mejora opcional posterior). El objetivo es un reemplazo
*drop-in*: los cuatro consumidores (`hulk_parser_demo`, `hulk_eval`, `hulk_semantic`,
`hulk_backend`) siguen instanciando `hulk::parser::Parser` exactamente igual.

### Decisiones tomadas (preguntas resueltas con el usuario)

- **Lenguaje del generador:** **C++ self-hosted**. La herramienta se compila con el
  proyecto (sin dependencia Python/bison externa) y corre en build-time emitiendo C++.
- **Entrada de la gramática:** **Nuevo archivo `.grammar` fiel**, transcripción 1:1 de las
  producciones y precedencias de `grammar.y`, con los bloques de acción `{ … }` copiados
  **verbatim**. Sin lidiar con directivas bison-only (`%code`, `%define`, `variant`, etc.).

---

## 1. Estado actual del parser

### Archivos que participan

| Archivo | Rol | ¿Generado? |
|---|---|---|
| [src/parser/grammar.y](src/parser/grammar.y) | Gramática Bison + acciones semánticas + precedencias + `Parser::error` | Fuente |
| [src/parser/parser.cpp](src/parser/parser.cpp) / [parser.hpp](src/parser/parser.hpp) | Autómata LALR(1), `Parser`, `symbol_type`, `make_*` | **Bison** |
| [src/parser/location.hh](src/parser/location.hh) | Tipo `location` (spans begin/end) | **Bison** |
| [src/parser/parser_driver.hpp](src/parser/parser_driver.hpp) / [.cpp](src/parser/parser_driver.cpp) | `ParserDriver`: puente lexer↔parser, diagnósticos, `result()` | Hecho a mano |
| [src/parser/parser_lexer_adapter.cpp](src/parser/parser_lexer_adapter.cpp) | `yylex(driver)`: mapea `TokenKind`→símbolo Bison, decodifica nº/strings | Hecho a mano |
| [src/parser/main.cpp](src/parser/main.cpp) | Demo: parse + dump AST (`dump_expr/dump_decl/dump_ast`) | Hecho a mano |
| [Makefile](Makefile) | `parser-gen` (bison), `parser-sync-check`, `parser-demo`, etc. | — |

Consumidores que instancian el parser (todos con el mismo patrón):
[main.cpp:486](src/parser/main.cpp#L486), [eval/main.cpp:57](src/eval/main.cpp#L57),
[main_semantic.cpp:58](src/semantic/main_semantic.cpp#L58),
[backend_driver.cpp:36](src/backend/backend_driver.cpp#L36).

### API pública que se DEBE preservar (contrato drop-in)

```
hulk::lexer::Lexer       lexer(source, engine);
hulk::parser::ParserDriver driver(lexer, engine);
hulk::parser::Parser     parser(driver);     // ctor toma ParserDriver&
int parse_rc = parser.parse();               // 0 = éxito
Hulk::ASTnode* root = driver.result();       // AST (downcast a Hulk::Program)
```

`ParserDriver` se mantiene **sin cambios** en su interfaz pública:
`next_token()`, `report_syntax_error(msg[, span])`, `set_result()`, `result()`,
`take_result()`. Internamente sigue tirando tokens del `Lexer` y reportando al
`DiagnosticEngine`.

### Qué lógica vive hoy en Bison vs. en el adapter

- **En Bison (`grammar.y` → `parser.cpp`):** el autómata LALR(1) (estados, ACTION/GOTO),
  el value-stack heterogéneo (`%define api.value.type variant`), el merge de localizaciones
  (`@$`, `@n`), el bucle shift/reduce, el dispatch de acciones, y `Parser::error()`.
- **En `parser_lexer_adapter.cpp`:** la conversión `hulk::lexer::Token` → símbolo del parser
  (vía `Parser::make_*`), **la validación del literal numérico** (`std::stod`, finitud,
  rango) y **la decodificación de strings** (comillas, escapes), emitiendo diagnósticos
  *sintácticos* en caso de error. Esta lógica **NO es de Bison** y debe conservarse intacta.

---

## 2. Arquitectura propuesta

Reemplazamos Bison por **tres piezas** + un **spec de gramática**:

```
src/parser/hulk.grammar                  ← (NUEVO) gramática fiel + acciones verbatim
        │  (build-time)
        ▼
tools/parsergen/  (NUEVO, C++ self-hosted)  ← generador LALR(1)
        │  emite
        ▼
src/parser/parser_tables.{hpp,cpp}       ← (GENERADO) ACTION/GOTO + metadata + switch de acciones
        │  consumido por
        ▼
src/parser/lr_engine.{hpp,cpp} + parser.hpp   ← (NUEVO) motor table-driven = clase `Parser`
        ▲
src/parser/parser_lexer_adapter.cpp      ← (MODIFICADO) yylex devuelve el nuevo Symbol
```

### Representación de la gramática (`.grammar`)

Formato propio mínimo (no Bison), transcrito 1:1 desde `grammar.y`:
- **Tokens** con su tipo semántico opcional (`IDENTIFIER : string`, `NUMBER_LITERAL : double`, …).
- **Tabla de `%type`** (nonterminal → tipo C++ del valor).
- **Niveles de precedencia** en orden (de menor a mayor), con asociatividad (`left/right/nonassoc`).
- **Producciones**, cada alternativa con: símbolos del RHS, un `%prec TOKEN` opcional,
  y un **bloque de acción `{ … }` copiado textual** desde `grammar.y` (usa `$$/$n/@$/@n`).

### Representación de tokens, no-terminales, producciones y acciones

- **Símbolos**: un único espacio de ids enteros — terminales primero (incluido `$end`),
  luego no-terminales — igual que Bison. El generador asigna los ids de forma estable
  desde el orden de declaración.
- **`Symbol` en runtime** (NUEVO, reemplaza `Parser::symbol_type`): `{ int sym; ParserValue value; hulk::common::Span span; }`.
- **Producciones**: en las tablas generadas, cada regla se describe con `(lhs_sym, rhs_len)`
  y su `rule_precedence` (para resolución de conflictos). El número de regla es el orden
  en el `.grammar` (clave para reduce/reduce).
- **Acciones**: se emiten como un `switch(rule_number)` cuyo cuerpo es el bloque verbatim,
  con `$$/$n/@$/@n` sustituidos a accesos al value/location-stack (ver §5).

### Tablas ACTION/GOTO

Se **generan offline** (modelo Bison) y se emiten como arreglos C++ en `parser_tables.cpp`:
- `ACTION[state][terminal]` → shift(state) | reduce(rule) | accept | error.
- `GOTO[state][nonterminal]` → state.
- Metadata: `rule_lhs[]`, `rule_len[]`, `rule_prec[]`, `symbol_name[]` (para mensajes de error).
Se pueden emitir densas (matriz) o comprimidas; para esta gramática (cientos de estados,
~60 terminales) una **matriz densa** es suficiente y mucho más simple de depurar.

### Valores semánticos heterogéneos (equivalente a `%type` + `variant`)

`ParserValue` = `std::variant` con **exactamente** los tipos declarados en `grammar.y`
(líneas 145–162) más `std::monostate` para tokens sin valor:

`monostate, double, std::string, ExprPtr, ExprList, DeclPtr, DeclList, ProgramPtr,
BindingPtr, BindingList, ParamList, Param, ProtocolMethodList, ProtocolMethodSig,
ElifList, TypeMember, TypeMemberList, InheritsInfo, LValueTarget, TopLevelItems`.

`std::variant` soporta tipos *move-only* (los `unique_ptr` y `vector<unique_ptr>`), así
que `$n`/`$$` se traducen a `std::get<T>` + `std::move`. Los `struct` auxiliares
(`InheritsInfo`, `LValueTarget`, `TopLevelItems`) se mueven desde `grammar.y` a un header
compartido (`parser_semantic_types.hpp`) para que tanto el spec como el engine los usen.

---

## 3. Algoritmo de generación (núcleo del generador)

Pasos del generador (en `tools/parsergen/`), en orden:

1. **Gramática aumentada**: añadir `S' → program $end`. `program` es el símbolo inicial.
2. **Nullable + FIRST sets**: punto fijo estándar sobre todas las producciones
   (necesario para los lookaheads LR(1)).
3. **Ítems LR(1)**: construir la colección canónica de conjuntos de ítems
   (`closure`, `goto`) con lookahead de 1 token.
4. **Merge LALR(1)**: fusionar estados con el **mismo core LR(0)** (mismos ítems ignorando
   lookahead), uniendo los lookaheads. Resultado: autómata LALR(1) equivalente al de Bison.
   *(Alternativa válida: DeRemer–Pennello directo; se elige canónico+merge por ser el más
   simple de verificar contra el `.output` de Bison.)*
5. **ACTION/GOTO**: por cada estado, shift para ítems con terminal tras el punto, reduce
   para ítems completos sobre su lookahead, `accept` sobre `S' → program · $end`, goto para
   no-terminales.
6. **Detección y resolución de conflictos** (idéntica a Bison):
   - **shift/reduce**: si el terminal y la regla tienen precedencia ⇒ gana mayor precedencia;
     si **empatan** ⇒ `%left`→**reduce**, `%right`→**shift**, `%nonassoc`→**error**; si a
     alguno le falta precedencia ⇒ **default: shift** (y se cuenta como conflicto reportado).
   - **reduce/reduce**: gana la **regla de menor número** (la declarada antes en el archivo);
     se reporta.
   - El generador imprime un **reporte de conflictos** y **falla el build si el conjunto de
     conflictos difiere del baseline congelado** de Bison (archivo `doc/parser/expected_conflicts.txt`).
7. **Emisión**: `parser_tables.hpp/.cpp` con las tablas, la metadata de reglas, los nombres
   de símbolos, y el `switch` de acciones con `$/@` sustituidos.

Tamaño esperado: la gramática es pequeña; la construcción canónica LR(1) + merge corre en
milisegundos. No hay riesgo real de explosión de estados.

---

## 4. Conflictos actuales y su resolución

**Hallazgo central:** la gramática está **completamente estratificada** — hay un
no-terminal por nivel de precedencia
(`logic_or → logic_and → equality → relation → type_test_expr → concat → additive →
multiplicative → power → unary → postfix → primary`). Por construcción, **no hay
conflictos de operadores**: las declaraciones `%left/%right/%nonassoc/%prec` son en su
mayoría **vestigiales** y no cambian el parseo de los casos listados. Esto reduce mucho el
riesgo de la migración.

**Sitios candidatos a conflicto y resolución requerida (todos → preferir shift, = default Bison):**

1. **`type_expr · STAR`** vs reducir `type_test_expr : concat IS/AS type_expr ·` ante
   lookahead `STAR`. Es el conflicto "real" más probable. **Resolución: SHIFT** (para que
   `x is T*` y `T**` anidados consuman el `*` como sufijo de tipo, no como multiplicación).
2. **`expr_list : expr_list · SEMICOLON expr`** vs **`opt_semi : SEMICOLON ·`** en bloques
   (el `;` final opcional). Tras shiftear `;`, el estado mezcla `opt_semi : SEMICOLON ·`
   (reduce ante `RBRACE`) y `expr_list : … SEMICOLON · expr` (shift ante inicio de expr).
   Lookaheads **disjuntos** ⇒ LALR(1) lo resuelve limpio. Mismo patrón en `top_level_items`
   con `opt_semi`.
3. **`lvalue` vs `postfix`/`primary`** sobre `postfix DOT IDENTIFIER`, `IDENTIFIER`, `SELF`:
   `lvalue` solo va seguido de `DESTRUCTIVE_ASSIGN` (su único FOLLOW), que **no** está en
   FOLLOW(postfix/primary). Lookaheads disjuntos ⇒ se resuelve por LALR(1) **sin conflicto
   real**.

**Acción obligatoria en implementación (Fase 0):** obtener la lista **autoritativa** con
`bison --report=all -Wcounterexamples` (genera `parser.output` con estados, ítems y la
cuenta exacta de conflictos y su resolución). Congelarla como `doc/parser/expected_conflicts.txt`.
El generador propio **debe reproducir esas mismas resoluciones** y fallar si difieren.

**No** se reescribe la gramática para "limpiar" estos puntos (queda como mejora opcional
posterior). El parser nuevo replica la política de Bison: shift sobre S/R sin precedencia,
regla de menor número sobre R/R.

---

## 5. Portado de acciones semánticas

Las acciones se copian **verbatim** desde `grammar.y` al `.grammar`. El generador hace la
sustitución mecánica (idéntica a Bison):

- `$$` → slot de valor del resultado (tipado por el `%type` del LHS).
- `$n` → `std::get<Tn>(std::move(stack[base+n-1].value))`, con `Tn` = `%type` del símbolo n.
- `@$` → span resultado (ver §6, merge primero..último).
- `@n` → `stack[base+n-1].span`.
- Regla **sin acción** con LHS tipado ⇒ acción por defecto `$$ = $1` (como Bison). En esta
  gramática solo `opt_semi` (void) carece de acción, así que el caso por defecto es trivial,
  pero se implementa por robustez.
- **No hay mid-rule actions** en `grammar.y` (todas las acciones están al final de la regla)
  ⇒ se elimina toda esa clase de complejidad.
- **No hay reglas `error`** (sin recuperación de errores) ⇒ el engine, ante un error de
  sintaxis, llama a `driver.report_syntax_error` y aborta con `parse_rc != 0`, igual que
  Bison sin recuperación.

Tipos semánticos importantes (de `grammar.y`): `ExprPtr`, `ExprList`, `DeclPtr`, `DeclList`,
`ProgramPtr`, `BindingPtr`, `BindingList`, `ParamList`, `Param`, `ProtocolMethodList`,
`ProtocolMethodSig`, `ElifList`, `TypeMember`, `TypeMemberList`, `InheritsInfo`,
`LValueTarget`, `TopLevelItems`, más `std::string`/`double` de terminales.

Nodos AST que las acciones deben construir **idénticos** (mismos constructores que hoy):
`Program`, `FunctionDecl` (4 sobrecargas según return-ann/inline-vs-block), `TypeDecl`
(4 sobrecargas según ctor-params/parent), `ProtocolDecl`, `ProtocolMethodSig`, `LetIn`,
`VariableBinding`, `IfStmt` (+`ElifBranch`), `WhileStmt`, `For`, `DestructiveAssign`,
`DestructiveAssignMember`, `LogicBinOp`, `ArithmeticBinOp`, `StringBinOp`,
`ArithmeticUnaryOp`, `LogicUnaryOp`, `FunctionCall`, `MethodCall`, `MemberAccess`,
`NewExpr`, `BaseCall`, `IsExpr`, `AsExpr`, `Print`, `BuiltinCall`, `Number`, `String`,
`Boolean`, `VariableReference`, `SelfRef`, `ExprBlock`, `TypeMemberAttribute`,
`TypeMemberMethod`, `Param`. Todos llevan `span = to_span(@$)` salvo donde hoy no se asigna
(p.ej. `LPAREN expr RPAREN` devuelve el `expr` interno tal cual).

---

## 6. Contratos de comportamiento invariantes (no deben cambiar)

- **Estructura del programa**: `decl* exprGlobalFinal`. Si **no** hay expresión global final
  ⇒ diagnóstico `"el programa debe contener una expresion global final"` y se construye
  `Program` con `ExprBlock` vacío. Si hay **más de una** ⇒
  `"Solo se permite una expresion global final"` (en el span de la segunda, `@2`).
- **Precedencia/asociatividad** (de menor a mayor, líneas 133–143 de `grammar.y`):
  `OR` < `AND` < `== !=` < `< <= > >=` < `is/as` < `@ @@` < `+ -` < `* / %` < `^` < unario `- !` < postfix.
- **`^` asociativo a derecha** — estructural (`power : unary CARET power`). `2 ^ 3 ^ 2` = `2^(3^2)`.
- **`-2 ^ 2` = `(-2) ^ 2`** — **estructural**, no por precedencia: `power : unary CARET power`
  toma `unary` (que incluye el menos unario) como base, así que el `-` se liga al `2` antes
  del `^`. El generador lo reproduce solo con replicar las producciones.
- **`1 < 2 < 3` = `(1<2)<3`** y **`1 == 2 == 3` = `(1==2)==3`**: **aceptados y asociativos a
  izquierda** vía recursión por izquierda, *a pesar* de `%nonassoc` (que aquí no se consulta
  por no haber conflicto). Replicar la recursión izquierda preserva esto.
- **`(expr)` devuelve `expr` directamente** (`primary : LPAREN expr RPAREN` sin envoltura).
- **`PI` y `E`** → `Number` (3.14159265358979323846 / 2.71828182845904523536).
- **`print(expr)`** → `Print`.
- **`sqrt/sin/cos/exp/log/rand`** → `BuiltinCall` (rand sin args; log con 2 args).
- **`FunctionCall` vs `MethodCall`** (`postfix LPAREN args RPAREN`): si el callee es
  `VariableReference` ⇒ `FunctionCall`; si es `MemberAccess` ⇒ `MethodCall`; en otro caso ⇒
  diagnóstico `"solo se pueden invocar identificadores o accesos a metodo"`. (Ej.: `foo()(1)`
  parsea estructuralmente pero **emite ese error**; `foo.bar.baz(1)` ⇒ `MethodCall`.)
- **Type holes `_` y `auto`**, y **tipos `T*`, `T**`, …** (`type_expr : type_expr STAR`).
- **Spans (`to_span(@$)`)**: `@$.begin = @1.begin`, `@$.end = @N.end`; para regla vacía,
  begin=end = posición del lookahead actual. Estos spans fluyen a los nodos AST y a los
  diagnósticos, así que el merge debe ser idéntico al de Bison.

---

## 7. Adaptación del lexer y literales (`parser_lexer_adapter.cpp`)

- Se **conserva la función** `yylex(ParserDriver& driver)` pero ahora **devuelve el nuevo
  `Symbol`** en lugar de `Parser::symbol_type`. El `switch(token.kind)` que mapea cada
  `hulk::lexer::TokenKind` a su símbolo se mantiene 1:1 (mismo conjunto de terminales).
- **Se preserva intacta** la validación numérica (`parse_number_lexeme`: `std::stod`,
  `consumed == size`, `std::isfinite`, captura de `std::out_of_range`) y la decodificación de
  strings (`decode_string_lexeme`: comillas, escapes `n r t " \\`, escape incompleto, escape
  no soportado). Solo cambia el *tipo de retorno* y cómo se construye el símbolo de error.
- **Diagnósticos sintácticos a preservar verbatim** (mismos strings, mismo nivel
  `Syntactic`/`Error`, mismo span):
  - `"Literal numerico invalido"`
  - `"Literal numerico fuera de rango"`
  - `"Literal de string invalido"`
  - `"Escape de string incompleto"`
  - `"Escape de string no soportado: \X"`
- **Camino de error**: hoy, ante literal inválido, el adapter devuelve `Parser::make_YYerror`,
  lo que dispara el error de Bison. En el nuevo motor, `yylex` devolverá un **símbolo de error
  centinela** que el engine trata como token inválido ⇒ `report_syntax_error` del parser + abort.
  El diagnóstico específico del literal ya se emitió dentro del adapter (igual que hoy), de modo
  que el efecto observable (mensaje + exit code) sea idéntico.

---

## 8. Cambios esperados en archivos

**Crear:**
- `src/parser/hulk.grammar` — gramática fiel + acciones verbatim.
- `src/parser/parser_semantic_types.hpp` — `InheritsInfo`, `LValueTarget`, `TopLevelItems`,
  aliases de tipos (extraídos de `grammar.y`).
- `src/parser/symbol.hpp` — `Symbol` + `ParserValue` (variant).
- `src/parser/lr_engine.hpp/.cpp` — clase `Parser` table-driven (API preservada) + merge de spans.
- `src/parser/parser_tables.hpp/.cpp` — **generado** (ACTION/GOTO + switch de acciones).
- `tools/parsergen/` — generador C++ (reader `.grammar`, FIRST, ítems LR(1), merge LALR,
  ACTION/GOTO, resolución de conflictos, emisor).
- `doc/parser/expected_conflicts.txt` — baseline de conflictos de Bison (oráculo).
- `tests/parser/run_parser_golden.sh` + corpus de AST dumps golden.

**Modificar:**
- [src/parser/parser_lexer_adapter.cpp](src/parser/parser_lexer_adapter.cpp) — `yylex` devuelve `Symbol`.
- [src/parser/parser.hpp](src/parser/parser.hpp) — pasa a declarar la **nueva** clase `Parser`
  (table-driven), conservando la API. (Deja de ser el header de Bison.)
- [Makefile](Makefile) — ver abajo.
- Docs: [CLAUDE.md](CLAUDE.md) (sección build/pipeline: "Bison LALR(1)" → "generador propio LALR(1)"),
  `doc/gramatica_atributada_hulk.md`, [tests/parser/README.md](tests/parser/README.md),
  `REPORT.md`.

**Eliminar / retirar (tras lograr paridad):**
- `src/parser/parser.cpp` (autómata Bison), `src/parser/location.hh` (Bison). El `grammar.y`
  se **conserva temporalmente** como oráculo (ver §9) y se elimina en una segunda etapa.

**Makefile:**
- Nuevo target **`parser-gen-own`**: compila `tools/parsergen` y ejecuta
  `parsergen src/parser/hulk.grammar -o src/parser/parser_tables`.
- `PARSER_OBJS` pasa a `lr_engine.o + parser_tables.o + parser_driver.o + parser_lexer_adapter.o`
  (quita `parser.o` de Bison del camino por defecto).
- `parser_tables.cpp` se compila con flags relajados (reusar `CXXFLAGS_BISON`, renombrar a
  `CXXFLAGS_GENERATED`).
- Nuevo **`parser-sync-check-own`**: regenera tablas y hace `diff` contra las committeadas
  (garantiza que el generado está sincronizado con `hulk.grammar`).
- `parser-gen` / `parser-sync-check` (Bison) quedan **gated tras `USE_BISON=1`** para el
  oráculo; se eliminan en la etapa final.

---

## Plan por fases

### Fase 0 — Congelar el oráculo Bison y la línea base

- **Objetivo:** capturar el comportamiento autoritativo de Bison (conflictos + AST dumps)
  antes de tocar nada, para diff continuo.
- **Archivos:** `doc/parser/expected_conflicts.txt`, `tests/parser/run_parser_golden.sh`,
  corpus golden bajo `tests/parser/golden/`.
- **Trabajo:**
  - `bison --report=all -Wcounterexamples` sobre `grammar.y` → guardar `parser.output` y
    extraer la lista exacta de conflictos y resoluciones a `expected_conflicts.txt`.
  - Generar AST dumps golden con el `hulk_parser_demo` **actual** sobre TODO el corpus
    `.hulk` (`tests/parser`, `tests/eval`, `tests/semantic`, `tests/typecheck`,
    `tests/backend`, `tests/hulk`, `examples`). Estos dumps son la "tabla de verdad".
  - Script de comparación golden (parser nuevo vs dump committeado).
- **Riesgos:** el dump del demo **no imprime spans**; los spans se validan aparte (vía un
  diff de diagnósticos cuya posición depende del span). Mitigación: extender temporalmente el
  demo para volcar spans, o validar spans con casos de error dedicados.
- **Pruebas:** `make parser-demo`; el corpus golden queda fijado y reproducible.

### Fase 1 — Motor runtime + bridge de tokens (con tablas mínimas de prueba)

- **Objetivo:** construir la nueva clase `Parser` table-driven, el `Symbol`/`ParserValue` y
  el `yylex` nuevo, validados con una mini-tabla hecha a mano (aún sin generador).
- **Archivos:** `src/parser/symbol.hpp`, `src/parser/parser_semantic_types.hpp`,
  `src/parser/lr_engine.hpp/.cpp`, `src/parser/parser.hpp` (nueva clase),
  `parser_lexer_adapter.cpp` (modificado).
- **Trabajo:**
  - Definir `Symbol` y `ParserValue` (variant con los ~19 tipos de §2).
  - Implementar el bucle shift/reduce/goto/accept genérico que consume `ACTION/GOTO`
    (parametrizado por las tablas), con value-stack + location-stack paralelos.
  - Implementar el **merge de spans** `@$`/`@n` (primero..último; vacío = lookahead).
  - Reescribir `yylex` para devolver `Symbol` (mapa `TokenKind`→id; mantener decode de
    nº/strings + diagnósticos verbatim; literal inválido → símbolo de error centinela).
  - Cablear `Parser::error`/abort a `driver.report_syntax_error`; `accept` de `program`
    llama a `driver.set_result`.
- **Riesgos:** correcto manejo de tipos move-only en el variant; exactitud del merge de spans;
  off-by-one en índices del stack (`base+n-1`).
- **Pruebas:** compilación; **unit test del engine** con una gramática de expresiones mínima
  (tabla a mano) verificando shift/reduce/goto/accept, merge de spans y construcción de valores.

### Fase 2 — Generador LALR(1) self-hosted (el núcleo)

- **Objetivo:** la herramienta C++ que lee `hulk.grammar` y emite `parser_tables.*`.
- **Archivos:** `tools/parsergen/*`, `src/parser/hulk.grammar`.
- **Trabajo:** implementar §3 completo (aumentada, FIRST, ítems LR(1), merge LALR, ACTION/GOTO,
  resolución de conflictos idéntica a Bison, emisor del switch con sustitución `$/@`).
  Transcribir `grammar.y` → `hulk.grammar` (producciones + precedencias + acciones verbatim).
- **Riesgos (la parte más difícil):** corrección del cálculo de lookaheads y del merge de
  cores; que la **sustitución `$n`→`std::get<Tn>`** use exactamente el `%type` declarado (si
  no, no compila o castea mal); reproducir la resolución de conflictos de Bison al pie de la
  letra.
- **Pruebas:** el generador imprime su reporte de conflictos y se compara con
  `expected_conflicts.txt`; unit tests de FIRST sets y de 2–3 estados conocidos contra el
  `parser.output` de Bison.

### Fase 3 — Integración y build dual (A/B contra Bison)

- **Objetivo:** cablear `parser_tables` al engine; construir todos los binarios con el parser
  nuevo; conservar Bison tras `USE_BISON=1` para comparación.
- **Archivos:** [Makefile](Makefile) (target `parser-gen-own`, `PARSER_OBJS`,
  `parser-sync-check-own`, gating de Bison).
- **Trabajo:** Makefile como en §8; regenerar tablas; compilar `hulk_parser_demo`,
  `hulk_eval`, `hulk_semantic`, `hulk_backend` con el nuevo `PARSER_OBJS`.
- **Riesgos:** los 4 binarios comparten `PARSER_OBJS` (un cambio se propaga a todos);
  frescura del generado vs committeado.
- **Pruebas:** `make parser-demo`; **A/B**: correr demo nuevo vs Bison sobre el corpus golden
  de Fase 0 → diff cero.

### Fase 4 — Paridad total + casos extra

- **Objetivo:** AST dumps **byte-idénticos** vs el oráculo en todo el corpus + casos extra.
- **Archivos:** nuevos `tests/parser/*.hulk` para los casos extra; ajustes finos en
  `lr_engine`/spec si aparece algún diff.
- **Trabajo:** correr todas las suites; cerrar cualquier diferencia. Añadir casos:
  `-2 ^ 2`, `2 ^ 3 ^ 2`, `1 < 2 < 3`, `1 == 2 == 3`, `foo.bar.baz(1)`, `foo()(1)`,
  múltiples expresiones globales, string con escape inválido, número fuera de rango.
- **Riesgos:** diferencias sutiles de span en producciones vacías / EOF; que `foo()(1)` y los
  dos errores de expresión global produzcan el **mismo** mensaje y span.
- **Pruebas:** `make parser-demo parser-tests run-tests backend-tests end-to-end-tests
  hulk-tests` en verde; A/B vs oráculo diff cero; casos extra con AST/errores esperados.

### Fase 5 — Corte de Bison y documentación

- **Objetivo:** eliminar la dependencia de Bison del camino por defecto y actualizar docs.
- **Archivos:** retirar `src/parser/parser.cpp`/`location.hh`; decidir destino de `grammar.y`;
  `CLAUDE.md`, `doc/gramatica_atributada_hulk.md`, `tests/parser/README.md`, `REPORT.md`.
- **Trabajo:** quitar `parser.o`/`parser-gen`/`parser-sync-check` (o dejarlos solo bajo
  `USE_BISON=1` por **un release** como red de seguridad, luego borrar). Actualizar docs:
  build sin Bison, nuevo `parser-gen-own`, descripción del generador propio.
- **Riesgos:** romper `parser-sync-check` (debe retirarse); olvidar referencias a Bison en docs.
- **Pruebas:** build limpio sin `bison` instalado; todas las suites verdes.

---

## Riesgos globales

- **Lookaheads LALR mal calculados** → estados/acciones distintos a Bison. Mitigación:
  construcción canónica LR(1) + merge (la más verificable) y diff contra `parser.output`.
- **Spans divergentes** en reglas vacías / EOF → cambian posiciones de diagnósticos y AST.
  Mitigación: replicar el merge exacto de Bison y tests de error con span.
- **Sustitución `$/@` y tipos del variant** → errores de compilación o casteos erróneos si
  el `%type` no coincide. Mitigación: el generador valida tipos contra la tabla `%type`.
- **Resolución de conflictos** distinta a Bison en el punto `type_expr STAR`. Mitigación:
  baseline `expected_conflicts.txt` + fallo de build si difiere.
- **Decisiones que pueden cambiar comportamiento sin querer:** "limpiar" la gramática,
  cambiar la asociatividad implícita por recursión, o normalizar spans. **Todo eso queda
  fuera** de la primera migración.
- **Dejar fuera de la primera migración:** recuperación de errores (`error` rules), mensajes
  de error "esperaba X" más ricos que los de Bison, compresión de tablas, y cualquier
  refactor de la gramática.

---

## Verificación end-to-end

1. **Oráculo (Fase 0):** `bison --report=all` + AST dumps golden committeados.
2. **A/B continuo:** `tests/parser/run_parser_golden.sh` corre el demo nuevo y hace diff
   contra el golden; objetivo **diff cero** en todo el corpus.
3. **Suites del repo (deben quedar idénticas):**
   `make parser-demo`, `make parser-tests`, `make run-tests`, `make backend-tests`,
   `make end-to-end-tests`, `make hulk-tests`.
4. **Casos de aceptación extra** (AST/errores esperados):
   `-2 ^ 2`→`(-2)^2`; `2 ^ 3 ^ 2`→`2^(3^2)`; `1 < 2 < 3`→`(1<2)<3`;
   `1 == 2 == 3`→`(1==2)==3`; `foo.bar.baz(1)`→`MethodCall`; `foo()(1)`→parsea + error
   `"solo se pueden invocar identificadores o accesos a metodo"`; múltiples expr globales →
   `"Solo se permite una expresion global final"`; escape inválido →
   `"Escape de string no soportado: \X"`; número fuera de rango →
   `"Literal numerico fuera de rango"`.
5. **Build sin Bison instalado** (Fase 5) compila y pasa todas las suites.

Criterio de aceptación global: **todas las suites verdes + A/B diff cero vs oráculo + casos
extra correctos**, sin cambios en `eval`/`semantic`/`backend`.

---

## Recomendación de orden de implementación

1. **Fase 0** — congelar oráculo (conflictos + AST golden). *Sin red, no hay migración segura.*
2. **Fase 1** — motor + `Symbol`/variant + `yylex` nuevo, validados con tabla mínima a mano.
3. **Fase 2** — generador LALR(1) self-hosted + `hulk.grammar` (el grueso del trabajo).
4. **Fase 3** — integrar tablas, build dual, A/B contra Bison.
5. **Fase 4** — cerrar a paridad byte-idéntica + casos extra.
6. **Fase 5** — cortar Bison y actualizar docs.

Hito mínimo demostrable lo antes posible: al final de **Fase 3**, `hulk_parser_demo` con el
parser propio produce AST **idéntico** a Bison en el corpus golden. A partir de ahí, todo es
cierre de diferencias y limpieza.
