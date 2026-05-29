# Informe: estado de `for` e iterables en el compilador HULK

Fecha: 2026-05-25

Actualizacion: 2026-05-26

Actualizacion A.10: 2026-05-26

Este informe resume la situacion actual de `for` en el repositorio, contrasta
ese estado con `doc/hulk-docs.pdf` y propone rutas de implementacion para
atacar el problema mas adelante.

## Nota de vigencia

El cuerpo original de este informe describe el estado previo del repo, cuando
`for`, `range`, `Iterable` y `protocol` estaban bloqueados como features no
soportados. Ese diagnostico ya no representa el estado actual.

En la implementacion posterior se adopto la ruta protocolar completa para este
corte:

- `protocol` queda registrado en semantica como contrato estructural;
- `extends` de protocolos esta soportado;
- `Iterable` existe como protocolo builtin;
- `Range` existe como tipo builtin interno;
- `range(Number, Number)` retorna `Range`;
- `Range` conforma a `Iterable` por firma, usando el mismo mecanismo general
  que los tipos de usuario;
- la conformidad protocolar tambien usa firmas inferidas de metodos concretos
  cuando son determinables;
- `T*` existe como iterable tipado, con estrellas anidadas como `Number**`, y
  se representa mediante protocolos sinteticos internos;
- `for` exige que el iterable conforme a `Iterable`;
- la variable del `for` toma el retorno de `current()`;
- `for` baja a llamadas virtuales `next/current` y saltos en IR;
- el evaluador legacy tambien ejecuta `range` y `for`.

Siguen fuera de este corte:

- vectores;
- comprehensions;
- `Enumerable`;
- functors;
- `lambda`, cuya implementacion fue retirada y cuya sintaxis se rechaza en frontend;
- macros.

## Resumen ejecutivo

Esta seccion queda como registro historico del estado anterior a la
implementacion de 2026-05-26.

En la implementacion anterior, `for` **no estaba soportado end-to-end**.

El frontend reconoce la sintaxis y construye un nodo AST `For`, pero el flujo se
bloquea de forma explicita en semantica:

```text
Feature no soportado en el flujo end-to-end: for sobre Iterable/range.
```

Esto ocurre incluso si el `for` no usa `range`:

```hulk
for (item in items) item
```

El parser acepta ese programa, pero `SemanticAnalyzer` y `BackendDriver` lo
rechazan antes de generar IR.

La causa de fondo es que HULK no define `for` como un bucle independiente tipo
C. En la documentacion, `for` siempre itera sobre un valor que cumple el
contrato iterable. El ejemplo base del PDF es:

```hulk
for (x in range(0, 10)) print(x);
```

y se explica como una transpilacion conceptual a:

```hulk
let iterable = range(0, 10) in
while (iterable.next())
    let x = iterable.current() in
        print(x);
```

Por tanto, para implementar `for` de forma util no basta con activar el nodo
`For`: tambien hace falta una fuente de iterables y una bajada segura a
`while`/saltos.

## Estado actual del repo

### Lexer y parser

El lexer reconoce `for` como palabra clave y el parser tiene la regla:

```bnf
for_expr ::= "for" "(" IDENT "in" expr ")" expr
```

La accion semantica construye:

```cpp
std::make_unique<Hulk::For>(varName, iterable, body)
```

Archivo relevante:

- `src/parser/grammar.y`

Conclusion: **la sintaxis de `for` existe**.

### AST

Existe el nodo:

```cpp
class For : public Expr {
    std::string varName;
    std::unique_ptr<Expr> iterable;
    std::unique_ptr<Expr> body;
};
```

Archivo relevante:

- `src/ast/loops/for.h`

Conclusion: **el AST puede representar `for`**.

### Binding

`SymbolResolver::visit(For&)` resuelve el iterable, abre un scope nuevo,
registra una variable sintetica para el simbolo del bucle y resuelve el body.

En terminos de infraestructura, esto ya apunta a la forma correcta:

```text
for (x in iterable) body
```

debe crear una variable local `x` visible solo dentro del body.

Archivo relevante:

- `src/binding/symbol_resolver.cpp`

Conclusion: **hay soporte parcial para scope y variable sintetica**, pero no
hay tipo real asignado a esa variable desde `current()`.

### Semantica

`SemanticAnalyzer` bloquea cualquier `For`:

```cpp
void visit(For& node) override {
    report_unsupported(node.span, "for sobre Iterable/range");
    visit_expr(node.GetIterable());
    visit_expr(node.GetBody());
}
```

Tambien bloquea:

- llamada directa `range(...)`;
- `BuiltinCall::Range`;
- declaracion `protocol`.

Archivo relevante:

- `src/semantic/analyzer.cpp`

Conclusion: **`for` no llega a inferencia, typecheck ni backend como feature
valido**.

### Tabla semantica builtin

`SemanticTables` registra:

```text
Object
Number
String
Boolean
```

y builtins como:

```text
print, sqrt, sin, cos, exp, log, rand
```

No registra:

```text
Iterable
Range
range
Vector
```

Archivo relevante:

- `src/semantic/semantic_tables.cpp`

Conclusion: **no existe un tipo builtin iterable reconocido por semantica**.

### Inferencia y typecheck

`TypeInferencer::visit(For&)` visita iterable y body, y asigna al `For` el tipo
del body. Esto coincide con la documentacion: el valor de un `for` es el ultimo
valor producido por su body.

Pero falta lo esencial:

- inferir el tipo de la variable del bucle desde `iterable.current()`;
- verificar que `iterable.next()` existe y retorna `Boolean`;
- verificar que `iterable.current()` existe;
- distinguir `Range`, `Iterable`, iterables tipados o tipos de usuario.

`TypeChecker::visit(For&)` tambien solo visita iterable y body. No valida el
contrato iterable.

Archivos relevantes:

- `src/inference/type_inferencer.cpp`
- `src/typecheck/type_checker.cpp`

Conclusion: **hay traversal, pero no hay reglas de tipo para iterables**.

### Evaluador

El evaluador directo rechaza `For`:

```cpp
void Evaluator::visit(For& n) {
    report_error(n.span, "SEM_UNSUPPORTED", "for");
}
```

Tambien rechaza `range()`.

Archivo relevante:

- `src/eval/evaluator.cpp`

Conclusion: **el evaluador no ejecuta `for` ni `range`**.

### Backend e IR

`IRGen::visit(For&)` hace:

```cpp
unsupported("for/range");
```

Tambien rechaza `BuiltinFunc::Range`.

Archivo relevante:

- `src/backend/ir_gen.cpp`

Conclusion: **no hay bajada de `for` a HulkIR**.

### BannerIR y BannerVM

BannerIR y BannerVM si tienen las piezas de control de flujo necesarias para
implementar un `for` desazucarado:

```text
Label
Jump
JumpIfTrue
JumpIfFalse
VCall / Call
```

Esas piezas ya se usan para `while`, `if`, llamadas y despacho dinamico.

Lo que no existe es una operacion `FOR`, ni hace falta necesariamente. La ruta
natural es bajar `for` a una forma equivalente a `while`.

Archivos relevantes:

- `src/ir/ir.h`
- `src/backend/hulkir_to_banner.cpp`
- `src/vm/banner_vm.cpp`

Conclusion: **la VM no necesita una instruccion nueva para `for`; necesita que
el frontend/backend lo desazucaren correctamente**.

## Que dice `hulk-docs.pdf`

La documentacion oficial menciona `for` en dos niveles.

### Seccion de loops

HULK define dos bucles:

- `while`;
- `for`.

El ejemplo inicial de `for` es:

```hulk
for (x in range(0, 10)) print(x);
```

La documentacion dice que es equivalente a usar un iterable con `next()` y
`current()`:

```hulk
let iterable = range(0, 10) in
while (iterable.next())
    let x = iterable.current() in
        print(x);
```

Esto implica que `for` depende de un contrato iterable.

### Seccion de iterables

El PDF define conceptualmente:

```hulk
protocol Iterable {
    next(): Boolean;
    current(): Object;
}
```

Luego explica que `range` retorna un `Range` builtin que implementa ese
protocolo.

Tambien menciona que, en la practica, el compilador de referencia transpila
`for` a una forma que llama explicitamente a `next()` y `current()`.

### Seccion de vectores

Mas adelante aparecen vectores:

```hulk
let numbers = [1,2,3,4,5,6,7,8,9] in
for (x in numbers)
    print(x);
```

Pero los vectores son una extension posterior sobre el mismo concepto iterable.
No son necesarios para implementar el primer caso documentado de `for`.

Conclusion: **segun el PDF, `for` es parte del lenguaje, pero su caso minimo
real depende de `range`/`Range` o de algun objeto iterable**.

## Se puede construir un iterable hoy?

Depende de lo que se entienda por "iterable".

### Como objeto manual con `next/current`

Si. Hoy se puede escribir un objeto con metodos `next()` y `current()` y usarlo
manualmente con `while`:

```hulk
type Counter(max) {
    i = 0;
    limit = max;

    next(): Boolean => {
        self.i := self.i + 1;
        self.i <= self.limit;
    };

    current(): Number => self.i;
}

let c = new Counter(3) in
    while (c.next()) print(c.current());
```

Ese programa funciona porque solo usa:

- objetos;
- atributos;
- metodos;
- `while`;
- asignacion destructiva;
- `print`.

### Como iterable reconocido por `for`

Si. Desde la actualizacion de protocolos, un tipo definido por el usuario puede
usarse en `for` si conforma estructuralmente a `Iterable`. Por ejemplo:

```hulk
let c = new Counter(3) in
    for (x in c) print(x);
```

El compilador valida que `Counter` tenga `next(): Boolean` y `current()` con
retorno compatible. No hace falta una sintaxis `implements`.

La bajada de backend genera llamadas normales a `next/current`, de modo que no
existe opcode especial para protocolos ni para `for`.

### Como `range`

Si. `range(Number, Number)` esta registrado como builtin y devuelve `Range`,
que conforma a `Iterable`.

```hulk
for (x in range(0, 3)) print(x);
```

Produce:

```text
0
1
2
```

### Como vector

No. La sintaxis de vectores y comprehensions no esta implementada como feature
end-to-end.

## Problema real a resolver

La pregunta no es solo "activar `for`". Hay que decidir que significa
"iterable" para este compilador.

Hay cuatro contratos posibles:

1. `for` solo funciona con `range(...)`.
2. `for` funciona con cualquier tipo que tenga `next(): Boolean` y `current(): T`.
3. `for` funciona con tipos que implementan un protocolo builtin `Iterable`.
4. `for` funciona con toda la familia del PDF: `Iterable`, `T*`, `Enumerable`,
   vectores, comprehensions y protocolos.

Cada opcion tiene costo y alcance muy distintos.

## Posibles soluciones

### Opcion A: mantener `for` como no soportado

Esta es la politica actual.

Ventajas:

- menor riesgo;
- el backend no ejecuta un feature incompleto;
- los errores son tempranos y estables;
- no obliga a implementar protocolos, vectores ni `range`.

Desventajas:

- el compilador no soporta una construccion documentada en `hulk-docs.pdf`;
- la sintaxis existe pero siempre falla;
- puede ser dificil defenderlo si el alcance del proyecto exige loops `for`.

Esta opcion es valida solo si el informe del proyecto declara explicitamente que
`for`/iterables quedan fuera de alcance.

### Opcion B: MVP `range` + `for` desazucarado

Objetivo: soportar el ejemplo minimo del PDF:

```hulk
for (x in range(0, 10)) print(x);
```

Cambios necesarios:

1. Rehabilitar `range(Number, Number)`.
2. Introducir un tipo interno `Range`.
3. Hacer que `Range` tenga estado:

```text
min/current
max
```

4. Hacer que `Range.next()` retorne `Boolean`.
5. Hacer que `Range.current()` retorne `Number`.
6. Bajar:

```hulk
for (x in iterable) body
```

a una forma equivalente a:

```hulk
let __iter = iterable in
while (__iter.next())
    let x = __iter.current() in
        body;
```

Ventajas:

- implementa el primer ejemplo oficial de `for`;
- no requiere vectores;
- no requiere protocolos completos;
- aprovecha control flow y llamadas virtuales existentes;
- mantiene la VM sin nuevas instrucciones.

Desventajas:

- si solo funciona con `range`, no resuelve iterables definidos por el usuario;
- hay que decidir como representar `Range` en semantica, IR y VM;
- puede introducir una excepcion builtin si no se disena cuidadosamente.

Subopciones de implementacion para `Range`:

- **Prelude interno**: inyectar un `type Range` y una funcion `range` como si
  fueran declaraciones builtin ocultas.
- **Tipo builtin en SemanticTables + lowering especial**: registrar `Range` y
  `range`, y hacer que `IRGen` genere la construccion directamente.
- **Operacion builtin de VM**: agregar una instruccion o builtin runtime para
  construir rangos. Esta opcion acopla mas la VM al lenguaje alto.

Recomendacion si se elige esta opcion: preferir prelude interno o lowering
especial en backend; evitar crear una instruccion VM `FOR`.

### Opcion C: `for` estructural sobre `next/current`

Objetivo: permitir:

```hulk
type Counter(max) {
    i = 0;
    limit = max;

    next(): Boolean => {
        self.i := self.i + 1;
        self.i <= self.limit;
    };

    current(): Number => self.i;
}

let c = new Counter(3) in
    for (x in c) print(x);
```

sin implementar protocolos formales.

Regla semantica:

```text
for (x in e) body
```

es valido si el tipo estatico de `e` tiene:

```text
next(): Boolean
current(): T
```

Entonces:

```text
tipo(x) = T
tipo(for) = tipo(body)
```

Bajada:

```hulk
let __iter = e in
while (__iter.next())
    let x = __iter.current() in
        body
```

Ventajas:

- habilita iterables de usuario sin protocolos;
- se alinea con la idea operacional del PDF;
- no exige vectores ni `T*`;
- se puede combinar despues con `range`.

Desventajas:

- no es nominal ni protocolar como el HULK completo;
- hay que tener cuidado con herencia y overrides de `next/current`;
- si el tipo estatico es `Object` o `Unknown`, no se puede validar bien;
- puede diferir de la semantica oficial con protocolos.

Esta opcion es atractiva como paso intermedio si se quiere que `for` sea util
sin abrir todo el sistema de protocolos.

### Opcion D: MVP recomendado: `Range` builtin + `for` estructural

Esta era una ruta intermedia razonable antes de implementar protocolos. El
estado actual adopta una version mas completa: `Range` builtin + `Iterable`
builtin + conformidad protocolar estructural.

Incluye:

- `range(Number, Number) -> Range`;
- tipo interno `Range` con `next(): Boolean` y `current(): Number`;
- `for` valido sobre cualquier tipo que conforme a `Iterable`;
- lowering de `for` a `while` en IRGen;
- protocolos A.10 core soportados;
- `T*` soportado mediante protocolos sinteticos, incluyendo formas recursivas
  como `Number**`;
- vectores, `Enumerable`, comprehensions, functors, lambdas y macros siguen
  fuera de alcance.

Con esto se soporta:

```hulk
for (x in range(0, 10)) print(x);
```

y tambien:

```hulk
let c = new Counter(3) in
    for (x in c) print(x);
```

sin comprometerse todavia con:

- vectores `Number[]`;
- comprehensions `[x^2 | x in range(...)]`;
- `Enumerable`.

Ventajas:

- cubre el ejemplo base del PDF;
- cubre iterables de usuario simples;
- evita implementar todo protocolos/vectores;
- deja una ruta de evolucion clara hacia HULK completo.

Desventajas:

- requiere tocar varias capas;
- hay que documentar que la conformidad iterable es estructural temporal;
- hay que reservar o proteger nombres internos como `Range` y `range`.

### Opcion E: implementacion completa segun `hulk-docs.pdf`

Esta opcion busca soportar toda la familia:

- `protocol Iterable`;
- `protocol Enumerable`;
- `T*`;
- vectores `T[]`;
- literales vectoriales `[1, 2, 3]`;
- comprehensions `[expr | x in iterable]`;
- `range`;
- `for` sobre iterable y enumerable.

Ventajas:

- maxima fidelidad al PDF;
- permite defender soporte completo de iterables;
- desbloquea muchas features futuras.

Desventajas:

- es una implementacion grande;
- toca parser, AST, semantica, binding, inferencia, typecheck, IR, VM y tests;
- requiere disenar protocolos de verdad;
- aumenta mucho el riesgo de regresiones.

Esta opcion no parece recomendable como primer paso.

## Recomendacion tecnica

La ruta recomendada es la opcion D:

```text
Range builtin + for estructural sobre next/current
```

Motivos:

- soporta el ejemplo principal de `hulk-docs.pdf`;
- no obliga a implementar vectores ni protocolos completos;
- permite usar objetos HULK actuales como iteradores;
- reduce el problema a una transpilacion controlada a `while`;
- no requiere nuevas instrucciones de VM.

## Diseno propuesto para la opcion recomendada

### Semantica

Para `for (x in iterable) body`:

1. Inferir/resolver el tipo de `iterable`.
2. Buscar metodo `next` sin parametros.
3. Verificar que `next` retorna `Boolean`.
4. Buscar metodo `current` sin parametros.
5. Tomar el retorno de `current` como tipo de `x`.
6. Resolver/typecheckear `body` con `x` en scope.
7. El tipo del `for` es el tipo del body.

Errores sugeridos:

```text
El tipo '<T>' no es iterable: falta metodo next(): Boolean.
El tipo '<T>' no es iterable: falta metodo current().
El metodo next() de '<T>' debe retornar Boolean.
```

### Binding

La variable sintetica del `for` ya existe. Habria que extenderla para que su
tipo pueda fijarse a partir de `current()`.

Tambien hay que asegurar que `IRGen` pueda asociar esa variable sintetica con un
slot local concreto.

### Inferencia

Actualmente `For` solo infiere iterable y body. Debe cambiar a:

```text
iterable_type = infer(iterable)
current_type = return_type(iterable_type.current())
synthetic_type[x] = current_type
body_type = infer(body)
for_type = body_type
```

Si el tipo del iterable no se conoce todavia, puede quedar `Unknown` y luego
refinarse, pero para una primera version conviene exigir que el tipo sea
determinable antes de validar `for`.

### Typecheck

Debe confirmar:

- `next()` existe;
- `next()` retorna `Boolean`;
- `current()` existe;
- el body es valido usando `x` con el tipo de `current()`.

### Backend / IRGen

`IRGen::visit(For&)` debe generar control flow equivalente a:

```text
iter_slot = lower(iterable)
result_slot = nil

start:
cond_slot = vcall iter_slot.next()
jump_if_false cond_slot, end

x_slot = vcall iter_slot.current()
body_slot = lower(body)   // con x asociado a x_slot
result_slot = body_slot
jump start

end:
expr_result = result_slot
```

Puntos importantes:

- `iterable` debe evaluarse una sola vez;
- `x` debe vivir solo en el scope del body;
- si no hay iteraciones, el resultado debe seguir la politica de `while`
  actual;
- las llamadas deben usar despacho compatible con metodos normales.

### `range`

Para `range`, la primera version puede ser un builtin que retorna `Range`.

Politica propuesta:

```text
range(start: Number, end: Number) -> Range
```

Semantica:

- ambos argumentos deben ser `Number`;
- `start` incluido;
- `end` excluido;
- si `start >= end`, no itera.

`Range.current()` retorna `Number`.

No se recomienda agregar soporte para paso/step en la primera version, porque el
PDF base usa dos argumentos.

### VM

No se propone una instruccion `FOR`.

La VM ya tiene:

- labels;
- jumps;
- llamadas;
- objetos;
- campos;
- asignacion;
- despacho.

Si `Range` se implementa como tipo interno o prelude, la VM solo ejecuta codigo
normal.

## Tests necesarios

### Tests validos minimos

```hulk
for (x in range(0, 3)) print(x);
```

Salida:

```text
0
1
2
```

```hulk
let total = 0 in {
    for (x in range(1, 4))
        total := total + x;
    print(total);
}
```

Salida:

```text
6
```

```hulk
type Counter(max) {
    i = 0;
    limit = max;

    next(): Boolean => {
        self.i := self.i + 1;
        self.i <= self.limit;
    };

    current(): Number => self.i;
}

let c = new Counter(3) in
    for (x in c) print(x);
```

Salida:

```text
1
2
3
```

### Tests de tipo

Faltan `next`:

```hulk
type Bad {
    current(): Number => 1;
}

let b = new Bad() in for (x in b) print(x);
```

`next` retorna mal:

```hulk
type Bad {
    next(): Number => 1;
    current(): Number => 1;
}

let b = new Bad() in for (x in b) print(x);
```

Falta `current`:

```hulk
type Bad {
    next(): Boolean => false;
}

let b = new Bad() in for (x in b) print(x);
```

### Tests de runtime/IR

- `range(0, 0)` no ejecuta el body.
- `range(3, 0)` no ejecuta el body.
- el iterable se evalua una sola vez.
- el resultado del `for` coincide con el ultimo resultado del body.
- loops anidados con variables distintas.
- shadowing de `x` dentro del body.
- `--emit-ir`, `--emit-banner` y `--emit-banner-compiled` funcionan.

## Riesgos

### Tipo de la variable del `for`

Si el tipo de `current()` es `Object`, el body puede perder precision. Esto es
exactamente el problema que el PDF resuelve luego con `T*` y protocolos
especializados.

Para el MVP estructural, si `current()` esta declarado como `Number`, `String`,
etc., se conserva precision.

### Estado mutable del iterable

La transpilacion llama `next()` antes de `current()`. Eso presupone la misma
convencion del PDF:

```text
next avanza y dice si hay valor disponible
current devuelve el valor actual
```

Hay que documentarlo claramente.

### Reutilizacion del iterable

Un objeto iterable como `Range` puede quedar consumido despues de iterarse. Eso
es coherente con la idea de iterator, pero no con la idea de collection reusable.
La documentacion resuelve esto mas tarde con `Enumerable`.

Para el MVP, `for` debe tratar su entrada como iterator de una sola pasada.

### Nombres reservados

Si se introduce `Range` y `range`, hay que decidir si el usuario puede declarar
un tipo o funcion con esos nombres.

Recomendacion: reservar ambos como builtin para evitar ambiguedades.

### Interaccion con protocolos

Si mas adelante se implementan protocolos reales, la regla estructural temporal
debe migrar o convivir con `Iterable`. Conviene documentarla como politica del
MVP, no como semantica final irreversible.

## Plan de ataque sugerido

### Fase 1: habilitar `for` estructural sin `range`

Objetivo: hacer funcionar iteradores de usuario con `next/current`.

Tareas:

- retirar el bloqueo incondicional de `SemanticAnalyzer::visit(For&)`;
- validar contrato estructural `next/current`;
- asignar tipo a la variable sintetica del `for`;
- implementar `IRGen::visit(For&)` como desazucarado a labels/jumps/vcalls;
- mantener `range` bloqueado.

Ventaja: prueba la parte esencial de `for` sin introducir builtins nuevos.

### Fase 2: implementar `range` y `Range`

Objetivo: soportar el ejemplo principal del PDF.

Tareas:

- registrar `range(Number, Number) -> Range`;
- introducir tipo interno `Range`;
- implementar estado y metodos `next/current`;
- agregar tests end-to-end con `for (x in range(...))`.

### Fase 3: decidir protocolos

Objetivo: decidir si se implementa el modelo oficial completo.

Opciones:

- mantener regla estructural como extension local;
- implementar `protocol Iterable`;
- implementar `T*`;
- implementar `Enumerable`.

Estado actual: `protocol Iterable` y `T*` ya estan implementados. El pendiente
de esta lista es `Enumerable`.

### Fase 4: vectores

Objetivo: habilitar:

```hulk
let numbers = [1, 2, 3] in for (x in numbers) print(x);
```

Esto requiere parser, AST, semantica, heap/runtime y backend para vectores.

## Conclusion

`for` esta en la documentacion HULK y el ejemplo base usa `range`. En el repo,
`for`, `range`, `Iterable` y `protocol` ya estan incorporados al flujo
end-to-end, y `T*` tambien quedo incorporado como iterable tipado.

La ruta implementada fue:

```text
1. protocolos estructurales A.10
2. range + Range builtin
3. Iterable builtin
4. for sobre valores que conforman a Iterable
5. T* como protocolo sintetico especializado, con estrellas anidadas
```

Con eso se cubre el ejemplo principal del PDF y se habilitan iterables creados
por el usuario. Siguen pendientes las capas posteriores del lenguaje: vectores,
`Enumerable`, comprehensions, functors, lambdas y macros.
