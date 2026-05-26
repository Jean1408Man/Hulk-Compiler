# Nota tecnica: protocolos HULK A.10

Fecha: 2026-05-26

## Estado

El nucleo A.10 de protocolos queda soportado como feature de compilacion:

- `protocol P { ... }`;
- `protocol P extends Q { ... }`;
- conformidad estructural implicita;
- varianza en implementacion y extension de protocolos;
- uso de protocolos en anotaciones;
- llamadas de metodos sobre valores tipados como protocolo;
- iterables tipados `T*` como protocolos sinteticos internos.

Los protocolos no existen en runtime. Despues del typecheck, `ProtocolDecl` no
genera metadata, objetos, vtables ni instrucciones especiales.

## Conformidad

Un tipo conforma a un protocolo cuando contiene todos los metodos requeridos con
firmas compatibles. La compatibilidad se calcula asi:

- parametros contravariantes: el metodo real puede aceptar el mismo tipo o un
  supertipo;
- retorno covariante: el metodo real puede devolver el mismo tipo o un subtipo;
- si una firma concreta no esta anotada, el typechecker usa la firma inferida
  siempre que sea determinable.

Si una parte necesaria de la firma concreta queda ambigua, el programa falla y
debe anotar el metodo.

## Iterables tipados `T*`

La notacion `T*` queda soportada como azucar semantico para un protocolo
sintetico interno:

```hulk
protocol T* extends Iterable {
    current(): T;
}
```

El nombre sintetico no existe en runtime y no puede usarse con `is` ni `as`.
Sirve para anotar parametros, variables, atributos y retornos cuando se necesita
preservar el tipo del elemento dentro de un `for`.

La notacion es recursiva. Por ejemplo, `Number**` representa un iterable cuyos
elementos son `Number*`, y el compilador registra la cadena de protocolos
sinteticos necesaria.

## Fuera de alcance

Este cierre no implementa las features posteriores que usan protocolos como
base:

- `Enumerable`;
- vectores;
- comprehensions;
- functors;
- lambdas;
- macros.
