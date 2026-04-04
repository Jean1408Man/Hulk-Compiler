# Informe de implementación del lexer de HULK

### Literales y nombres

- identificadores,
- números,
- strings,
- booleanos (`true`, `false`).

### Keywords del lenguaje base

- `let`, `in`
- `if`, `elif`, `else`
- `while`, `for`
- `function`
- `type`, `inherits`
- `new`
- `is`, `as`

### Builtins y constantes tratadas como keywords en esta implementación

Por decisión de diseño de esta versión, también se reservan léxicamente:

- `print`
- `sqrt`, `sin`, `cos`, `exp`, `log`, `rand`
- `PI`, `E`

### Operadores y signos

- aritméticos: `+`, `-`, `*`, `/`, `%`, `^`
- asignación y comparación: `=`, `:=`, `==`, `!=`, `<`, `<=`, `>`, `>=`
- booleanos: `&`, `|`, `!`
- concatenación: `@`
- inline functions: `=>`
- acceso y anotación: `.`, `:`
- delimitadores: `(`, `)`, `{`, `}`, `,`, `;`

### Trivia soportada

- espacios,
- tabs,
- saltos de línea,
- comentarios de línea con `//`.

## Decisiones de diseño principales

### 1. Implementación manual

El lexer se implementa **a mano**, sin generadores.

Esto permite:

- más control sobre el comportamiento,
- mejor comprensión del flujo del scanner,
- menor complejidad inicial,
- integración rápida con parser y AST.

### 2. Enfoque híbrido

La estrategia usada es una **vía híbrida**:

- casos simples se resuelven por categorías con código directo,
- casos delicados se implementan siguiendo la lógica de pequeños autómatas deterministas.


## Arquitectura utilizada

```text
src/
  common/
    position.hpp
    span.hpp
    diagnostic.hpp
  lexer/
    token_kind.hpp
    token.hpp
    cursor.hpp
    keywords.hpp
    lexer.hpp
    lexer.cpp
    main.cpp
```

### `common/position.hpp`

Define una posición en el archivo fuente:

- índice absoluto,
- línea,
- columna.

### `common/span.hpp`

Representa un rango dentro del archivo:

- posición inicial,
- posición final.

Se usa tanto para tokens como para errores.

### `common/diagnostic.hpp`

Modela diagnósticos léxicos:

- nivel,
- mensaje,
- span.

### `lexer/token_kind.hpp`

Contiene el `enum class TokenKind` con todas las categorías léxicas reconocidas.

### `lexer/token.hpp`

Define el token producido por el lexer:

- `kind`,
- `lexeme`,
- `span`.

### `lexer/cursor.hpp`

Encapsula el recorrido del input y evita mezclar posicionamiento con reconocimiento.

Responsabilidades:

- `peek()`,
- `advance()`,
- `match()`,
- consulta de posición,
- control de EOF.

### `lexer/keywords.hpp`

Tabla de palabras reservadas y símbolos predefinidos reservados por esta implementación.

### `lexer/lexer.hpp`

Interfaz principal del scanner.

Expone:

- `next_token()`,
- `tokenize()`,
- acceso a diagnósticos.

### `lexer/lexer.cpp`

Implementa la lógica real del reconocimiento léxico.

---

## Vía híbrida aplicada

La vía híbrida usada en este lexer mezcla dos estilos.

### A. Reconocimiento por categorías

Se usa en casos simples:

- identificadores / keywords,
- operadores simples,
- delimitadores,
- whitespace,
- comentario de línea.

Aquí el código se organiza por funciones especializadas y decisiones directas según el primer carácter.

### B. Reconocimiento con lógica de autómata

Se usa en casos donde conviene pensar en estados:

- números,
- strings,
- operadores compuestos con *longest-match*.

---

## Autómatas implementados

Aunque no todos están escritos con un `enum State`, sí existen como autómatas deterministas implícitos en el código.

### 1. Identificadores / keywords

Regla:

- el identificador debe empezar por letra,
- luego puede contener letras, dígitos o `_`.

Esquema:

```text
S0 --[letter]--> S1
S1 --[letter|digit|_]--> S1
S1 --[otro]--> ACCEPT
```

Después de aceptar el lexema, se consulta la tabla de keywords.

### 2. Números

Soporta:

- enteros,
- decimales simples.

Esquema:

```text
S0 --[digit]--> S1
S1 --[digit]--> S1
S1 --[.]--> S2
S2 --[digit]--> S3
S3 --[digit]--> S3
```

La parte decimal solo se activa si el punto está seguido por un dígito.

### 3. Strings

Soporta:

- comilla de apertura,
- contenido normal,
- escapes,
- cierre correcto,
- error por newline o EOF sin cierre.

Esquema:

```text
S0 --["]--> S1
S1 --[\\]--> S2
S1 --["]--> ACCEPT
S1 --[otro]--> S1
S2 --[char escapado]--> S1
```

### 4. Operadores y delimitadores

Se usa reconocimiento con prioridad al lexema más largo.

Primero se intentan operadores compuestos:

- `:=`
- `==`
- `!=`
- `<=`
- `>=`
- `=>`

Si no aplica ninguno, se consume un solo símbolo y se clasifica por `switch`.

### 5. Trivia

Whitespace y comentario de línea se consumen sin generar token.

---

## Flujo general del lexer

La función central sigue esta secuencia:

1. saltar whitespace y comentarios,
2. si EOF: devolver `EndOfFile`,
3. si empieza por letra: escanear identificador/keyword,
4. si empieza por dígito: escanear número,
5. si empieza por `"`: escanear string,
6. en otro caso: escanear operador o delimitador.

Este flujo mantiene el lexer claro, corto y fácil de extender.

---

## Manejo de errores

Cuando aparece un carácter inválido o un string mal formado:

- se crea un `Diagnostic`,
- se devuelve un token `Error` con su `span`.

Esto permite:

- informar bien el problema,
- depurar con facilidad,
- decidir más adelante si el parser aborta o intenta recuperación.
