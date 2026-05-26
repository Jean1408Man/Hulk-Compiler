# Tests del parser actual

Estos tests están contrastados contra `doc/hulk-docs.pdf`, pero representan solo el subconjunto
del lenguaje que el parser actual implementa.

## Sintaxis confirmada en la documentación

El PDF confirma, entre otras, estas construcciones:

- literales numéricos, strings y booleanos;
- `print(expr)`;
- builtins `sqrt`, `sin`, `cos`, `exp`, `log`, `rand`;
- operadores `+`, `-`, `*`, `/`, `%`, `^`;
- comparaciones `<`, `>`, `<=`, `>=`, `==`, `!=`;
- booleanos `&`, `|`, `!`;
- concatenación `@`;
- bloques `{ expr; expr; ... }`;
- `let ... in ...`;
- asignación destructiva `:=`;
- `if / elif / else`;
- `while`;
- `for`;
- funciones y tipos.

## Subconjunto que sí soporta hoy el parser

Por ahora el parser implementa y prueba:

- literales;
- `print(expr)`;
- `sqrt`, `sin`, `cos`, `exp`, `log`, `rand`, `PI`, `E`;
- `+`, `-`, `*`, `/`, `%`, `^`;
- `@` y `@@`;
- `==`, `!=`, `<`, `<=`, `>`, `>=`;
- `&`, `|`, `!`;
- `let`;
- `:=`;
- bloques de expresiones.
- `if / elif / else`;
- `while`;
- `for`;
- funciones;
- tipos;
- protocolos;
- `extends` en protocolos;
- anotaciones de iterables tipados `T*`, incluyendo estrellas anidadas;
- acceso a miembros, llamadas a métodos, `new`, `is`, `as`;
- una única expresión global final, con bloques explícitos para secuencias.

## Sintaxis documentada pero aún no soportada por este parser

- tipos extendidos como `T[]` o tipos de functor;
- vectores explícitos e implícitos;
- lambdas;
- macros y pattern matching.

## Nota importante

El parser acepta `;` opcional tras la expresión global final. Si un ejemplo
necesita ejecutar varias expresiones al final del programa, los tests usan un
bloque explícito `{ ... }` para mantener el contrato `decl* expr`.
