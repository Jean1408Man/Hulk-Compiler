# Vulnerabilidades actuales del flujo end-to-end

Este documento lista vulnerabilidades tecnicas, riesgos y deuda del flujo actual:

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

Aqui "vulnerabilidad" no significa solamente seguridad externa. Tambien incluye puntos donde el compilador puede aceptar programas que no deberia, donde la VM puede crecer memoria sin limite, donde el backend depende de fallbacks de alto nivel, o donde un error interno se reporta con poca informacion.

El backend actual ya cumple el objetivo central de ejecutar una IR propia en una VM propia. Los puntos siguientes son lo que falta endurecer para hacerlo mas robusto.

## 1. Tolerancias semanticas en el BackendDriver

### Que falla

`BackendDriver` ignora algunos diagnosticos semanticos antes de generar IR:

```cpp
if (diagnostic.message == "Los atributos son privados. Solo se pueden acceder mediante 'self'.") {
    continue;
}
if (diagnostic.message.find("'is' no plausible:") != std::string::npos) {
    continue;
}
```

Esto significa que el backend puede seguir compilando aunque el analizador haya reportado errores.

### Por que falla

Esta tolerancia se agrego para mantener compatibilidad con tests o comportamiento previo mientras el backend maduraba. El problema es que rompe una regla importante del pipeline:

```text
si semantica reporta error, el backend no deberia generar ni ejecutar codigo
```

Cuando se ignora un error, `IRGen`, `HulkIRToBanner` o `BannerVM` pueden recibir un programa que no cumple las garantias semanticas esperadas.

### Que es necesario hacer

Separar errores bloqueantes de warnings de forma formal. No se debe filtrar por texto exacto del mensaje.

### Como se haria

1. Agregar codigos estables de diagnostico o categorias en `Diagnostic`.
2. Marcar cada diagnostico como bloqueante o no bloqueante.
3. Cambiar `BackendDriver` para que no compare strings.
4. Actualizar tests que dependan de aceptar esos errores.

Ejemplo esperado:

```cpp
if (engine.has_blocking_errors()) {
    engine.print_all();
    return result;
}
```

### Explicacion

Comparar mensajes humanos es fragil. Si cambia el texto del error, el backend puede cambiar de comportamiento sin que nadie toque logica real. Lo correcto es que el diagnostico tenga identidad propia, por ejemplo `DiagnosticCode::PrivateAttributeAccess`.

## 2. Acceso privado a atributos tolerado

### Que falla

El mensaje:

```text
Los atributos son privados. Solo se pueden acceder mediante 'self'.
```

se ignora en el backend. Eso permite que ciertos accesos a atributos pasen hacia IR/VM aunque semantica los haya marcado como invalidos.

### Por que falla

La VM implementa objetos como:

```text
object_ref -> VMObject { type_id, fields[] }
```

La VM no conoce reglas de privacidad del lenguaje. Solo sabe ejecutar `GETATTR` y `SETATTR`.

Si el backend deja pasar un acceso privado, la VM lo ejecuta porque para la VM es solo:

```text
fields[slot]
```

### Que es necesario hacer

La regla de privacidad debe cerrarse en semantica/typecheck, antes de `IRGen`.

### Como se haria

1. Quitar la excepcion del `BackendDriver`.
2. Asegurar que tests C6 validos usen metodos publicos para acceder a campos.
3. Si el lenguaje quiere permitir acceso a campos desde fuera, cambiar la regla semantica y documentarla.
4. Si la regla es privacidad estricta, agregar tests negativos de backend.

### Explicacion

La VM no debe decidir si un acceso es privado o publico. Esa informacion pertenece al lenguaje fuente. En una VM baja, `GETATTR` no tiene contexto suficiente para saber si el programa fuente escribio `self.x` o `obj.x`.

## 3. `is` no plausible se tolera parcialmente

### Que falla

El backend ignora diagnosticos cuyo mensaje contiene:

```text
'is' no plausible:
```

Eso permite ejecutar algunas comprobaciones `is` que el type checker considera imposibles.

### Por que falla

La VM tiene `IS_TYPE`, y puede responder `false` en runtime. Pero el type checker intenta rechazar comparaciones de tipos que no tienen relacion posible.

Ejemplo conceptual:

```hulk
3 is String
```

Semantica puede decidir que eso no tiene sentido y reportarlo. Si el backend lo ignora, la VM simplemente devuelve `false`.

### Que es necesario hacer

Definir la politica del lenguaje:

- politica estricta: `is` no plausible es error y bloquea backend;
- politica permisiva: `is` no plausible compila y devuelve `false`.

### Como se haria

Si se elige politica estricta:

1. Quitar la excepcion del `BackendDriver`.
2. Mantener el error de type checker como bloqueante.
3. Actualizar tests esperados.

Si se elige politica permisiva:

1. Cambiar el type checker para que no reporte error en esos casos.
2. Documentar que `is` siempre es permitido.
3. Mantener `IS_TYPE` en VM como operacion runtime.

### Explicacion

El problema no es tecnico, es de especificacion. Ahora el proyecto esta en un punto intermedio: semantica lo reporta como error, pero backend lo tolera. Esa inconsistencia es riesgosa.

## 4. Falta GC: heap con vida de programa completa

### Que falla

`VMHeap` guarda objetos y strings en vectores:

```cpp
std::vector<VMObject> objects_;
std::vector<VMString> strings_;
```

Durante la ejecucion no se libera nada individualmente. Todo queda vivo hasta que termina `BannerVM::run`.

### Por que falla

El modelo actual es una arena de vida completa:

```text
allocate object/string
allocate object/string
allocate object/string
no free
al final: liberar todo
```

Esto es simple y correcto para programas pequenos. Pero un programa largo puede consumir memoria sin limite.

Ejemplo problematico:

```hulk
let s = "" in {
  while (true) {
    s := s @@ "x";
  }
}
```

Cada `CONCAT` crea un string nuevo. Los strings viejos quedan en heap aunque ya no sean alcanzables.

### Que es necesario hacer

Implementar un recolector de basura mark-and-sweep.

### Como se haria

1. Agregar metadata de marcado a objetos y strings:

```cpp
struct VMObject {
    int type_id;
    std::vector<Word> fields;
    bool marked = false;
};

struct VMString {
    std::string value;
    bool marked = false;
};
```

2. Exponer una funcion `VMHeap::collect(...)`.

3. Pasar como raices:

- slots de todos los frames activos;
- `param_buffer` de todos los frames;
- `CompiledProgram::data`, porque strings de `.DATA` deben seguir vivos mientras corre el programa;
- cualquier valor temporal que la VM mantenga fuera de frames.

4. Fase mark:

```text
para cada Word raiz:
  si es string_ref: marcar string
  si es object_ref: marcar objeto y recorrer sus fields
```

5. Fase sweep:

```text
para cada objeto/string:
  si no esta marcado: liberar o poner en free list
  si esta marcado: desmarcar para la siguiente ronda
```

6. Como los handles actuales son indices a vector, no conviene borrar elementos y mover indices. Hay que usar una free list:

```text
objects_[i] puede quedar vacio
allocate_object reutiliza indices libres
object_ref(i) sigue siendo estable
```

7. Disparar GC:

- cuando se supera un umbral de objetos/strings;
- o despues de N allocaciones;
- o manualmente en modo debug.

### Explicacion

Mark-and-sweep es adecuado para esta VM porque los valores son `Word` y todas las referencias a heap tienen tags (`string_ref`, `object_ref`). Eso permite recorrer raices y distinguir referencias de numeros o booleanos.

La parte delicada es no invalidar handles. Como `Word` guarda indices, si se borran elementos de `objects_` y el vector compacta, todos los `object_ref` existentes quedan apuntando al lugar incorrecto. Por eso se recomienda free list o slots opcionales, no compactacion inicial.

## 5. No hay proteccion contra crecimiento de stack VM

### Que falla

La recursion HULK usa frames propios de VM, lo cual es correcto. Pero no hay limite explicito de profundidad.

Un programa como:

```hulk
function f(x) => f(x);
print(f(0));
```

puede hacer crecer `stack` hasta agotar memoria.

### Por que falla

`BannerVM::run` usa:

```cpp
std::vector<Frame> stack;
```

Cada `CALL`, `VCALL` o `SCALL` puede hacer `push_back(make_frame(...))`.

### Que es necesario hacer

Agregar limites configurables de ejecucion:

- max frames;
- max instrucciones ejecutadas;
- max heap allocations;
- max heap bytes aproximados.

### Como se haria

1. Crear `VMOptions`:

```cpp
struct VMOptions {
    std::size_t max_frames = 100000;
    std::uint64_t max_steps = 10000000;
    std::size_t max_heap_objects = 1000000;
};
```

2. Agregar `BannerVM::run(program, options)`.
3. Incrementar `steps` en cada instruccion.
4. Antes de `stack.push_back`, validar `stack.size()`.
5. En `VMHeap`, validar allocaciones o disparar GC.

### Explicacion

Un backend robusto debe fallar con un error controlado, no dejar que el proceso host consuma memoria hasta morir. Esto importa aunque el lenguaje permita recursion infinita.

## 6. BannerIR textual conserva demasiados simbolos

### Que falla

BannerIR imprimible conserva:

- nombres de locales;
- nombres de labels;
- nombres de campos;
- nombres de metodos;
- nombres de funciones;
- nombres de tipos.

Esto es bueno para debug, pero la forma textual no representa completamente la forma baja que ejecuta la VM.

### Por que falla

La VM compila internamente a `CompiledProgram`, donde muchos nombres se vuelven indices. Pero ese paso no se refleja completamente en `--emit-banner`.

### Que es necesario hacer

Decidir si `--emit-banner` debe imprimir:

1. BannerIR simbolica legible; o
2. BannerIR baja ya indexada; o
3. ambas vistas.

### Como se haria

Opcion recomendada: dos modos.

```text
--emit-banner          imprime BannerIR legible
--emit-banner-compiled imprime BannerIR indexada/slots
```

La vista compilada mostraria:

```text
function #0 hulk_main
  slot 0 hulk_tmp_number_0
  slot 1 hulk_tmp_number_1
  0: CONST_NUMBER s0, 2
  1: ADD s2, s0, s1
```

### Explicacion

La IR legible ayuda a humanos. La IR indexada ayuda a demostrar que la VM ejecuta bajo nivel. Tener ambas evita sacrificar depuracion.


## 7. Errores runtime sin span ni contexto de instruccion

### Que falla

Los errores runtime dicen cosas como:

```text
Runtime error: se esperaba Object.
Runtime error: division por cero.
Runtime error: slot local no encontrado.
```

Pero no incluyen:

- archivo fuente;
- linea/columna;
- funcion actual;
- pc;
- instruccion BannerIR;
- stack trace VM.

### Por que falla

La IR no conserva spans del AST. La VM solo tiene instrucciones y nombres internos.

### Que es necesario hacer

Propagar metadata de origen.

### Como se haria

1. Agregar `SourceSpan` opcional a `IRInstr`.
2. Copiarlo a `BannerInstr`.
3. Copiarlo a `CompiledInstr`.
4. En errores runtime, imprimir:

```text
Runtime error en hulk_main pc=14
source: tests/eval/foo.hulk:3:12
instr: div s2 s0 s1
causa: division por cero
```

5. Agregar stack trace de frames:

```text
at hulk_fn_factorial_0 pc=8
at hulk_main pc=4
```

### Explicacion

Sin contexto, depurar fallos runtime exige mirar IR manualmente. Para usuarios, el error debe volver al programa fuente.

## 8. Tests no cubren invariantes internas de VM

### Que falla

Los tests actuales verifican salidas end-to-end y snapshots IR. Eso es bueno, pero no prueban directamente invariantes internas:

- `Word` tags;
- heap handles invalidos;
- strings dinamicos;
- dispatch por slot;
- fallback por nombre;
- limite de aridad;
- errores runtime.

### Por que falla

El runner prueba programas HULK, no unidades internas de VM.

### Que es necesario hacer

Agregar tests unitarios o smoke tests especificos de VM.

### Como se haria

1. Crear target `vm-tests`.
2. Testear `vm_value`:

```text
make_number/as_number
make_bool/as_bool
make_string_ref/as_string_index
make_object_ref/as_object_index
NaN
```

3. Testear `VMHeap`:

```text
allocate_string
allocate_object
invalid handles
field storage
```

4. Testear `BannerVM` con `BannerProgram` construido en C++:

- programa minimo;
- llamada;
- recursion pequena;
- objeto con campo;
- vcall.

### Explicacion

Los tests end-to-end pueden pasar aunque una invariante interna este fragil. Los tests unitarios ayudan a aislar errores cuando se cambie `Word`, GC o slots.

## 9. Falta implementar `restricted-inference` en el chequeo semantico

### Que falla

El reporte de inferencia propone un modo restringido donde la inferencia solo se permite cuando el programador la pide explicitamente con `_` o `auto`.

Actualmente el flujo semantico acepta inferencia implicita normal de HULK y tambien acepta `_`/`auto` como type holes, pero no existe una opcion tipo:

```bash
./hulk_backend file.hulk --restricted-inference
```

En otras palabras, estos dos estilos pueden convivir sin distincion:

```hulk
let x = 42 in print(x);
let y: _ = 42 in print(y);
```

En modo normal eso esta bien. En modo restringido, el primer caso deberia fallar porque el programador omitio el tipo sin pedir inferencia explicita.

### Por que falla

El parser ya conserva suficiente informacion basica para saber si una declaracion tenia anotacion o no:

```cpp
binding->HasTypeAnnotation()
param.HasTypeAnnotation()
method->HasReturnTypeAnnotation()
```

Pero el `SemanticAnalyzer`, `TypeInferencer` y `TypeChecker` no reciben una politica de inferencia. Para ellos, una anotacion ausente y una anotacion inferible son parte del flujo normal.

Eso deja incompleta la extension descrita en el reporte: `_` y `auto` existen, pero no se puede activar el modo donde son obligatorios para inferir.

### Que es necesario hacer

Agregar una opcion semantica que distinga tres situaciones:

```text
tipo explicito       -> permitido siempre
tipo _ / auto        -> permitido siempre y resuelto por inferencia
tipo omitido         -> permitido en modo normal, error en modo restringido
```

El objetivo no es cambiar la semantica por defecto. El modo restringido debe ser opt-in para pruebas y para demostrar el feature.

### Como se haria

1. Crear opciones semanticas:

```cpp
struct SemanticOptions {
    bool restricted_inference = false;
};
```

2. Pasar esas opciones a `SemanticAnalyzer`.

```cpp
SemanticAnalyzer sem(engine, options.semantic);
```

3. Agregar una opcion CLI:

```text
--restricted-inference
```

4. En semantica, antes o durante inferencia, detectar anotaciones omitidas en posiciones donde el modo restringido exige tipo explicito o type hole:

- bindings de `let`;
- parametros de funciones;
- retornos de funciones;
- parametros de metodos;
- retornos de metodos;
- parametros de constructores;
- atributos de tipos.

5. Permitir `_` y `auto` porque son solicitudes explicitas de inferencia:

```cpp
bool is_type_hole(const std::string& t) {
    return t == "_" || t == "auto";
}
```

6. Reportar error semantico bloqueante cuando falte anotacion:

```text
Inferencia implicita no permitida en modo restringido.
Use ': _', ': auto' o escriba un tipo concreto.
```

7. Asegurar que el backend no genere IR si aparece ese error. Esto se conecta con la vulnerabilidad 1: los errores semanticos bloqueantes no deben filtrarse por string.

### Como se probaria

Agregar tests validos:

```hulk
let x: _ = 42 in print(x);
```

```hulk
function square(x: _): _ => x * x;
print(square(5));
```

```hulk
type Box(value: _) {
    value: _ = value;
    get(): _ => self.value;
}
print(new Box(7).get());
```

Agregar tests invalidos en modo restringido:

```hulk
let x = 42 in print(x);
```

```hulk
function square(x) => x * x;
print(square(5));
```

```hulk
type Box(value) {
    value = value;
}
```

Agregar al runner una seccion nueva:

```text
BACKEND RESTRICTED-INFERENCE VALIDOS
BACKEND RESTRICTED-INFERENCE INVALIDOS
```

Ejemplos de comandos:

```bash
./hulk_backend tests/extension/restricted_valid_let.hulk --restricted-inference
./hulk_backend tests/extension/restricted_invalid_implicit_let.hulk --restricted-inference
```

### Explicacion

Este modo hace que `_` y `auto` sean mas que alias internos de `Unknown`. Vuelven visible la intencion del programador:

```text
sin anotacion  -> no quiero escribir tipo, solo valido en modo normal
_:             -> quiero inferencia explicita
auto:          -> quiero inferencia explicita legible
```

Tambien ayuda a defender la extension como cambio sintactico y semantico real. Sin este modo, `_` y `auto` funcionan, pero el lenguaje sigue permitiendo inferencia implicita en todas partes.

## 10. Limpiar del flujo end-to-end `lambda`, `range` y `protocol` si no seran parte del lenguaje soportado

### Que falla

El proyecto conserva nodos, parser, semantic checks o referencias para features que el backend final no ejecuta:

- `lambda`;
- `for/range`;
- `protocol`.

Actualmente `IRGen` los rechaza explicitamente:

```cpp
unsupported("for/range");
unsupported("lambda");
unsupported("protocol");
```

Eso significa que el frontend puede reconocer parte de esas construcciones, pero el flujo end-to-end no las soporta.

### Por que falla

El pipeline queda inconsistente:

```text
Parser / AST / Semantica aceptan o modelan el feature
Backend lo rechaza tarde durante IRGen
```

Ese rechazo tardio es mejor que ejecutar mal, pero no es ideal. Si una construccion no forma parte del lenguaje soportado, debe fallar temprano y de forma clara.

Ademas, mantener AST, includes, visitors, semantic paths y tests parciales para features descartados crea deuda. Un cambio en esas zonas puede romper el proyecto aunque el feature no sea parte del objetivo final.

### Que es necesario hacer

Decidir formalmente la politica:

```text
opcion A: implementar lambda/range/protocol en backend
opcion B: retirarlos del lenguaje soportado end-to-end
```

Si la decision es que no seran aceptados, se recomienda la opcion B: limpiarlos o bloquearlos temprano.

### Como se haria

1. Documentar la matriz oficial de soporte:

```text
Feature       Estado
lambda        no soportado
range         no soportado
for           no soportado si depende de range/Iterable
protocol      no soportado
```

2. En parser o semantica, reportar error bloqueante cuando aparezcan esas construcciones.

Opcion estricta:

```text
rechazar desde parser si aparece sintaxis lambda/protocol
```

Opcion mas conservadora:

```text
parser construye AST, semantica reporta "feature no soportado"
```

La opcion conservadora suele ser mejor si ya existen nodos AST y tests de parser, porque evita una refactorizacion grande inmediata.

3. Eliminar o separar tests que presenten esos features como validos end-to-end.

4. Mantener tests negativos claros:

```text
lambda debe fallar con mensaje estable
range debe fallar con mensaje estable
protocol debe fallar con mensaje estable
```

5. Limpiar includes y visitors solo despues de que los tests negativos esten estables.

No conviene borrar todo de golpe. Primero se debe bloquear semanticamente, luego limpiar codigo muerto.

6. Quitar `range` de builtins si no sera parte del lenguaje soportado.

Hoy aparece como builtin semantico:

```text
range(Number, Number) -> Iterable
```

Si `range` no se acepta, esa entrada puede inducir a pensar que el feature existe. Hay que retirarla o marcarla como no soportada.

7. Revisar docs y ejemplos para que no ensenen features excluidos.

Si un ejemplo tiene:

```hulk
for (x in range(0, 10)) print(x);
```

debe moverse a una seccion de "no soportado" o eliminarse.

### Como se probaria

Agregar tests invalidos de frontend/backend:

```hulk
let f = (x) => x + 1 in print(f(2));
```

```hulk
for (x in range(0, 3)) print(x);
```

```hulk
protocol Named {
    name(): String;
}
```

Cada test debe verificar:

- el compilador falla;
- el backend no genera IR;
- el mensaje dice que el feature no esta soportado;
- el error aparece antes de `BannerVM`.

### Explicacion

Una vulnerabilidad end-to-end no siempre es un crash. Tambien puede ser una promesa falsa del lenguaje. Si el frontend acepta una construccion pero el backend no puede ejecutarla, el usuario descubre el problema tarde.

Para un compilador pequeno, es mejor tener un lenguaje soportado mas pequeno pero coherente:

```text
lo que parsea como programa valido
tambien pasa semantica
tambien baja a IR
tambien ejecuta en VM
```

Si `lambda`, `range` y `protocol` no son parte del alcance, deben quedar rechazados de manera explicita, probada y documentada.

## 11. Literales numericos invalidos se convierten silenciosamente en `0`

### Que falla

El adaptador lexer-parser convierte los lexemas numericos con `std::stod`, pero si la conversion falla devuelve `0.0`:

```cpp
double parse_number_lexeme(const std::string& lexeme) {
    try {
        return std::stod(lexeme);
    } catch (const std::exception&) {
        return 0.0;
    }
}
```

Ubicacion:

- `src/parser/parser_lexer_adapter.cpp`

Un literal numerico fuera de rango o no convertible puede llegar al AST como el numero `0`.

### Por que falla

El lexer acepta una secuencia de digitos como token `Number`. Despues, el parser la transforma en `double`. Si esa transformacion falla, el error se oculta y se genera un valor valido.

Ejemplo problematico:

```hulk
print(999999999999999999999999999999999999999999999999999999);
```

En vez de reportar un error de literal numerico, el programa puede ejecutarse como:

```hulk
print(0);
```

### Que es necesario hacer

La conversion de numeros debe reportar diagnostico bloqueante cuando el lexema no se pueda representar como `Number`.

### Como se haria

1. Cambiar `parse_number_lexeme` para no ocultar excepciones.
2. Reportar un diagnostico sintactico o lexico con el span del token.
3. Evitar construir `Number(0)` como fallback.
4. Agregar tests negativos para literales fuera de rango.

Ejemplo esperado:

```text
error: literal numerico fuera de rango
```

### Explicacion

Esta es una vulnerabilidad alta porque corrompe silenciosamente el programa fuente. Un compilador puede rechazar una entrada invalida, pero no debe cambiarla por un valor valido sin avisar.

## 12. Escapes de strings aceptados pero no interpretados

### Que falla

El lexer acepta barras invertidas dentro de strings:

```cpp
if (c == '\\') {
    lexeme += cursor_.advance();
    ...
    lexeme += cursor_.advance();
    continue;
}
```

Pero el parser solo elimina las comillas externas:

```cpp
return lexeme.substr(1, lexeme.size() - 2);
```

Ubicaciones:

- `src/lexer/lexer.cpp`
- `src/parser/grammar.y`
- `src/parser/parser.cpp`

Entonces escapes como `\n`, `\t` o `\"` quedan como dos caracteres literales en el valor del string.

### Por que falla

La documentacion oficial de HULK describe caracteres escapados para saltos de linea, tabulaciones y comillas. El frontend reconoce la forma lexica, pero no transforma el contenido a su valor semantico.

Ejemplo problematico:

```hulk
print("a\nb");
```

La salida esperada segun el lenguaje es un salto de linea entre `a` y `b`. El flujo actual conserva `\n` como texto.

### Que es necesario hacer

Implementar una funcion de decodificacion de strings y validar escapes desconocidos.

### Como se haria

1. Reemplazar `unquote_string_literal` por una funcion que recorra el contenido.
2. Traducir al menos:

```text
\n -> newline
\t -> tab
\" -> "
\\ -> \
```

3. Reportar error para escapes no soportados si la especificacion no los permite.
4. Agregar tests de parser/backend para strings escapados.

### Explicacion

No basta con que el lexer acepte la barra invertida. El literal de string tiene un valor semantico, y ese valor es lo que debe llegar al AST, IR, BannerIR y VM. Si no se decodifica, el programa ejecutado no coincide con el programa escrito.

## 13. `grammar.y` y el parser generado estan desincronizados

### Que falla

La gramatica fuente contiene alternativas para tokens:

```yacc
type_expr
    : IDENTIFIER
    | AUTO
    | UNDERSCORE_TYPE
```

Pero el lexer y el adaptador no producen `AUTO` ni `UNDERSCORE_TYPE`. En la practica, el `parser.cpp` generado que esta en el repo no contiene esas alternativas como tokens reales y trata `auto` y `_` como identificadores.

Ubicaciones:

- `src/parser/grammar.y`
- `src/parser/parser.cpp`
- `src/lexer/token_kind.hpp`
- `src/lexer/keywords.hpp`
- `src/parser/parser_lexer_adapter.cpp`
- `Makefile`

### Por que falla

El build no es reproducible de forma confiable desde las fuentes declaradas. Si se regenera `parser.cpp` desde `grammar.y`, el resultado puede no compilar o cambiar el comportamiento del parser.

La regla `parser-gen` existe:

```make
bison -d -o src/parser/parser.cpp src/parser/grammar.y
```

Pero las fuentes alrededor del lexer no estan completas para los tokens que la gramatica declara.

### Que es necesario hacer

Sincronizar lexer, gramatica y parser generado.

### Como se haria

Opcion A: hacer `auto` y `_` tokens reales.

1. Agregar `Auto` y `UnderscoreType` a `TokenKind`.
2. Agregar `auto` a `kKeywords`.
3. Hacer que `_` produzca `UnderscoreType` solo en contexto de tipo, o aceptar el token globalmente si esa es la politica.
4. Actualizar `parser_lexer_adapter.cpp`.
5. Regenerar `parser.cpp` y `parser.hpp`.

Opcion B: mantenerlos como identificadores especiales.

1. Quitar `AUTO` y `UNDERSCORE_TYPE` de `grammar.y`.
2. Dejar que `type_expr: IDENTIFIER` acepte `auto` y `_`.
3. Documentar que son pseudo-tipos reconocidos semanticamente.
4. Regenerar el parser.

### Explicacion

Esta vulnerabilidad afecta la confiabilidad del proyecto. Un compilador debe poder reconstruirse desde sus fuentes sin depender de un archivo generado accidentalmente viejo.

## 14. El parser acepta multiples expresiones globales

### Que falla

La documentacion oficial dice que el cuerpo del programa termina con una sola expresion global como entrypoint. El parser actual acepta varias expresiones globales y las empaqueta en un bloque:

```cpp
} else {
    $$ = std::make_unique<Hulk::Program>(
        std::move($1.decls),
        std::make_unique<Hulk::ExprBlock>(std::move($1.exprs))
    );
}
```

Ubicacion:

- `src/parser/grammar.y`

Ejemplo aceptado actualmente:

```hulk
print(1);
print(2);
print(3);
```

### Por que falla

El frontend extiende silenciosamente el lenguaje. Eso puede parecer conveniente, pero crea una diferencia entre el contrato oficial y el comportamiento real.

### Que es necesario hacer

Decidir si varias expresiones globales son parte del dialecto soportado.

### Como se haria

Si no son parte del lenguaje:

1. Cambiar la gramatica para permitir declaraciones globales y una unica expresion final.
2. Reportar error si aparece otra expresion despues del entrypoint.
3. Agregar tests negativos.

Si se quieren permitir:

1. Documentarlo como extension local de HULK.
2. Definir claramente que se ejecutan como bloque y que el valor del programa es el de la ultima expresion.

### Explicacion

La vulnerabilidad es de coherencia end-to-end. El usuario puede escribir programas que el compilador acepta, pero que no corresponden al lenguaje oficial usado como referencia.

## 15. Concatenacion `@` acepta valores fuera del contrato documentado

### Que falla

El type checker no valida restricciones para `StringBinOp`:

```cpp
void TypeChecker::visit(StringBinOp& node) {
    node.GetLeft()->accept(*this);
    node.GetRight()->accept(*this);
}
```

La VM convierte cualquier `Word` a texto:

```cpp
heap_.allocate_string(to_string(lhs, heap_) + to_string(rhs, heap_))
```

Ubicaciones:

- `src/typecheck/type_checker.cpp`
- `src/vm/banner_vm.cpp`
- `src/vm/vm_value.cpp`

Ejemplo aceptado:

```hulk
print("x" @ true);
```

### Por que falla

La documentacion oficial describe concatenacion con strings y representacion de numeros. El flujo actual permite booleanos, nil y objetos porque `to_string` tiene una representacion para varios tags internos.

Esto tambien puede filtrar detalles de VM:

```text
object#3
```

### Que es necesario hacer

Definir y aplicar una regla semantica para `@` y `@@`.

### Como se haria

Politica estricta:

1. Exigir que al menos un operando sea `String`.
2. Permitir conversion implicita solo de `Number` si se quiere seguir la documentacion.
3. Rechazar `Boolean`, objetos y nil.

Politica permisiva:

1. Documentar que todo valor HULK tiene representacion textual.
2. Evitar exponer detalles internos como `object#<id>`.
3. Definir `toString` de objetos o una salida estable.

### Explicacion

El problema no es que la VM pueda convertir valores a texto para debug. El problema es que esa conversion se convierte en semantica publica del lenguaje sin haber sido especificada ni chequeada.

## 16. Funciones matematicas pueden producir `nan` e `inf` sin diagnostico

### Que falla

La VM delega funciones matematicas directamente a `<cmath>`:

```cpp
std::sqrt(...)
std::log(...)
std::exp(...)
std::pow(...)
```

Ubicacion:

- `src/vm/banner_vm.cpp`

Ejemplos:

```hulk
print(sqrt(-1));
print(log(1, 10));
```

Pueden producir:

```text
nan
inf
```

### Por que falla

`Number` se trata como `double`, pero el lenguaje no documenta explicitamente `NaN` o `Infinity` como valores observables. Una vez que esos valores entran al programa, afectan igualdad, orden, conversion a string y comportamiento de ramas.

### Que es necesario hacer

Definir si HULK admite semantica IEEE-754 completa o si los dominios invalidos son errores runtime.

### Como se haria

Si se rechazan valores no finitos:

1. Validar dominio antes de `sqrt`, `log`, `pow` cuando aplique.
2. Validar resultado con `std::isfinite`.
3. Reportar error runtime controlado.

Ejemplo:

```text
Runtime error: dominio invalido para sqrt.
```

Si se aceptan:

1. Documentar `nan`, `inf` y sus reglas.
2. Agregar tests de igualdad, comparaciones e impresion.

### Explicacion

Esta vulnerabilidad es media-baja porque no necesariamente rompe memoria, pero si introduce valores especiales no especificados que pueden hacer que programas tipados tengan resultados sorprendentes.

## 17. La firma semantica de `print` no coincide con el comportamiento real

### Que falla

La tabla semantica registra:

```cpp
{"print", 1, {"Object"}, "String"}
```

Pero el nodo `Print` infiere como retorno el tipo del argumento:

```cpp
set_type(node, arg_type);
```

Y la VM devuelve el mismo valor impreso:

```cpp
set(instr.dest_slot, value);
```

Ubicaciones:

- `src/semantic/semantic_tables.cpp`
- `src/inference/type_inferencer.cpp`
- `src/vm/banner_vm.cpp`

### Por que falla

Hay dos contratos internos para la misma operacion:

```text
print(Object) -> String
print(x: T) -> T
```

Hoy `print` suele parsearse como nodo especial, por lo que la tabla builtin puede quedar parcialmente escondida. Pero si una ruta lo resuelve como builtin normal, la inferencia y el runtime no estan alineados.

### Que es necesario hacer

Definir una unica firma oficial para `print`.

### Como se haria

Opcion recomendada:

```text
print(x: Object) -> Object
```

o, si se quiere modelar identidad:

```text
print<T>(x: T) -> T
```

Como el sistema actual no tiene genericos, lo mas simple es:

1. Cambiar la tabla builtin para no decir que retorna `String`.
2. Mantener el nodo `Print` como identidad si esa es la semantica deseada.
3. Agregar tests donde `print(1) + 2` o `print("x") @ "y"` prueben el tipo retornado.

### Explicacion

La firma de una funcion builtin es parte del contrato entre semantica, IR y VM. Si cada fase cree algo distinto, pueden aparecer falsos positivos, falsos negativos o codigo que compila pero falla al ejecutarse.


## Prioridades recomendadas

### Prioridad alta

1. Eliminar tolerancias semanticas por string en `BackendDriver`.
2. Corregir la conversion de literales numericos para que no convierta errores a `0`.
3. Sincronizar `grammar.y`, lexer, adaptador y parser generado.
4. Implementar `restricted-inference` si se va a defender como parte formal del feature de inferencia.
5. Limpiar o bloquear temprano `lambda`, `range` y `protocol` si no seran aceptados.
6. Agregar limites de ejecucion VM: max frames, max steps, max heap.
7. Implementar mark-and-sweep o al menos preparar `VMHeap` con free list.
8. Mejorar errores runtime con funcion, pc e instruccion.

### Prioridad media

1. Decodificar correctamente escapes de strings.
2. Alinear la firma semantica de `print` con la VM.
3. Definir politica de concatenacion `@`/`@@`.
4. Definir politica para `nan`/`inf` en funciones matematicas.
5. Agregar tests unitarios de VM/Word/Heap.
6. Documentar matriz de soporte por feature.

### Prioridad baja

1. Decidir si multiples expresiones globales son extension soportada o error.
2. Vista `--emit-banner-compiled`.

## Plan especifico para Mark-and-Sweep

### Objetivo

Liberar objetos y strings que ya no son alcanzables desde la ejecucion activa, sin cambiar la semantica observable.

### Cambios de datos

Reemplazar vectores simples por entradas con estado:

```cpp
template <typename T>
struct HeapSlot {
    T value;
    bool occupied = false;
    bool marked = false;
    std::uint32_t generation = 0;
};
```

Objetos:

```cpp
std::vector<HeapSlot<VMObject>> objects_;
std::vector<std::size_t> free_objects_;
```

Strings:

```cpp
std::vector<HeapSlot<VMString>> strings_;
std::vector<std::size_t> free_strings_;
```

### Cambios en handles

Actualmente `Word` guarda solo indice. Para GC seguro, conviene agregar generation:

```text
payload = index + generation
```

Asi, si un slot se libera y luego se reutiliza, una referencia vieja no apunta silenciosamente al objeto nuevo.

### Raices

El collector debe recibir raices desde la VM:

- todos los `Frame::slots`;
- todos los `Frame::param_buffer`;
- `CompiledProgram::data`;
- valores temporales locales durante una instruccion, si hay alguno vivo al invocar GC.

### Mark

Algoritmo:

```text
mark_word(w):
  si w es string_ref:
    marcar string
  si w es object_ref:
    si objeto ya marcado: return
    marcar objeto
    para cada field en objeto.fields:
      mark_word(field)
```

Los numeros, bools y nil no se marcan.

### Sweep

Algoritmo:

```text
para cada objeto:
  si ocupado y no marcado:
    liberar slot
    agregar indice a free_objects
  si ocupado y marcado:
    marked = false

para cada string:
  si ocupado y no marcado:
    liberar slot
    agregar indice a free_strings
  si ocupado y marcado:
    marked = false
```

### Disparo de GC

Primera politica recomendada:

```text
si allocations_since_gc > threshold:
  collect()
```

Despues ajustar con:

- crecimiento del heap;
- porcentaje de memoria recuperada;
- modo debug.

### Tests para GC

1. Crear muchos strings temporales con `CONCAT` y verificar que GC recupera.
2. Crear objetos enlazados y verificar que campos mantienen vivos sus hijos.
3. Crear ciclo de objetos y verificar que mark-and-sweep lo libera si no hay raiz externa.
4. Verificar que `.DATA` no se recolecta mientras el programa corre.
5. Verificar que handles invalidos reportan error controlado.

### Riesgo principal

El riesgo principal es invalidar handles. Por eso no se debe compactar el heap en la primera version. Mark-and-sweep sin compactacion es mas simple y seguro para el diseno actual.

## Cierre

El flujo end-to-end actual es funcional y bastante claro: genera HulkIR, baja a BannerIR y ejecuta en BannerVM. Las vulnerabilidades principales no son que falte una VM, sino que el sistema necesita endurecimiento:

- semantica debe bloquear de forma consistente;
- la VM debe tener limites y GC;
- los errores deben tener contexto;
- los fallbacks simbolicos deben desaparecer del camino de ejecucion;
- los tests deben cubrir invariantes internas, no solo salidas finales.

Estos cambios harian que el backend sea mas predecible, mas defendible y mas facil de mantener.
