# Compilador HULK — Reporte de Arquitectura y Diseño

## 1. Introducción

HULK (*Havana University Language for Kompilers*) es un lenguaje didáctico de tipado estático con inferencia de tipos opcional, orientado a objetos y con características funcionales. Este reporte describe la arquitectura del compilador, las decisiones de diseño más relevantes, los features implementados y las limitaciones notables del proyecto.

El compilador traduce archivos fuente `.hulk` a través de una pipeline de múltiples etapas que culmina en la ejecución sobre una máquina virtual de pila propia llamada *BannerVM*. Paralelamente existe un evaluador de árbol que sirve como camino de ejecución alternativo, utilizado para pruebas en etapas tempranas del desarrollo y como oráculo de referencia diferencial. Ambos caminos comparten todas las fases anteriores al backend (lexer, parser, semántica) y divergen únicamente en la etapa de ejecución.

---

## 2. Pipeline de compilación

El pipeline corre en ocho fases secuenciales. Cada fase consume la salida de la anterior y produce una nueva representación; ninguna fase muta estructuras producidas por etapas anteriores.

### 2.1 Lexer

El lexer está escrito a mano en `src/lexer/`. Lee la cadena fuente carácter a carácter y produce una lista plana de tokens. Cada token lleva su `kind`, el lexema original y un `Span` que registra la línea y columna de inicio y fin en el archivo fuente. Ese span se propaga a través de todas las fases posteriores para que los mensajes de error puedan señalar la ubicación exacta en el código.

La recuperación de errores está integrada: en lugar de lanzar una excepción ante un carácter inválido, el lexer registra un diagnóstico en el `DiagnosticEngine` y emite un token de clase error. El parser puede saltarlo y continuar, permitiendo reportar múltiples problemas en una sola pasada.

### 2.2 Parser

El parser es LALR(1) y se genera con una herramienta propia en C++ (`tools/parsergen/`) a partir de `src/parser/hulk.grammar`. El generador construye la coleccion canonica LR(1), fusiona estados con el mismo core LR(0) para obtener LALR(1), resuelve conflictos con la misma politica que Bison y emite `src/parser/parser_tables.cpp/.hpp`.

En runtime, `src/parser/lr_engine.cpp` ejecuta esas tablas con un motor shift/reduce generico. `ParserDriver` conserva el mismo contrato publico: toma tokens del lexer, reporta diagnosticos y recibe el AST final. `parser_lexer_adapter.cpp` traduce `TokenKind` al `Symbol` del parser, preservando la validacion de literales numericos y strings. Las acciones semanticas de la gramatica se copian al archivo `.grammar` y el generador las transforma en un `switch` C++ que construye el AST.

### 2.3 Árbol de Sintaxis Abstracta (AST)

El AST está definido en `src/ast/` y contiene aproximadamente 50 tipos de nodos agrupados por categoría: literales, operadores binarios, condicionales, bucles, bindings, declaraciones de tipos, de protocolos y de funciones. Hay tres clases base abstractas: `ASTNode`, `Expr` (cualquier cosa que produce un valor) y `Decl` (funciones, tipos, protocolos). Un nodo `Program` contiene una lista de `Decl` más la expresión global final.

Cada clase de nodo concreta implementa `accept(Visitor&)`, habilitando el **patrón Visitor** en todas las fases subsiguientes. 

### 2.4 Análisis semántico

El análisis semántico es orquestado por `SemanticAnalyzer` (`src/semantic/`) y corre en tres sub-fases.

**Binding** (`src/binding/`) resuelve cada referencia a nombre — variables, llamadas a funciones, nombres de tipos, llamadas a métodos — hacia su declaración. Corre tres pases: el primero registra todas las declaraciones de nivel superior en las tablas semánticas, permitiendo que tipos y funciones mutuamente recursivos funcionen correctamente; el segundo recorre todas las expresiones y guarda la resolución en un `resolution_map` indexado por puntero al nodo AST; el tercero valida ciclos de herencia, compatibilidad de firmas en overrides, aridad de llamadas a funciones y corrección de extensiones de protocolo.

**Inferencia de tipos** (`src/inference/`) determina el tipo de cada expresión mediante iteración de punto fijo. El sistema de tipos tiene siete kinds: `Number`, `String`, `Boolean`, `Void`, `Object`, `Unknown` y `Error`. `Error` se propaga — una vez que una sub-expresión es marcada como errónea, sus consumidores también lo son, evitando cascadas de errores. Para expresiones `if`, el tipo inferido es el **Ancestro Común Más Bajo** (LCA) de las ramas then y else en la jerarquía de herencia.

**Type checking** (`src/typecheck/`) valida la conformancia (`T1 <= T2`) en todo el árbol anotado: tipos de argumentos en sitios de llamada, tipos de operandos en operadores built-in, compatibilidad de asignación en bindings, y validez de casts con `as`.

Las tres sub-fases producen *mapas laterales* — `unordered_map<Expr*, X>` — en lugar de anotar los nodos del AST directamente. Esto mantiene el AST como un registro puro de lo que escribió el programador, independiente de cualquier resultado de análisis.

### 2.5 Evaluador de árbol (Camino A)

`src/eval/` contiene un evaluador basado en el patrón Visitor que ejecuta programas directamente desde el AST anotado. Cada método `visit` computa un `HulkValue` (unión etiquetada de `Number | Bool | String | Object | Nil`) y lo deposita en un slot de resultado. El scoping de variables usa una cadena de `Environment`: cada binding `let` empuja un environment hijo; al salir del binding se hace pop.

Este evaluador fue el primer camino de ejecución funcional, utilizado antes de que existiera el backend. Hoy sirve principalmente como oráculo de referencia.

### 2.6 Generación de IR (Camino B)

`src/backend/ir_gen.cpp` es un segundo visitor del AST que traduce el árbol anotado a **HulkIR**, una representación intermedia de nivel medio definida en `src/ir/`. HulkIR es un formato de código de tres direcciones que todavía conoce los tipos, los nombres de campos y la jerarquía de herencia. Los temporales tienen nombres de string (`add_3`, `tmp_call_7`) en lugar de registros numéricos; la asignación de slots ocurre más tarde en la VM.

Decisiones clave de lowering en esta etapa: los bucles `for` se desazucarean a bucles `while` sobre un iterador siguiendo el protocolo `Iterable`; las expresiones `new` emiten una instrucción `NewObject` seguida de una llamada al constructor.

Un `NameMangler` produce etiquetas únicas para métodos sobreescritos (`Dog__speak` vs. `Animal__speak`) de modo que cada función en el namespace plano del BannerIR tenga un identificador único.

### 2.7 Lowering a BannerIR

`src/backend/hulkir_to_banner.cpp` convierte HulkIR a **BannerIR** , un IR de bajo nivel, (`src/banner/`)y un formato linealizado con tres secciones:

- `.TYPES` — layouts aplanados de clases, incluyendo todos los atributos heredados y una vtable que mapea números de slot de método a etiquetas de función.
- `.DATA` — pool estático de strings.
- `.CODE` — funciones de nivel superior con declaraciones explícitas de `PARAM` / `LOCAL` seguidas de instrucciones de tres direcciones.

El cambio estructural principal respecto a HulkIR es la introducción de instrucciones `PARAM` explícitas antes de cada `CALL` o `VCALL`, respetando la convención de llamada de arquitecturas reales y simplificando el loop de ejecución de la VM.

### 2.8 BannerVM

La VM (`src/vm/`) compila el `BannerProgram` antes de ejecutarlo. El paso de compilación asigna IDs numéricos a los tipos, distribuye los campos de objetos en arrays de slots planos (garantizando que los campos heredados ocupen el mismo índice independientemente del tipo dinámico, lo que permite acceso O(1) por índice), y convierte los nombres de temporales a índices numéricos de frame.

La ejecución mantiene un `vector<Frame>` como pila de llamadas. Cada frame tiene un program counter, un array de slots para locales y temporales, un buffer de parámetros para la próxima llamada y un índice de slot de retorno. El loop principal despacha sobre `BannerInstruction::op` y avanza el contador o salta a una dirección de etiqueta.

Un **recolector de basura mark-and-sweep** (`src/vm/vm_heap.h`) administra objetos y strings alojados en el heap. Se activa periódicamente según el número de alocaciones, marca todos los objetos alcanzables desde los frames vivos y libera el resto. Handles generacionales detectan referencias colgantes sin requerir un recorrido completo en cada acceso.

Límites de seguridad configurables mediante `VMOptions` — profundidad máxima de pila, número máximo de instrucciones, tamaño máximo del heap — impiden que recursión infinita o bucles sin fin cuelguen el proceso.

---


## 3. Features implementados

El compilador implementa los features del lenguaje HULK conforme a los apéndices A.2 a A.11 de la especificación oficial:

**A.2 — Expresiones.** El lenguaje es completamente orientado a expresiones: toda construcción (condicionales, bucles, bloques, let-bindings) produce un valor. Se implementan los operadores aritméticos (`+`, `-`, `*`, `/`, `%`, `^`), concatenación de strings (`@`, `@@`), comparación, lógica (`&`, `|`, `!`), asignación destructiva (`:=`) y los literales `Number`, `String`, `Boolean` y `Nil`. Las funciones y constantes builtin están disponibles: `print`, `sqrt`, `sin`, `cos`, `exp`, `log`, `rand`, `PI`, `E`.

**A.3 — Funciones.** Se soportan funciones globales con sintaxis inline (`function f(x) => expr`) y de bloque (`function f(x) { ... }`). Las funciones pueden ser mutuamente recursivas gracias al pase de registro previo al binding. La aridad de las llamadas se valida en tiempo de compilación.

**A.4 — Variables.** El binding permite múltiples bindings simultáneos con sombreado de nombres del scope externo. La asignación destructiva `:=` permite mutar una variable ya vinculada dentro de su scope.

**A.5 — Condicionales.** La construcción `if (cond) e elif (cond) e else e` está completamente implementada. El tipo del resultado es el LCA de las ramas, garantizando que el condicional siempre produzca un tipo concreto compatible con ambos caminos.

**A.6 — Bucles.** Se implementan `while (cond) body` y `for (x in iter) body`. El bucle `for` es azúcar sintáctica que el compilador desazucarea a un `while` sobre un iterador que sigue el protocolo `Iterable`, conforme a la especificación.

**A.7 — Tipos nominales.** Las declaraciones `type` crean tipos nominales con herencia simple (`inherits`). Se soportan constructores con parámetros, inicialización de atributos, referencia a la instancia actual con `self`, llamadas al método del padre con `base()`, test de tipo dinámico con `is` y downcast explícito con `as`.

**A.8 — Type checking.** El type checker valida la conformancia `T1 <= T2` en todo el árbol anotado: argumentos en sitios de llamada, operandos de operadores built-in, compatibilidad en asignaciones y casts. La relación de conformancia sigue la jerarquía de herencia; para protocolos, verifica que el tipo tenga los métodos requeridos con firmas compatibles.

**A.9 — Inferencia de tipos.** El inferidor de punto fijo determina el tipo de cada expresión en hasta diez iteraciones. Las anotaciones de tipo son opcionales en todos los lugares del lenguaje; el inferidor las completa cuando se omiten. Si la inferencia falla para algún símbolo, el programa no compila.

**A.10 — Protocolos.** Las declaraciones `protocol … extends …` definen interfaces estructurales. La conformancia es puramente estructural: un tipo satisface un protocolo si provee los métodos requeridos con firmas compatibles, sin ninguna declaración explícita de implementación. Los protocolos son verificados en tiempo de compilación y borrados antes del backend.

**A.11 — Iterables.** El protocolo `Iterable` está integrado como builtin del compilador. Los tipos que implementan `next(): Boolean` y `current(): T` son reconocidos como iterables. El bucle `for` se compila a través de este protocolo. La sintaxis `T*` (iterable de T) y `T[]` (vector de T) como anotaciones de tipo está soportada en la gramática.

---

## 4. Feature adicional: inferencia explícita con `_` y `auto`

Más allá de la especificación base, el compilador implementa **type holes explícitos** — una extensión sintáctica que permite a los programadores marcar posiciones específicas con `_` o `auto` para solicitar inferencia de tipos del compilador en ese punto concreto.

**Sintaxis.** La gramática extiende `type_ann` con dos nuevas alternativas:

```
type_ann ::= ... | '_' | 'auto'
```

Ambas son semánticamente idénticas; `_` es más compacto y se lee como un hueco, mientras que `auto` es más descriptivo. Son válidas en todas las posiciones de anotación: tipos de bindings `let`, tipos de parámetros de función, tipos de retorno, parámetros y retornos de métodos, y anotaciones de tipo de atributos.

**Semántica.** Cada `_` o `auto` introduce un *hueco de tipo* — un nodo `InferTypeAnnotation` en el AST. Durante la fase de inferencia, los huecos se tratan como variables de tipo frescas. El inferidor recolecta restricciones del entorno de la expresión (por ejemplo, `x + 1` restringe el hueco de `x` a `Number`; `!x` lo restringe a `Boolean`; `x * x` restringe tanto el parámetro como el tipo de retorno a `Number`) y resuelve cada hueco al único tipo concreto consistente con todas sus restricciones.

Si un hueco tiene una única resolución válida, se sustituye y el type checker recibe un árbol completamente anotado, equivalente a uno donde el programador hubiera escrito el tipo inferido explícitamente. Si un hueco no tiene resolución posible (falta de información) o si dos restricciones sobre el mismo hueco son incompatibles (por ejemplo, una variable usada en contexto aritmético y booleano a la vez), el compilador emite un error descriptivo con las restricciones en conflicto y una sugerencia de anotación explícita.

**Interacción con `--restricted-inference`.** Cuando esta bandera está activa, omitir una anotación de tipo es un error de compilación; solo `_`, `auto` o un nombre de tipo explícito son aceptables. Esto hace ejecutable en tiempo de compilación la distinción entre "el programador olvidó anotar" y "el programador pidió deliberadamente inferencia".

---

## 5. Limitaciones notables

**GC síncrono.** El recolector mark-and-sweep corre sincrónicamente durante la alocación. Para programas con heaps grandes o alocación frecuente, esto puede introducir pausas observables. Un recolector incremental o generacional mitigaría esto, pero está fuera del alcance de la implementación actual.

**Mensajes de error sin recuperación cross-fase.** Si el binding encuentra errores, las fases de inferencia y type checking continúan sobre el árbol parcialmente resuelto, pero los diagnósticos producidos en esas fases posteriores pueden referirse a nodos cuya resolución fue marcada como `Unresolved`. Esto genera en ocasiones errores redundantes o mensajes confusos que apuntan a consecuencias del error original en lugar de a su causa raíz. Un sistema de supresión de errores derivados — que silencie automáticamente los diagnósticos cuyo origen sea un nodo ya reportado — reduciría el ruido en la salida de error.

---

## 6. Conclusión

El compilador HULK implementa una pipeline completa desde el texto fuente hasta la ejecución en una máquina virtual, cubriendo análisis léxico, parsing LALR(1), análisis semántico en tres fases (binding, inferencia de punto fijo y type checking), generación de IR, lowering y una VM de pila con recolector de basura. Los principios de diseño centrales — patrón Visitor para el recorrido del AST, nodos inmutables con mapas laterales de anotación, borrado de tipos tras la validación semántica y dos caminos de ejecución paralelos — dan como resultado una base de código donde cada capa tiene una responsabilidad única y clara, y cada fase puede probarse de forma independiente.

El feature adicional de inferencia explícita con `_` y `auto` extiende el lenguaje con type holes que permiten a los programadores señalar exactamente dónde quieren delegar el tipo al compilador, con diagnósticos precisos cuando la inferencia es ambigua. Junto con la bandera `--restricted-inference`, ofrece un espectro de requerimientos de anotación aplicables a cualquier programa HULK.
