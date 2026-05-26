# Informe: estructuras del lenguaje involucradas en `for` e iterables

Fecha: 2026-05-25

Actualizacion: 2026-05-26

Actualizacion A.10: 2026-05-26

Este informe explica las partes estructurales del lenguaje HULK que se tocan, o
podrian tocarse, al implementar `for` e iterables en el compilador. El objetivo
es separar tres cosas:

1. que dice HULK como lenguaje;
2. que forma gramatical tiene cada construccion;
3. que implica cada una para nuestro frontend, semantica, IR y VM.

El punto central es este: en HULK, `for` no es una estructura aislada. `for`
depende de la idea de "iterable", y esa idea conecta con protocolos, metodos,
tipos, scopes, inferencia, vectores y builtins como `range`.

## Nota de vigencia

El informe original analizaba las piezas necesarias antes de implementar
protocolos reales. Desde la actualizacion de 2026-05-26, el repo ya soporta el
nucleo end-to-end de esa ruta:

- declaracion de protocolos de usuario;
- herencia de protocolos con `extends`;
- conformidad estructural implicita por metodos;
- conformidad A.10 con firmas concretas inferidas cuando son determinables;
- anotaciones con nombres de protocolos;
- protocolo builtin `Iterable`;
- iterables tipados `T*` como protocolos sinteticos internos;
- tipo builtin interno `Range`;
- funcion builtin `range(Number, Number): Range`;
- `for` sobre `range` y sobre iterables creados por el usuario;
- bajada de `for` a `next/current` sin opcode VM nuevo.

Por tanto, las secciones que hablan de `for`, `range`, `Iterable` y `protocol`
como "bloqueados" deben leerse como contexto historico. Lo que sigue sin
implementarse en este corte son vectores, comprehensions, `Enumerable`,
functors, lambdas y macros.

## Resumen general

La documentacion de HULK presenta `for` como una expresion que itera sobre un
valor iterable:

```hulk
for (x in range(0, 10)) print(x);
```

Ese `for` se entiende como azucar sintactico para una forma basada en `while`:

```hulk
let iterable = range(0, 10) in
while (iterable.next())
    let x = iterable.current() in
        print(x);
```

Por eso una implementacion real de `for` necesita, como minimo:

- una expresion iterable;
- una forma de llamar `next()`;
- una forma de llamar `current()`;
- una variable local sintetica para el elemento actual;
- una bajada a control de flujo de bajo nivel;
- reglas de tipo para saber que `next()` retorna `Boolean` y `current()` retorna
  el tipo de la variable del bucle.

Antes de la implementacion de 2026-05-26 existian algunas piezas:

- el lexer reconoce `for`;
- el parser construye `For`;
- el AST tiene nodo `For`;
- binding crea una variable sintetica para el simbolo del bucle;
- `while`, objetos y metodos ya funcionan.

En ese momento faltaban las piezas que hacian que `for` fuera un feature
ejecutable:

- `SemanticAnalyzer` bloqueaba cualquier `For`;
- `range` estaba bloqueado;
- `Iterable` no existia como builtin soportado;
- `protocol` estaba bloqueado;
- no hay vectores;
- `IRGen` rechazaba `For`;
- la VM no recibia una forma desazucarada del bucle.

## Mapa de estructuras

| Estructura | Que es en HULK | Gramatica aproximada | Estado actual | Impacto en `for` |
| --- | --- | --- | --- | --- |
| Expresion | Unidad que produce valor | `expr ::= ...` | Soportada | `for` y `while` son expresiones. |
| Bloque | Secuencia de expresiones con valor final | `{ expr; expr; ... }` | Soportado | Sirve como body de `for`. |
| `let` | Binding local con scope | `let bindings in expr` | Soportado | Forma natural del desazucarado. |
| `while` | Bucle expresion | `while (expr) expr` | Soportado | Target natural para bajar `for`. |
| `for` | Iteracion sobre iterable | `for (x in expr) expr` | Soportado | Itera sobre valores que conforman a `Iterable`. |
| Tipo/objeto | Clase nominal con atributos/metodos | `type T(...) { ... }` | Soportado | Un iterable puede ser un objeto. |
| Metodo | Funcion asociada a tipo | `m(args): T => expr` | Soportado | `next/current` son metodos. |
| Protocolo | Contrato estructural | `protocol P { ... }` | Soportado | Base oficial de `Iterable`. |
| `Iterable` | Protocolo con `next/current` | builtin conceptual | Soportado | Contrato oficial para `for`. |
| `range`/`Range` | Iterable builtin numerico | `range(a,b)` | Soportado | Ejemplo base del PDF. |
| `T*` | Iterable tipado de `T` | `type_expr "*"` | Soportado | Da tipo preciso a `x`. |
| `Enumerable` | Coleccion que crea iteradores | `protocol Enumerable` | No soportado | Permite iterar colecciones reutilizables. |
| Vector | Coleccion homogenea | `[a,b,c]`, `T[]` | No soportado | Caso posterior de iterable. |
| Comprehension | Vector generado por iteracion | `[expr | x in iterable]` | No soportado | Depende de `for`/iterables. |

## Expresiones como base del lenguaje

HULK es un lenguaje orientado a expresiones. Muchas construcciones que en otros
lenguajes son "sentencias" aqui producen valor:

```hulk
if (cond) a else b
while (cond) body
for (x in iterable) body
{
    expr1;
    expr2;
}
```

Esto afecta directamente a `for`: no basta con ejecutar el body. El `for` debe
tener un valor de resultado. La documentacion indica que, igual que `while`, el
valor del `for` es el ultimo valor producido por su body.

Implicacion para implementacion:

- `IRGen::visit(For&)` debe producir un slot resultado;
- si hay iteraciones, ese slot se actualiza con el valor del body;
- si no hay iteraciones, debe seguir la misma politica que `while` en el repo
  actual, normalmente un valor vacio/nil/default segun la representacion usada.

## Bloques

Los bloques son expresiones que agrupan varias expresiones:

```bnf
block ::= "{" block_body_opt "}"
block_body_opt ::= expr (";" expr)* ";"? | epsilon
```

Ejemplo:

```hulk
{
    print(1);
    print(2);
    3;
}
```

Un bloque ejecuta sus expresiones en orden y retorna el valor de la ultima.

Por que importa para `for`:

- el body de `for` puede ser una expresion simple o un bloque;
- el valor de cada iteracion es el valor del body;
- un `for` con body bloque debe preservar scopes internos, asignaciones y
  side-effects.

Estado actual:

- soportado en parser, AST, semantica, IR y VM;
- es una pieza reutilizable para implementar `for`.

## `let` y scope local

El `let` introduce variables locales:

```bnf
let_expr ::= "let" binding_list "in" expr
binding_list ::= binding ("," binding)*
binding ::= IDENT type_ann_opt "=" expr
```

Ejemplo:

```hulk
let x = 10 in x + 1
```

El desazucarado de `for` necesita dos usos conceptuales de `let`:

```hulk
let iterable = source in
while (iterable.next())
    let x = iterable.current() in
        body
```

Hay dos variables importantes:

- `iterable`: guarda el objeto iterador; debe evaluarse una sola vez;
- `x`: variable del bucle; cambia en cada iteracion y vive solo dentro del body.

Estado actual:

- `let` ya funciona;
- binding y typecheck manejan scopes locales;
- `For` ya crea una variable sintetica en `SymbolResolver`, pero falta conectarla
  con un valor/slot en IRGen.

## `while`

Gramatica:

```bnf
while_expr ::= "while" "(" expr ")" expr
```

Ejemplo:

```hulk
let a = 10 in while (a >= 0) {
    print(a);
    a := a - 1;
}
```

En HULK, `while` es una expresion. Su condicion debe ser `Boolean`, y su body
puede ser cualquier expresion.

Por que importa para `for`:

- el PDF define `for` como una transformacion a `while`;
- nuestro backend ya baja `while` a labels, jumps y condiciones;
- implementar `for` como desazucarado evita agregar una instruccion VM nueva.

Implementacion esperada de `for`:

```text
iter = lower(iterable)
result = nil

start:
cond = iter.next()
jump_if_false cond, end
x = iter.current()
body_value = lower(body)
result = body_value
jump start

end:
return result
```

## `for`

Gramatica:

```bnf
for_expr ::= "for" "(" IDENT "in" expr ")" expr
```

Ejemplo base del PDF:

```hulk
for (x in range(0, 10)) print(x);
```

Que es realmente:

- no es un contador numerico por si mismo;
- no define inicio, fin ni incremento;
- itera sobre un valor que sabe avanzar con `next()`;
- obtiene cada elemento con `current()`;
- crea una variable local para el elemento actual.

Esto significa que `for` depende de un contrato:

```text
next()    -> Boolean
current() -> T
```

Donde `T` es el tipo de la variable del bucle.

Estado actual:

- parseado;
- representado en AST;
- variable sintetica parcialmente registrada;
- bloqueado en semantica;
- bloqueado en IRGen.

Decisiones necesarias para implementarlo:

- aceptar solo `range` o cualquier objeto con `next/current`;
- decidir como se tipa la variable del bucle;
- decidir que error se reporta si faltan metodos;
- decidir si `for` sera estructural temporalmente o dependera de protocolos.

## Tipos, objetos, atributos y metodos

HULK permite definir tipos:

```bnf
type_decl ::= "type" IDENT ctor_params_opt inherits_opt "{" type_member* "}"
```

Ejemplo:

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
```

Este tipo ya se parece a un iterable:

- tiene estado interno (`i`, `limit`);
- tiene `next()`;
- tiene `current()`.

Hoy ese objeto puede usarse manualmente:

```hulk
let c = new Counter(3) in
    while (c.next()) print(c.current());
```

Tras la implementacion de protocolos, un objeto de usuario si puede usarse con
`for` siempre que conforme estructuralmente al protocolo `Iterable`, es decir,
si expone `next(): Boolean` y `current()` con retorno compatible.

Implicacion actual:

- la validacion ya no es una regla temporal ad hoc;
- `for` se apoya en conformidad protocolar real;
- los tipos definidos por el usuario pueden actuar como iterables sin declarar
  `implements`.

## Llamadas a metodos y acceso a miembros

Gramatica aproximada:

```bnf
postfix ::= primary
          | postfix "." IDENT
          | postfix "." IDENT "(" args_opt ")"
```

Ejemplos:

```hulk
iterable.next()
iterable.current()
obj.field
obj.method(arg)
```

Por que importa para `for`:

- el desazucarado usa llamadas a metodos;
- `next()` debe ser llamada sin argumentos;
- `current()` debe ser llamada sin argumentos;
- esas llamadas pueden ser virtuales si el tipo real redefine metodos.

Estado actual:

- llamadas a metodos y despacho existen;
- `for` puede aprovechar esa infraestructura;
- IRGen baja `For` a llamadas `next/current` y flujo de control explicito.

## Protocolos

Un protocolo en HULK es un contrato estructural. Describe metodos requeridos,
pero no es una clase concreta.

Gramatica conceptual:

```bnf
protocol_decl ::= "protocol" IDENT protocol_extends_opt "{" protocol_member* "}"
protocol_extends_opt ::= "extends" IDENT | epsilon
protocol_member ::= IDENT "(" typed_params_opt ")" ":" type_expr ";"
```

Ejemplo del concepto iterable:

```hulk
protocol Iterable {
    next(): Boolean;
    current(): Object;
}
```

Que significa:

- cualquier tipo que tenga esos metodos conforma al protocolo;
- no hace falta que el tipo declare explicitamente "implements Iterable";
- la conformidad se deduce por estructura;
- los protocolos sirven para tipar variables, parametros y retornos de forma mas
  abstracta.

Por que importa para `for`:

- en el HULK completo, `for` funciona sobre valores que cumplen `Iterable`;
- `Iterable` es el contrato oficial de `next/current`;
- `T*` se apoya en protocolos especializados de iterables.

Estado actual:

- hay token/parser/AST para `protocol` y `extends`;
- `SemanticTables` registra protocolos y sus metodos;
- hay conformidad estructural implicita;
- la conformidad A.10 usa firmas explicitas o inferidas cuando son
  determinables;
- hay typecheck de protocolos en anotaciones, llamadas y asignaciones;
- los protocolos siguen siendo compile-time only y no generan runtime.

Impacto de la implementacion:

- `SemanticTables` contiene protocolos builtin y de usuario;
- el resolver bloquea redeclaraciones y usos runtime invalidos;
- inferencia y typecheck consultan metodos de protocolos cuando el receptor esta
  tipado como protocolo;
- backend ignora `ProtocolDecl` porque la seguridad queda cerrada antes de IR.

Quedan fuera de este corte otras capas que el PDF construye encima de
protocolos: `Enumerable`, vectores, comprehensions, functors, lambdas y macros.

## `Iterable`

`Iterable` es un protocolo builtin conceptual:

```hulk
protocol Iterable {
    next(): Boolean;
    current(): Object;
}
```

No representa "una lista" ni "un vector". Representa un iterador de una sola
pasada:

- `next()` avanza;
- `next()` dice si hay valor;
- `current()` devuelve el valor actual.

El retorno de `current()` aparece como `Object` en el protocolo base. Esto da
flexibilidad, pero pierde precision de tipo. Por eso HULK introduce despues
iterables tipados (`T*`).

Estado actual del repo:

- `Iterable` esta registrado como protocolo builtin;
- `Range` conforma a `Iterable` por el mismo mecanismo general de protocolos;
- los tipos de usuario tambien pueden conformar a `Iterable`;
- `for` exige conformidad con `Iterable` y usa el retorno concreto de
  `current()` para tipar la variable del ciclo cuando es posible.

Implementacion aplicada:

- registrar `Iterable` como protocolo builtin;
- registrar `Range` como tipo builtin interno;
- registrar `range(Number, Number): Range`;
- validar `for` contra `Iterable`;
- bajar `for` a llamadas `next/current`.

## `range` y `Range`

El primer ejemplo oficial de iterable en el PDF es `range`.

Forma de uso:

```hulk
range(start, end)
```

Ejemplo:

```hulk
for (x in range(0, 10)) print(x);
```

Semantica esperada:

- `start` incluido;
- `end` excluido;
- produce numeros;
- se consume mediante `next/current`.

El PDF describe conceptualmente un tipo `Range` con estado:

```hulk
type Range(min: Number, max: Number) {
    current = min - 1;

    next(): Boolean => (self.current := self.current + 1) < max;
    current(): Number => self.current;
}
```

Que implica implementar `range`:

- registrar una funcion builtin `range(Number, Number) -> Range`;
- introducir un tipo `Range` conocido por semantica/backend;
- generar un objeto con estado interno;
- hacer que `Range.next()` y `Range.current()` sean invocables;
- asegurar que `for` sobre `Range` asigna tipo `Number` a la variable.

Estado actual:

- `range(Number, Number)` esta registrado como builtin;
- `Range` existe como tipo builtin interno;
- `Range.next()` y `Range.current()` se inyectan en backend;
- `Range` conforma a `Iterable`;
- `for (x in range(a, b)) ...` funciona end-to-end y tipa `x` como `Number`.

## Iterables tipados: `T*`

La documentacion introduce `T*` para representar "iterable de T".

Ejemplo:

```hulk
function sum(numbers: Number*): Number =>
    let total = 0 in
        for (x in numbers)
            total := total + x;
```

Que significa:

- `numbers` es iterable;
- cada `current()` produce `Number`;
- dentro del `for`, `x` debe inferirse como `Number`.

Por que existe:

- `Iterable.current()` retorna `Object`;
- eso es demasiado impreciso para codigo numerico;
- `Number*` permite expresar un iterable cuyo elemento actual es `Number`.

Gramatica necesaria:

```bnf
type_expr ::= IDENT
            | type_expr "*"
```

Implementacion real:

- parsear `T*`;
- representar ese tipo en AST o tipo semantico;
- generar/verificar un protocolo especializado equivalente a:

```hulk
protocol Iterable_T extends Iterable {
    current(): T;
}
```

Estado actual:

- soportado;
- `type_expr` acepta `type_expr "*"` para anotaciones como `Number*` y
  `Number**`;
- el resolver crea protocolos sinteticos internos `T* extends Iterable`;
- el protocolo sintetico redefine `current(): T`;
- `T*` es compile-time only y se rechaza en `is`/`as`;
- `for` sobre un valor anotado como `Number*` tipa la variable del ciclo como
  `Number`.

Limitacion actual:

- se soportan estrellas anidadas como `Number**`, pero no tipos compuestos como
  `T[]` o functors;
- `Enumerable` todavia no esta implementado.

## `Enumerable`

Un `Iterable` es un iterador de una sola pasada. Una coleccion reusable necesita
crear un iterador nuevo cada vez.

La documentacion propone `Enumerable`:

```hulk
protocol Enumerable {
    iter(): Iterable;
}
```

Idea:

- un iterable se consume;
- un enumerable produce un iterable nuevo;
- esto permite multiples iteraciones sobre la misma coleccion.

Desazucarado conceptual:

```hulk
let iterable = enumerable.iter() in
while (iterable.next())
    let x = iterable.current() in body
```

Por que importa:

- vectores son colecciones, no solo iteradores;
- para iterar varias veces un vector, conviene que el vector sea enumerable y
  cree iteradores separados.

Estado actual:

- no soportado;
- depende de protocolos e iterables.

Recomendacion:

- dejar para una fase posterior a `range` y `for` estructural.

## Vectores

Un vector en HULK es una coleccion homogenea de objetos/valores del mismo tipo.
Es parecido a un arreglo dinamico o array.

Sintaxis explicita:

```hulk
let numbers = [1, 2, 3, 4, 5] in
for (x in numbers)
    print(x);
```

Acceso por indice:

```hulk
numbers[2]
```

Tipo vectorial:

```hulk
Number[]
```

Metodos esperados segun la documentacion:

```hulk
numbers.size()
numbers.next()
numbers.current()
```

En realidad, para un diseno mas limpio, el vector como coleccion deberia
exponer un iterador o cumplir `Enumerable`. El PDF presenta los vectores como
iterables, pero tambien habla de la diferencia entre iterable de una pasada y
coleccion reusable.

Gramatica necesaria:

```bnf
primary ::= "[" expr_list_opt "]"
type_expr ::= type_expr "[" "]"
postfix ::= postfix "[" expr "]"
```

Impacto de implementacion:

- nuevo AST para vector literal;
- nuevo AST para indexacion;
- nuevo tipo semantico `Vector<T>` o similar;
- heap/runtime para almacenar elementos;
- operaciones `size`, indexacion y posiblemente iteracion;
- GC debe marcar elementos dentro del vector;
- IR/VM deben poder crear y acceder a vectores.

Estado actual:

- no soportado.

Recomendacion:

- no implementar vectores como primer paso para `for`;
- implementar despues de tener `for` + `range`.

## Comprehensions o vectores implicitos

La documentacion presenta vectores generados por patron:

```hulk
let squares = [x^2 | x in range(1, 10)] in print(squares);
```

Gramatica:

```bnf
primary ::= "[" expr "|" IDENT "in" expr "]"
```

Que significa:

- se evalua un iterable;
- por cada elemento, se enlaza `x`;
- se evalua `x^2`;
- se construye un vector con los resultados.

Esto depende de:

- `for` o mecanismo equivalente de iteracion;
- vectores;
- scopes;
- inferencia del tipo de `x`;
- inferencia del tipo de los elementos generados.

Estado actual:

- no soportado.

Recomendacion:

- dejar para despues de vectores.

## Tipos extendidos: `T*`, `T[]` y tipos de funcion

Para iterables/vectores completos hay que ampliar la gramatica de tipos.

Hoy el nucleo usa tipos nominales simples:

```bnf
type_expr ::= IDENT
```

Para HULK completo haria falta:

```bnf
type_expr ::= IDENT
            | type_expr "*"
            | type_expr "[" "]"
            | "(" type_list ")" "->" type_expr
```

Donde:

- `T*` significa iterable de `T`;
- `T[]` significa vector de `T`;
- `(A, B) -> C` sirve para functors/lambdas.

Para el problema actual, los relevantes son:

- `T*`;
- `T[]`.

Estado actual:

- `T*` ya esta incorporado, incluyendo estrellas anidadas como `Number**`;
- `T[]` y tipos de funcion siguen pendientes.

## Variable sintetica del `for`

En:

```hulk
for (x in iterable) body
```

`x` no viene de un `let` escrito por el usuario, pero semantica debe tratarlo
como una variable local nueva.

Conceptualmente:

```hulk
let x = iterable.current() in body
```

Implicaciones:

- `x` vive solo dentro del body;
- `x` puede sombrear variables externas;
- `x` debe tener tipo igual al retorno de `current()`;
- `x` debe tener un slot/local en IRGen;
- el valor de `x` debe actualizarse en cada iteracion.

Estado actual:

- `SymbolResolver` crea un simbolo sintetico de for;
- falta asignarle tipo real;
- falta conectarlo con un slot/valor de IR.

## Capas del compilador que se tocarian

### Parser / AST

Para el MVP de `for`:

- no hay que cambiar la gramatica de `for`;
- el nodo `For` ya existe.

Para `range`:

- no hace falta gramatica nueva si se modela como llamada normal.

Para vectores/protocolos completos:

- si hay que ampliar gramatica y AST.

### Semantica

Hay que retirar el bloqueo incondicional de `For` y reemplazarlo por validacion:

```text
iterable tiene next(): Boolean
iterable tiene current(): T
```

Para `range`, hay que registrar o reconocer:

```text
range(Number, Number) -> Range
```

### Binding

Hay que mantener la variable sintetica del `for` y asociarla con un tipo y luego
con un slot de IR.

### Inferencia y typecheck

Hay que inferir:

```text
tipo(iterable)
tipo(x) = retorno de current()
tipo(for) = tipo(body)
```

Y validar:

```text
next() retorna Boolean
current() existe
```

### Backend / IR

Hay que implementar el desazucarado:

```text
For -> labels + jump_if_false + llamada next + llamada current + body
```

No se recomienda crear una instruccion IR `For`; es mejor bajar a control flow
existente.

### BannerVM

No necesita una operacion `FOR` si IRGen baja correctamente el bucle.

Si se implementan vectores, la VM si necesitara soporte de almacenamiento,
indexacion y marcado GC de elementos.

## Orden recomendado de implementacion

### Paso 1: `for` estructural sobre `next/current`

Permitir:

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

Esto prueba la semantica real de `for` sin implementar `range` ni vectores.

### Paso 2: `range` y `Range`

Permitir el ejemplo base del PDF:

```hulk
for (x in range(0, 10)) print(x);
```

### Paso 3: formalizar `Iterable`

Decidir si se mantiene validacion estructural o se implementa `protocol
Iterable` completo.

### Paso 4: iterables tipados `T*`

Permitir:

```hulk
function sum(numbers: Number*): Number => ...
```

Estado actual: implementado mediante protocolos sinteticos.

### Paso 5: vectores `T[]` y literales `[ ... ]`

Permitir:

```hulk
let numbers = [1, 2, 3] in for (x in numbers) print(x);
```

### Paso 6: comprehensions

Permitir:

```hulk
let squares = [x^2 | x in range(1, 10)] in print(squares);
```

## Conclusion

La implementacion de `for` no es solo "activar un visitor". En HULK, `for`
descansa sobre un ecosistema de estructuras:

```text
for -> iterable -> next/current -> tipos/metodos/scopes -> lowering a while
```

El PDF luego expande ese ecosistema con:

```text
protocol Iterable -> T* -> Enumerable -> Vector T[] -> comprehensions
```

Para nuestro compilador, la estrategia tecnicamente mas razonable es no empezar
por vectores ni protocolos completos. Primero debe implementarse el nucleo
operacional:

```text
for estructural sobre next/current + range/Range builtin
```

Eso alinea el compilador con el primer ejemplo oficial de HULK, reduce riesgo y
aprovecha piezas que ya existen en el repo: objetos, metodos, `while`, scopes,
labels, jumps y llamadas dinamicas.
