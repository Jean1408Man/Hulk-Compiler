# Reporte de feature: inferencia explícita con `_` y `auto` para HULK

## 1. Resumen ejecutivo

Se propone escoger como feature adicional una extensión de la inferencia de tipos de HULK basada en **anotaciones inferidas explícitas**. La extensión agrega dos nuevas formas sintácticas de anotación de tipo:

```hulk
_
auto
```

Ambas significan lo mismo: **el programador declara explícitamente que quiere que el compilador infiera ese tipo**.

Ejemplos:

```hulk
let x: _ = 42 in print(x);

let message: auto = "hello" in print(message);

function square(x: _): _ => x * x;

type Point(x: auto, y: auto) {
    x: _ = x;
    y: _ = y;

    norm(): auto => sqrt(self.x ^ 2 + self.y ^ 2);
}
```

La idea conserva la inferencia oficial de HULK, pero la convierte en un feature más defendible para el proyecto porque ya no se presenta solo como un pase semántico interno. Ahora hay una modificación visible de la sintaxis y una semántica nueva asociada a esa sintaxis.

**Nombre recomendado del feature:**

> Inferencia explícita mediante type holes: `_` y `auto`

También puede presentarse como:

> Anotaciones inferidas explícitas y modo restringido de inferencia

---

## 2. Justificación frente al requisito del proyecto

La orientación del proyecto exige implementar al menos un feature adicional que involucre modificaciones a la **sintaxis** y a la **semántica** del lenguaje. La inferencia de tipos oficial de HULK es principalmente una característica semántica: el compilador asigna tipos a expresiones y símbolos no anotados antes del type checking. Por sí sola, si se implementa exactamente como aparece en la documentación, puede ser discutible como feature adicional porque la sintaxis de omitir tipos ya existe en HULK.

Con esta propuesta, el feature sí modifica ambas cosas:

| Aspecto | Modificación propuesta |
|---|---|
| Sintaxis | Se agregan `_` y `auto` como anotaciones de tipo válidas en posiciones donde hoy se espera un `type_ann`. |
| Semántica | `_` y `auto` introducen un hueco de tipo que el inferidor debe resolver antes del type checker. |
| Diagnóstico | Si no se puede resolver el hueco, el compilador reporta las restricciones encontradas y sugiere una anotación explícita. |
| Opcional | Se agrega un `restricted mode` donde solo se permite inferencia si el programador la pide explícitamente con `_` o `auto`. |

La ventaja es que el feature queda conectado con A.9, pero tiene identidad propia: no es simplemente “implementar lo que HULK ya decía”, sino una extensión sintáctica y semántica clara.

---

## 3. Base oficial en HULK

La documentación de HULK define el lenguaje como estáticamente tipado con anotaciones opcionales e inferencia de tipos. El inferidor corre antes del type checker, asigna tipos a expresiones y a símbolos no anotados, y luego el type checker verifica conformidad.

La documentación también separa dos tareas:

1. Inferir tipos de expresiones de forma bottom-up.
2. Inferir tipos de símbolos no anotados, como variables, atributos, parámetros, argumentos de tipos y retornos.

Además, la propia documentación reconoce que la inferencia general es compleja y no fija un algoritmo único. En su lugar, exige que, si el inferidor logra inferir un tipo, ese tipo cumpla ciertas restricciones de corrección. Si no puede inferirlo de forma segura, debe fallar y pedir una anotación explícita.

Esta propuesta respeta esa filosofía. No intenta implementar una inferencia universal, sino una inferencia segura, explicable y acotada.

---

## 4. Qué cambia en la sintaxis

### 4.1 Sintaxis actual simplificada

Una versión simplificada de la gramática de tipos puede verse así:

```bnf
type_ann ::= ID
           | ID '[]'
           | ID '*'
           | '(' type_list ')' '->' type_ann
```

Con esto, una declaración de variable anotada se escribe:

```hulk
let x: Number = 42 in print(x);
```

Una función anotada se escribe:

```hulk
function tan(x: Number): Number => sin(x) / cos(x);
```

### 4.2 Nueva sintaxis propuesta

Se agregan dos alternativas a `type_ann`:

```bnf
type_ann ::= ID
           | ID '[]'
           | ID '*'
           | '(' type_list ')' '->' type_ann
           | '_'
           | 'auto'
```

Entonces pasan a ser válidos:

```hulk
let x: _ = 42 in print(x);
let y: auto = sqrt(25) in print(y);

function square(x: _): _ => x * x;
function negate(b: auto): auto => !b;
```

### 4.3 Diferencia entre `_` y `auto`

Para mantener el diseño simple:

```hulk
_    == auto
```

Ambos son alias semánticos. La única diferencia es de estilo:

- `_` es más corto y se parece a un “type hole”.
- `auto` es más legible y explícito para principiantes.

Ejemplo equivalente:

```hulk
let x: _ = 42 in print(x);
```

```hulk
let x: auto = 42 in print(x);
```

Ambos se normalizan internamente a un nodo de tipo:

```cpp
InferTypeAnnotation
```

O, si se quiere conservar el spelling original para mensajes de error:

```cpp
enum class InferTypeSyntax {
    Underscore,
    Auto
};

struct InferTypeAnnotation : TypeAnnotation {
    InferTypeSyntax syntax;
};
```

---

## 5. Dónde se permite `_` y `auto`

La extensión permite `_` y `auto` en todas las posiciones donde HULK permite anotaciones de tipo.

### 5.1 Variables de `let`

```hulk
let x: _ = 42 in print(x);
let text: auto = "hello" in print(text);
```

### 5.2 Parámetros de funciones

```hulk
function double(x: _) => x * 2;
function is_zero(x: auto): Boolean => x == 0;
```

### 5.3 Retornos de funciones

```hulk
function double(x: Number): _ => x * 2;
function greeting(name: String): auto => "hello " @ name;
```

### 5.4 Parámetros y retornos de métodos

```hulk
type Counter(value: Number) {
    value: Number = value;

    inc(delta: _): _ => self.value := self.value + delta;
}
```

### 5.5 Atributos

```hulk
type Point(x: Number, y: Number) {
    x: _ = x;
    y: auto = y;
}
```

### 5.6 Argumentos de tipo o constructor

```hulk
type Box(value: _) {
    value: _ = value;

    get(): _ => self.value;
}
```

Este caso es más delicado. Se recomienda permitirlo solo si el uso del argumento dentro de los atributos y métodos permite una inferencia única. Si no, el compilador debe fallar.

---

## 6. Semántica de `_` y `auto`

### 6.1 Concepto central: type holes

Cada aparición de `_` o `auto` introduce un **hueco de tipo**. El inferidor debe reemplazar ese hueco por un tipo concreto antes del type checking.

Ejemplo:

```hulk
let x: _ = 42 in print(x);
```

El AST inicial contiene algo equivalente a:

```txt
VariableBinding(
  name = "x",
  annotation = TypeHole#1,
  initializer = NumberLiteral(42)
)
```

Después de inferencia:

```txt
TypeHole#1 = Number
x : Number
```

El programa queda semánticamente equivalente a:

```hulk
let x: Number = 42 in print(x);
```

### 6.2 Diferencia entre no anotar y usar `_`/`auto`

En modo normal, estas dos formas pueden terminar haciendo lo mismo:

```hulk
let x = 42 in print(x);
```

```hulk
let x: _ = 42 in print(x);
```

Pero conceptualmente no son idénticas:

| Forma | Significado |
|---|---|
| `let x = 42` | El programador omitió el tipo. El compilador puede intentar inferirlo según la inferencia normal de HULK. |
| `let x: _ = 42` | El programador pidió explícitamente inferencia en esa posición. |
| `let x: auto = 42` | Igual que `_`, pero usando una palabra más descriptiva. |

Esta diferencia se vuelve importante en `restricted mode`.

---

## 7. Flujo de compilación propuesto

La inferencia explícita debe ocurrir después del binding y antes del type checking.

```txt
Lexer
  ↓
Parser
  ↓
AST con TypeAnnotation normal / TypeHole
  ↓
Binding y tablas semánticas
  ↓
Inferencia de tipos
  ↓
AST anotado completamente
  ↓
Type checking
  ↓
IR / evaluación / backend
```

El type checker no debería recibir huecos sin resolver. Si llega un `TypeHole` al type checker, eso debe considerarse un error interno o un error semántico de inferencia no resuelta.

---

## 8. Reglas de inferencia por casos

### 8.1 Literales

```hulk
let n: _ = 42 in print(n);
let s: _ = "hello" in print(s);
let b: _ = true in print(b);
```

Inferencia:

```txt
42      : Number
"hello" : String
true    : Boolean
```

Resultado:

```txt
n : Number
s : String
b : Boolean
```

Código equivalente:

```hulk
let n: Number = 42 in print(n);
let s: String = "hello" in print(s);
let b: Boolean = true in print(b);
```

### 8.2 Operaciones aritméticas

```hulk
function square(x: _): _ => x * x;
```

Reglas:

```txt
x * x exige x : Number
x * x produce Number
```

Resultado:

```txt
x : Number
square : (Number) -> Number
```

Equivalente:

```hulk
function square(x: Number): Number => x * x;
```

Otro ejemplo:

```hulk
function hypotenuse(a: auto, b: auto): auto => sqrt(a ^ 2 + b ^ 2);
```

Inferencia:

```txt
a ^ 2        exige a : Number
b ^ 2        exige b : Number
a^2 + b^2    produce Number
sqrt(Number) produce Number
```

Resultado:

```txt
a : Number
b : Number
retorno : Number
```

### 8.3 Operaciones booleanas

```hulk
function negate(x: _): _ => !x;
```

Inferencia:

```txt
!x exige x : Boolean
!x produce Boolean
```

Resultado:

```hulk
function negate(x: Boolean): Boolean => !x;
```

Otro ejemplo:

```hulk
function both(a: auto, b: auto): auto => a & b;
```

Resultado:

```txt
a : Boolean
b : Boolean
retorno : Boolean
```

### 8.4 Comparaciones numéricas

```hulk
function positive(x: _): _ => x > 0;
```

Inferencia:

```txt
x > 0 exige x : Number
x > 0 produce Boolean
```

Resultado:

```hulk
function positive(x: Number): Boolean => x > 0;
```

### 8.5 Igualdad y desigualdad

```hulk
function is_zero(x: _): _ => x == 0;
```

Regla recomendada:

- `==` y `!=` siempre producen `Boolean`.
- Si un lado tiene tipo concreto y el otro es un hueco, el hueco puede recibir ese tipo.
- Si ambos lados son huecos sin otra información, no se infiere un tipo concreto.

Inferencia:

```txt
0 : Number
x == 0 produce Boolean
x debe ser comparable con Number
x : Number
```

Resultado:

```hulk
function is_zero(x: Number): Boolean => x == 0;
```

Caso ambiguo:

```hulk
function same(a: _, b: _): _ => a == b;
```

Esto debe fallar si no hay más información:

```txt
error: no se pudo inferir el tipo de a ni de b
motivo: la igualdad solo indica que ambos deben ser comparables, pero no determina un tipo concreto
sugerencia: anote los parámetros explícitamente
```

### 8.6 Concatenación de strings

```hulk
function label(x: _): _ => "value = " @ x;
```

Este caso debe definirse con cuidado porque HULK permite concatenar strings con ciertos valores convertibles a string. Hay dos diseños posibles.

#### Diseño A: conservador

`@` produce `String`, pero no infiere un tipo único para un parámetro desconocido si ese parámetro podría ser más de un tipo imprimible.

Resultado:

```txt
retorno : String
x : no inferido
```

Error:

```txt
error: no se pudo inferir el tipo de x
motivo: x aparece como operando de @, pero @ acepta más de un tipo convertible a string
sugerencia: use x: String, x: Number o x: Object según intención
```

#### Diseño B: pragmático

Si el compilador decide que `@` acepta cualquier `Object`, entonces se puede inferir:

```txt
x : Object
retorno : String
```

Para el proyecto, se recomienda el **Diseño A** si quieren una inferencia más segura, o el **Diseño B** si ya modelaron `@` como conversión universal a string.

### 8.7 `let` con inicializador

```hulk
let x: _ = 42 in print(x);
```

Regla:

```txt
Tipo(x) = Tipo(inicializador)
```

Resultado:

```txt
x : Number
```

Ejemplo con dependencia entre bindings:

```hulk
let a: _ = 6, b: _ = a * 7 in print(b);
```

Inferencia:

```txt
a : Number
b : Number
```

Porque los bindings se procesan de izquierda a derecha.

### 8.8 `let` sin información suficiente

```hulk
function id(x: _) => x;
```

Si `x` no tiene usos restrictivos, no hay forma de saber si es `Number`, `String`, `Boolean`, `Object` o algún tipo definido por el usuario.

Debe fallar:

```txt
error: no se pudo inferir el tipo de x
motivo: el parámetro x no tiene usos que impongan un tipo concreto
sugerencia: escriba function id(x: Object): Object => x; o use un tipo más específico
```

### 8.9 Bloques de expresiones

```hulk
let x: _ = {
    print("calculating");
    40 + 2;
} in print(x);
```

Regla:

```txt
Tipo(block) = Tipo(última expresión del bloque)
```

Inferencia:

```txt
print("calculating") : Object o String/valor impreso según implementación
40 + 2               : Number
block                : Number
x                    : Number
```

Resultado:

```hulk
let x: Number = {
    print("calculating");
    40 + 2;
} in print(x);
```

### 8.10 Condicionales

```hulk
let x: _ = if (rand() < 0.5) 1 else 2 in print(x);
```

Regla:

```txt
condición debe ser Boolean
Tipo(if) = LCA(Tipo(then), Tipo(else))
```

Inferencia:

```txt
rand() < 0.5 : Boolean
then         : Number
else         : Number
if           : Number
x            : Number
```

Caso con jerarquía:

```hulk
type Animal {}
type Dog inherits Animal {}
type Cat inherits Animal {}

let pet: _ = if (rand() < 0.5) new Dog() else new Cat() in print(pet);
```

Inferencia:

```txt
new Dog() : Dog
new Cat() : Cat
LCA(Dog, Cat) = Animal
pet : Animal
```

### 8.11 `while`

```hulk
let x: _ = while (false) 42 in print(x);
```

Según la documentación, el tipo del `while` puede ser el tipo de su cuerpo. En ese caso:

```txt
false : Boolean
42    : Number
while : Number
x     : Number
```

Si el equipo decide que `while` retorna `Void` o `Object`, debe documentarlo y mantenerlo consistente. Para HULK, conviene seguir la documentación y tratarlo como el tipo del cuerpo.

### 8.12 `for`

```hulk
let result: _ = for (x in range(0, 10)) x * 2 in print(result);
```

Si `range(0, 10)` produce un iterable de números:

```txt
x : Number
x * 2 : Number
for : Number
result : Number
```

El `for` puede verse como azúcar de:

```hulk
let iterable = range(0, 10) in
    while (iterable.next())
        let x = iterable.current() in
            x * 2
```

### 8.13 Llamadas a funciones

```hulk
function double(x: Number): Number => x * 2;

let y: _ = double(21) in print(y);
```

Inferencia:

```txt
double(21) : Number
y : Number
```

Con retorno inferido:

```hulk
function double(x: Number): _ => x * 2;
```

Resultado:

```txt
retorno de double : Number
```

Con parámetro inferido:

```hulk
function double(x: _): Number => x * 2;
```

Resultado:

```txt
x : Number
```

### 8.14 Funciones recursivas

```hulk
function fib(n: _): _ =>
    if (n == 0 | n == 1) 1 else fib(n - 1) + fib(n - 2);
```

Inferencia esperada:

```txt
n == 0       fuerza n : Number
n - 1        fuerza n : Number
fib(n - 1) + fib(n - 2) produce Number
then branch  : Number
else branch  : Number
retorno fib  : Number
```

Resultado:

```hulk
function fib(n: Number): Number =>
    if (n == 0 | n == 1) 1 else fib(n - 1) + fib(n - 2);
```

Implementación recomendada:

1. Registrar primero la firma de la función con huecos.
2. Inferir el cuerpo generando restricciones.
3. Resolver los huecos.
4. Actualizar la firma.
5. Rechequear llamadas recursivas.

Si la recursión no permite resolver el retorno, se debe pedir anotación explícita.

### 8.15 Métodos y `self`

```hulk
type Counter(value: Number) {
    value: Number = value;

    inc(delta: _): _ => self.value := self.value + delta;
}
```

Inferencia:

```txt
self.value : Number
self.value + delta exige delta : Number
asignación destructiva a self.value exige Number
retorno de := : Number
```

Resultado:

```txt
delta : Number
inc : (Number) -> Number
```

### 8.16 Atributos

```hulk
type Point(x: Number, y: Number) {
    x: _ = x;
    y: auto = y;
}
```

Inferencia:

```txt
constructor x : Number
constructor y : Number
atributo x : Number
atributo y : Number
```

Caso con inicializador directo:

```hulk
type A {
    count: _ = 0;
    name: auto = "anonymous";
}
```

Resultado:

```txt
count : Number
name  : String
```

### 8.17 Argumentos de tipo / constructor

```hulk
type Range(min: _, max: _) {
    min: _ = min;
    max: _ = max;
    current: _ = min - 1;

    next(): _ => (self.current := self.current + 1) < self.max;
    current(): _ => self.current;
}
```

Inferencia:

```txt
min - 1 exige min : Number
self.current + 1 exige current : Number
self.current < self.max exige max : Number
next retorna Boolean
current retorna Number
```

Resultado:

```txt
min : Number
max : Number
attribute current : Number
method next : Boolean
method current : Number
```

### 8.18 Instanciación de tipos

```hulk
type Point(x: Number, y: Number) {
    x: Number = x;
    y: Number = y;
}

let p: _ = new Point(3, 4) in print(p);
```

Inferencia:

```txt
new Point(3, 4) : Point
p : Point
```

### 8.19 Acceso a atributos y métodos

```hulk
type Point(x: Number, y: Number) {
    x: Number = x;
    y: Number = y;

    norm(): Number => sqrt(self.x ^ 2 + self.y ^ 2);
}

let n: _ = new Point(3, 4).norm() in print(n);
```

Inferencia:

```txt
new Point(3, 4) : Point
Point.norm()    : Number
n               : Number
```

Caso problemático:

```hulk
function get_name(x: _) => x.name();
```

Si no se implementan protocolos sintéticos, esto debe fallar:

```txt
error: no se pudo inferir el tipo de x
restricción encontrada:
  - x debe tener método name()
motivo: no existe un único tipo nominal conocido que pueda inferirse solo por ese uso
sugerencia: anote x con un tipo concreto o implemente protocolos sintéticos
```

Esto mantiene el feature dentro de un alcance razonable.

### 8.20 `is` y `as`

```hulk
type Animal {}
type Dog inherits Animal {}

function is_dog(x: Animal): _ => x is Dog;
```

Inferencia:

```txt
x is Dog : Boolean
retorno : Boolean
```

Downcast:

```hulk
function as_dog(x: Animal): _ => x as Dog;
```

Inferencia:

```txt
x as Dog : Dog
retorno : Dog
```

---

## 9. Reglas de resolución de huecos

### 9.1 Modelo simple por restricciones

Cada `_` o `auto` genera una variable de tipo:

```txt
T1, T2, T3, ...
```

El recorrido de inferencia agrega restricciones:

```txt
T1 = Number
T2 conforms_to Object
T3 = LCA(Dog, Cat)
T4 must_have_method norm(): Number
```

Después se resuelven las restricciones:

1. Si el hueco tiene una única solución concreta, se reemplaza.
2. Si el hueco tiene varias soluciones posibles, falla por ambigüedad.
3. Si el hueco no tiene ninguna solución, falla por falta de información.
4. Si las restricciones se contradicen, falla por incompatibilidad.

### 9.2 Ejemplo de solución única

```hulk
function f(x: _): _ => x + 1;
```

Restricciones:

```txt
x debe ser Number
retorno debe ser Number
```

Solución:

```txt
x = Number
retorno = Number
```

### 9.3 Ejemplo de ambigüedad

```hulk
function id(x: _): _ => x;
```

Restricciones:

```txt
retorno = tipo(x)
```

No hay tipo concreto. Debe fallar:

```txt
error: no se pudo inferir el tipo de x
motivo: el hueco no tiene restricciones suficientes
```

### 9.4 Ejemplo de contradicción

```hulk
function bad(x: _): _ => {
    print(x + 1);
    !x;
};
```

Restricciones:

```txt
x + 1 exige x : Number
!x exige x : Boolean
```

Error:

```txt
error: restricciones incompatibles para x
  - uso aritmético exige Number
  - uso booleano exige Boolean
```

---

## 10. Modo normal vs restricted mode

### 10.1 Modo normal

En modo normal, el compilador aplica la inferencia estándar de HULK y además entiende `_` y `auto` como solicitudes explícitas de inferencia.

Válido:

```hulk
let x = 42 in print(x);
let y: _ = 42 in print(y);
let z: auto = 42 in print(z);
```

Los tres pueden inferirse como `Number`.

### 10.2 Restricted mode

El `restricted mode` es opcional. Se recomienda implementarlo como bandera de compilación:

```bash
hulk --restricted-inference file.hulk
```

O:

```bash
hulk --type-inference=restricted file.hulk
```

Semántica propuesta:

1. El compilador solo infiere tipos de símbolos cuando el programador lo pidió explícitamente con `_` o `auto`.
2. Las omisiones de tipo en posiciones importantes se rechazan.
3. La inferencia se limita a reglas locales y deterministas.
4. No se permite inferencia basada en protocolos sintéticos ni en heurísticas globales complejas.
5. Toda ambigüedad produce error.

### 10.3 Ejemplos en restricted mode

#### Aceptado

```hulk
let x: _ = 42 in print(x);
```

Resultado:

```txt
x : Number
```

#### Rechazado

```hulk
let x = 42 in print(x);
```

Error:

```txt
error: inferencia implícita deshabilitada en restricted mode
sugerencia: escriba let x: _ = 42 in ...
```

#### Aceptado

```hulk
function square(x: _): _ => x * x;
```

Resultado:

```txt
x : Number
retorno : Number
```

#### Rechazado

```hulk
function square(x) => x * x;
```

Error:

```txt
error: parámetro sin anotación en restricted mode
sugerencia: use x: _ o x: Number
```

#### Rechazado por ambigüedad

```hulk
function id(x: _): _ => x;
```

Error:

```txt
error: no se pudo inferir el tipo de x
motivo: el hueco no tiene restricciones suficientes
sugerencia: anote explícitamente el parámetro y el retorno
```

### 10.4 Por qué restricted mode ayuda

Este modo hace el feature más defendible porque:

- demuestra que `_` y `auto` no son solo decoración;
- fuerza una diferencia real entre “omití el tipo” y “quiero que lo infieras aquí”;
- simplifica pruebas;
- evita aceptar programas por inferencias demasiado agresivas;
- permite mostrar errores claros en la defensa.

---

## 11. Diagnósticos recomendados

La parte más original del feature puede ser la calidad de los mensajes de error.

### 11.1 Error por falta de información

Código:

```hulk
function id(x: _): _ => x;
```

Mensaje:

```txt
error [1:13 - 1:14]: no se pudo inferir el tipo de 'x'
  motivo: el parámetro no tiene usos que impongan un tipo concreto
  restricciones encontradas:
    - return_type = type(x)
  sugerencia: anote el parámetro explícitamente, por ejemplo x: Object
```

### 11.2 Error por conflicto

Código:

```hulk
function bad(x: _): _ => {
    print(x + 1);
    !x;
};
```

Mensaje:

```txt
error [1:14 - 1:15]: restricciones incompatibles para 'x'
  uso 1: x aparece en '+'; se esperaba Number
  uso 2: x aparece en '!'; se esperaba Boolean
```

### 11.3 Error por método desconocido

Código:

```hulk
function f(x: _) => x.foo();
```

Mensaje:

```txt
error [1:12 - 1:13]: no se pudo inferir el tipo de 'x'
  restricción encontrada:
    - x debe tener método foo()
  motivo: sin protocolos sintéticos, esa restricción no determina un tipo nominal único
  sugerencia: anote x con un tipo concreto que tenga foo()
```

### 11.4 Error en restricted mode

Código:

```hulk
let x = 42 in print(x);
```

Mensaje:

```txt
error [1:5 - 1:6]: inferencia implícita no permitida en restricted mode
  sugerencia: use 'let x: _ = 42 in ...' o 'let x: Number = 42 in ...'
```

---

## 12. Cambios de implementación

### 12.1 Lexer

Agregar reconocimiento de:

```txt
_      -> UNDERSCORE_TYPE o UNDERSCORE
auto   -> AUTO
```

Recomendación:

- `_` ya no es identificador válido como inicio de nombre, así que no debería romper nombres de usuario.
- `auto` puede tratarse como palabra reservada nueva, o como identificador especial solo en posición de tipo. Para simplificar el parser, conviene reservarlo.

### 12.2 Parser

Agregar alternativas en el parser de anotaciones de tipo:

```cpp
TypeAnnotation* parse_type_annotation() {
    if (match(TokenType::UNDERSCORE)) {
        return new InferTypeAnnotation(InferTypeSyntax::Underscore, previous().span);
    }

    if (match(TokenType::AUTO)) {
        return new InferTypeAnnotation(InferTypeSyntax::Auto, previous().span);
    }

    // resto: ID, ID[], ID*, function types...
}
```

### 12.3 AST

Agregar nodo nuevo:

```cpp
struct InferTypeAnnotation : TypeAnnotation {
    InferTypeSyntax syntax;
    Span span;
};
```

O, si el AST ya modela tipos como una variante:

```cpp
struct TypeRef {
    enum class Kind {
        Named,
        Vector,
        Iterable,
        Function,
        Infer
    } kind;
};
```

### 12.4 Representación de tipos internos

Agregar tipos internos para inferencia:

```cpp
struct HulkType {
    enum class Kind {
        Number,
        String,
        Boolean,
        Object,
        Named,
        Function,
        Vector,
        Iterable,
        TypeVar,
        Error
    } kind;

    int typevar_id; // solo si kind == TypeVar
};
```

### 12.5 Constraint collector

Crear un pase que recorra el AST y emita restricciones:

```cpp
struct TypeConstraint {
    enum class Kind {
        Equals,
        ConformsTo,
        HasMethod,
        HasAttribute
    } kind;

    HulkType left;
    HulkType right;
    Span span;
    std::string reason;
};
```

Ejemplos:

```txt
x + 1       -> constraint: type(x) == Number
!x          -> constraint: type(x) == Boolean
return expr -> constraint: type(expr) conforms_to declared_return
```

### 12.6 Resolver

El resolver procesa constraints y sustituye `TypeVar` por tipos concretos.

Reglas mínimas:

- `TypeVar == Number` resuelve a `Number`.
- `TypeVar == String` resuelve a `String`.
- `TypeVar == Boolean` resuelve a `Boolean`.
- `TypeVar == Named(T)` resuelve a `T`.
- Dos constraints incompatibles reportan error.
- Si un `TypeVar` queda sin resolver, reportar error.

### 12.7 Type checker

Después de la inferencia, el type checker trabaja como si todos los tipos hubieran sido escritos explícitamente.

Ejemplo:

```hulk
function square(x: _): _ => x * x;
```

Después de inferencia:

```hulk
function square(x: Number): Number => x * x;
```

El type checker solo verifica la versión resuelta.

---

## 13. Alcance recomendado para no complicarse demasiado

### 13.1 Sí implementar

- `_` y `auto` como alias de type hole.
- Inferencia de expresiones.
- Inferencia de variables de `let` desde inicializador.
- Inferencia de atributos desde inicializador.
- Inferencia de retornos desde cuerpo.
- Inferencia de parámetros por usos directos: aritmética, booleanos, comparaciones, llamadas a funciones conocidas.
- Inferencia de LCA para `if`.
- Diagnósticos de ambigüedad y conflicto.
- Restricted mode opcional.

### 13.2 No implementar en primera versión

- Protocolos sintéticos generados automáticamente.
- Inferencia Hindley-Milner completa.
- Inferencia de lambdas con closures.
- Inferencia global compleja por análisis de todos los call-sites.
- Sobrecarga de operadores.
- Tipos genéricos reales.

Esto mantiene la extensión implementable en el tiempo del proyecto.

---

## 14. Casos de prueba recomendados

### 14.1 Casos válidos

```hulk
let x: _ = 42 in print(x);
```

Esperado:

```txt
x : Number
```

```hulk
let s: auto = "hello" in print(s);
```

Esperado:

```txt
s : String
```

```hulk
function square(x: _): _ => x * x;
print(square(5));
```

Esperado:

```txt
x : Number
return : Number
```

```hulk
function not_value(x: auto): auto => !x;
print(not_value(false));
```

Esperado:

```txt
x : Boolean
return : Boolean
```

```hulk
type Point(x: _, y: _) {
    x: _ = x;
    y: _ = y;

    norm(): _ => sqrt(self.x ^ 2 + self.y ^ 2);
}

let p: _ = new Point(3, 4) in print(p.norm());
```

Esperado:

```txt
Point.x constructor arg : Number
Point.y constructor arg : Number
attribute x : Number
attribute y : Number
norm return : Number
p : Point
```

### 14.2 Casos inválidos

```hulk
function id(x: _): _ => x;
```

Esperado:

```txt
error: tipo de x ambiguo
```

```hulk
function bad(x: _): _ => {
    x + 1;
    !x;
};
```

Esperado:

```txt
error: x no puede ser Number y Boolean a la vez
```

```hulk
function f(x: _) => x.foo();
```

Esperado si no hay protocolos sintéticos:

```txt
error: método foo() no determina un tipo nominal único para x
```

```hulk
let x = 42 in print(x);
```

En modo normal: válido.

En restricted mode: error.

---

## 15. Argumento para defenderlo como feature elegido

Este feature es una buena elección porque:

1. Está alineado con la documentación oficial de HULK.
2. Extiende A.9 sin intentar resolver todo el problema general de inferencia.
3. Cumple el requisito de modificar sintaxis y semántica.
4. Es implementable sobre el pipeline que ya necesitan para binding, inferencia y type checking.
5. Permite mostrar demos claras con programas pequeños.
6. Tiene una parte original: `_`, `auto`, diagnósticos explicativos y restricted mode.
7. No obliga a tocar fuertemente el runtime ni el backend.

Comparado con protocolos, iterables, vectores, functors o macros, esta extensión tiene menor riesgo de romper muchas capas del compilador. Su complejidad está concentrada en el análisis semántico, que de todos modos deben implementar para llegar a type checking.

---

## 16. Riesgos y mitigaciones

| Riesgo | Mitigación |
|---|---|
| La inferencia de tipos general es demasiado compleja. | Limitar el alcance a restricciones locales y fallar ante ambigüedad. |
| `auto` puede chocar con un tipo llamado `auto`. | Reservar `auto` como palabra clave nueva o tratarlo como especial solo en posición de tipo. |
| Los parámetros sin uso son imposibles de inferir. | Reportar error y sugerir anotación explícita. |
| Métodos sobre receptores desconocidos requieren tipado estructural. | No inferirlos sin protocolos sintéticos; reportar una restricción `must_have_method`. |
| Recursión puede requerir varias pasadas. | Registrar firmas con huecos y hacer una segunda pasada; pedir anotación si no converge. |
| Mensajes de error pobres hacen que parezca que el feature falla. | Guardar razones y spans en cada constraint para diagnósticos explicativos. |

---

## 17. Definición de terminado

El feature se considera terminado cuando:

- El lexer reconoce `_` y `auto` en posiciones de tipo.
- El parser acepta `_` y `auto` en `let`, parámetros, retornos, atributos y argumentos de tipo.
- El AST representa esas anotaciones como huecos de tipo.
- El inferidor resuelve huecos en casos deterministas.
- El inferidor reporta errores claros en casos ambiguos o contradictorios.
- El type checker recibe el AST completamente anotado.
- Existe al menos una batería de tests válidos e inválidos.
- Opcionalmente, `--restricted-inference` rechaza inferencia implícita y exige `_` o `auto`.

---

## 18. Conclusión

La inferencia de tipos puede escogerse como feature adicional si se presenta como una extensión sintáctica explícita: `_` y `auto` como type holes. Esta modificación hace que el programador pueda indicar en qué puntos quiere delegar el tipo al compilador, mientras que el compilador conserva una semántica segura: resolver solo cuando hay una solución única, y fallar con diagnósticos útiles cuando no la hay.

La versión recomendada para el proyecto es:

```txt
Inferencia explícita con `_` y `auto` + diagnósticos de restricciones
```

Y, si queda tiempo:

```txt
Restricted mode: inferencia solo cuando se usa `_` o `auto`
```

Con esto el feature cumple el requisito de sintaxis y semántica, se diferencia de otros equipos y se mantiene dentro de un alcance razonable.
