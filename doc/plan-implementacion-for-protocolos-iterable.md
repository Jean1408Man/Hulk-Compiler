# Plan de implementacion: `for` con protocolos reales, `Iterable` builtin y `Range`

Fecha: 2026-05-26

## Objetivo

Incorporar `for` al conjunto soportado end-to-end de HULK sin implementar
vectores, comprehensions, `Enumerable` ni `T*` en este corte.

La ruta elegida es la ruta oficial del lenguaje:

- los protocolos son contratos estructurales;
- un tipo implementa un protocolo implicitamente si contiene los metodos
  requeridos con firmas compatibles;
- `Iterable` es un protocolo builtin;
- `Range` es un tipo builtin interno que conforma a `Iterable`;
- `range(Number, Number)` construye un `Range`;
- `for` itera llamando `next()` y `current()`.

## Fase 1: base semantica de protocolos

1. Crear `SemanticProtocolInfo` y `SemanticProtocolMethodInfo`.
2. Agregar tabla de protocolos a `SemanticTables`.
3. Agregar APIs de registro, busqueda y conformidad:
   - `register_protocol`;
   - `lookup_protocol`;
   - `is_protocol`;
   - `find_protocol_method`;
   - `type_conforms_to_protocol`;
   - `protocol_conforms_to_protocol`.
4. Implementar varianza:
   - parametros contravariantes;
   - retornos covariantes.
5. Implementar herencia de protocolos con `extends`.
6. Detectar ciclos de protocolos.
7. Detectar redefiniciones heredadas incompatibles.

## Fase 2: parser y AST de protocolos

1. Agregar token/keyword `extends`.
2. Extender la gramatica:

```bnf
protocol_decl ::= "protocol" IDENT protocol_extends_opt "{" protocol_member* "}"
protocol_extends_opt ::= "extends" IDENT | epsilon
protocol_member ::= IDENT "(" params_opt ")" ":" type_expr ";"
```

3. Reusar `ProtocolDecl` y su `parentName`.
4. Regenerar parser con `make parser-gen`.
5. Dejar las validaciones de tipos, retornos y parametros en semantica.

## Fase 3: registro y validacion de protocolos de usuario

1. Registrar `ProtocolDecl` en `SymbolResolver`.
2. Bloquear duplicados contra tipos, funciones, protocolos y builtins.
3. Validar metodos duplicados dentro del protocolo.
4. Validar que el padre de `extends` exista y sea protocolo.
5. Permitir protocolos en anotaciones:

```hulk
let x: Named = value in x;
function f(x: Iterable): Object => x.current();
```

6. Rechazar protocolos como entidades runtime:

```hulk
new Iterable()
x is Iterable
x as Iterable
```

## Fase 4: `Iterable`, `Range` y `range`

1. Registrar `Iterable` como protocolo builtin:

```hulk
protocol Iterable {
    next(): Boolean;
    current(): Object;
}
```

2. Registrar `Range` como tipo builtin interno con:
   - campos `current: Number` y `max: Number`;
   - metodo `next(): Boolean`;
   - metodo `current(): Number`.
3. Registrar `range(Number, Number): Range`.
4. Bloquear redeclaracion de `Iterable`, `Range` y `range`.
5. Bloquear construccion directa `new Range(...)`.
6. Verificar que `Range` conforma a `Iterable` con el mecanismo general de
   protocolos, no con un caso especial.

## Fase 5: inferencia y typecheck

1. Extender `HulkType::conforms_to` para reconocer protocolos.
2. Resolver llamadas de metodo sobre receptores tipados como protocolo usando
   la firma del protocolo.
3. Mantener llamadas de metodo sobre tipos concretos con retorno concreto.
4. En `for`:
   - inferir una vez el tipo del iterable;
   - exigir conformidad con `Iterable`;
   - exigir `next(): Boolean`;
   - usar el retorno concreto de `current()` como tipo de la variable.
5. Si el iterable esta anotado como `Iterable`, la variable del `for` queda
   como `Object`.
6. Si el iterable es `Range`, la variable queda como `Number`.

## Fase 6: variable sintetica del `for`

1. Registrar en binding una variable sintetica para el simbolo del bucle.
2. Guardar esa resolucion en `resolution_map[For*]`.
3. Hacer que inferencia asocie el tipo de `current()` a ese simbolo sintetico.
4. Hacer que typecheck use ese tipo al validar referencias a la variable.

## Fase 7: backend de protocolos

1. Tratar `ProtocolDecl` como no-op en IRGen.
2. No emitir metadata runtime para protocolos.
3. Bajar llamadas sobre valores protocolados como llamadas virtuales normales
   por nombre de metodo.
4. Mantener la seguridad de protocolos cerrada antes del backend.

## Fase 8: backend de `Range` y `range`

1. Emitir metadata IR para `Range` solo cuando el programa usa `range`.
2. Emitir funciones sinteticas:
   - `Range.next`;
   - `Range.current`.
3. Bajar `range(a, b)` a IR existente:

```text
obj = NewObject Range
current0 = a - 1
SetField obj.current = current0
SetField obj.max = b
result = obj
```

4. Bajar `Range.next` a:

```text
current = self.current + 1
self.current = current
return current < self.max
```

5. Bajar `Range.current` a:

```text
return self.current
```

6. No agregar opcode VM para `range`.

## Fase 9: backend de `for`

1. Bajar `For` a control explicito:

```text
iterable = lower(iterable_expr)
result = nil

loop:
    has_next = iterable.next()
    if_false has_next goto end

    current = iterable.current()
    x = current

    body_value = lower(body)
    result = body_value
    goto loop

end:
    return result
```

2. Crear un local backend para la variable sintetica.
3. Enlazar ese local en `CodegenContext` durante el body.
4. El resultado de un `for` sin iteraciones es `nil`, igual que `while`.

## Fase 10: evaluador legacy

1. `ProtocolDecl` es no-op.
2. `range` crea un objeto runtime interno `Range`.
3. `Range.next/current` se ejecutan como metodos nativos.
4. `For` usa `next/current`, crea scope local para la variable y retorna el
   ultimo valor del body.

## Fase 11: tests

Agregar cobertura para:

- parser de protocolos simples;
- parser de `protocol P extends Q`;
- parser de `for (x in range(0, 3)) print(x);`;
- tipo que conforma a protocolo de usuario;
- protocolo extendiendo otro protocolo;
- `Range` conformando a `Iterable`;
- funcion que recibe `Iterable`;
- `for` sobre `range`;
- `for` sobre iterable definido por usuario;
- invalidos por no iterable, falta de `next/current`, firma incorrecta,
  ciclos de protocolos, overrides incompatibles, redeclaraciones reservadas,
  `new Range`, `is Iterable` y `as Iterable`.

## Fase 12: documentacion

1. Crear este plan en `doc/plan-implementacion-for-protocolos-iterable.md`.
2. Actualizar `doc/informe-for-iterables-hulk.md`.
3. Actualizar `doc/informe-estructuras-lenguaje-for-iterables.md`.
4. Actualizar `doc/informe-vulnerabilidades-resueltas-chat.md`.

## Verificacion esperada

Ejecutar:

```bash
make parser-gen
make parser-sync-check
make parser-demo
make semantic
make run-tests
make backend-tests
make vm-tests
git diff --check
```

Los criterios de aceptacion son:

- se pueden declarar protocolos propios;
- se puede extender un protocolo;
- un tipo implementa un protocolo implicitamente por firma;
- se puede anotar un valor como protocolo;
- `Range` implementa `Iterable` mediante el mecanismo general;
- `range(0, 3)` retorna `Range`;
- `for` funciona sobre `range`;
- `for` funciona sobre iterables creados por el usuario;
- protocolos no generan estructuras runtime;
- `lambda` queda fuera del subconjunto soportado y se rechaza en frontend;
- no se implementan vectores, comprehensions, `Enumerable` ni `T*`.
