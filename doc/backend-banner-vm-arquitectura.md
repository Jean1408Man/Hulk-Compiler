# Backend BannerIR + BannerVM

Este documento explica el backend actual de HULK por capas. La idea es leerlo como una guia desde cero: que entra al backend, que se transforma, por que se transforma, que ejecuta la VM y que decisiones de diseno se tomaron para que el flujo sea un backend propio y no una traduccion a C++.

El flujo final implementado es:

```text
HULK fuente
  -> Lexer / Parser
  -> AST
  -> SemanticAnalyzer
  -> HulkIR
  -> BannerIR
  -> BannerVM
  -> salida del programa
```

El backend ya no genera C++ para ejecutar el programa. El programa se baja a una IR propia, llamada BannerIR, y esa IR se ejecuta en una maquina virtual escrita en C++.

## 1. Que problema resuelve el backend

Un compilador no ejecuta directamente el texto fuente. Primero convierte el programa a representaciones internas cada vez mas simples.

El codigo fuente de HULK tiene conceptos de alto nivel:

- expresiones anidadas;
- `let`;
- `if`;
- `while`;
- funciones;
- llamadas recursivas;
- tipos;
- objetos;
- herencia;
- metodos virtuales;
- `self`;
- `base`;
- casts con `is` y `as`.

Una maquina virtual no deberia ejecutar directamente esos conceptos como arboles del AST. La VM debe ejecutar instrucciones simples, parecidas a un ensamblador de alto nivel:

```text
t0 = const_number 2
t1 = const_number 3
t2 = add t0 t1
jump_if_false t2 L_end
return t2
```

Por eso el backend tiene varias capas. Cada capa elimina un poco mas de complejidad del lenguaje fuente hasta que queda una secuencia lineal de instrucciones.

## 2. Capa de entrada: BackendDriver

El punto de entrada del backend esta en `BackendDriver`.

Su trabajo es coordinar el pipeline completo:

```text
leer archivo
  -> parsear
  -> analizar semanticamente
  -> generar HulkIR
  -> opcionalmente imprimir HulkIR
  -> bajar a BannerIR
  -> opcionalmente imprimir BannerIR
  -> ejecutar BannerVM
```

El driver no ejecuta instrucciones de HULK por si mismo. Solo orquesta las fases.

Las opciones actuales importantes son:

- `--emit-ir`: imprime HulkIR y termina.
- `--emit-banner`: imprime BannerIR y termina.
- sin opcion de emision: ejecuta en BannerVM.
- `--run-banner`: queda como forma explicita de decir que se quiere ejecutar con la VM.

Decision de diseno:

Antes existia un backend temporal que generaba C++20. Ese camino se retiro para que el backend final sea realmente propio:

```text
HulkIR -> BannerIR -> BannerVM
```

Esto evita depender de `g++` para ejecutar programas HULK.

## 3. AST y semantica

El AST representa la estructura del programa fuente. Por ejemplo, una expresion:

```hulk
(2 + 3) * 4
```

en el AST sigue teniendo forma de arbol:

```text
        *
      /   \
     +     4
   /   \
  2     3
```

Ese arbol es util para analizar el programa, pero no es una buena forma para una VM de bajo nivel.

Antes de llegar al backend ejecutable, el `SemanticAnalyzer` resuelve y valida reglas del lenguaje:

- si las variables existen;
- si las funciones existen;
- si la aridad de llamadas es correcta;
- si los tipos son compatibles;
- si un metodo existe;
- si `self` y `base` se usan en lugares validos;
- si `is` y `as` son plausibles.

Decision de diseno:

La VM no debe reimplementar estas reglas. La VM ejecuta una IR ya validada. Puede tener chequeos defensivos de runtime, pero no debe volver a hacer inferencia, resolucion de scopes ni type checking completo.

## 4. HulkIR: IR media

`HulkIR` es una representacion intermedia de nivel medio.

Su objetivo principal es eliminar el AST como forma de ejecucion. En esta capa, las expresiones complejas se descomponen en temporales y las estructuras de control se convierten en labels y saltos.

Ejemplo conceptual:

```hulk
print((2 + 3) * 4);
```

se baja a algo similar a:

```text
hulk_tmp_number_0 = const_number 2
hulk_tmp_number_1 = const_number 3
hulk_tmp_arith_2 = add hulk_tmp_number_0 hulk_tmp_number_1
hulk_tmp_number_3 = const_number 4
hulk_tmp_arith_4 = mul hulk_tmp_arith_2 hulk_tmp_number_3
hulk_tmp_print_5 = builtin_print(hulk_tmp_arith_4)
return hulk_tmp_print_5
```

Esto es importante porque la VM no ejecuta un arbol de expresiones. Ejecuta instrucciones lineales.

### Temporales

Cada resultado intermedio se guarda en un temporal. Eso permite convertir una expresion anidada en una secuencia de asignaciones.

Por ejemplo:

```text
a + b * c
```

se convierte en:

```text
t0 = mul b c
t1 = add a t0
```

La VM no necesita saber que en el codigo fuente habia una expresion anidada. Solo ve `mul` y luego `add`.

### Control de flujo

Un `if` tampoco se ejecuta como nodo de AST. Se baja a saltos:

```text
jump_if_false cond L_else
then_value = ...
jump L_end
label L_else
else_value = ...
label L_end
```

Un `while` se baja a:

```text
label L_start
cond = ...
jump_if_false cond L_end
body = ...
jump L_start
label L_end
```

Decision de diseno:

Mantener HulkIR como IR media evita que BannerIR tenga que conocer el AST. HulkIR es comoda para generarse desde el frontend y suficientemente estructurada para luego bajarse a una IR mas ejecutable.

## 5. BannerIR: IR baja ejecutable

`BannerIR` es la IR baja del backend. Es la que ejecuta la VM, directamente o despues de compilarla a una forma interna indexada.

BannerIR se organiza en tres secciones:

```text
.TYPES
.DATA
.CODE
```

### `.TYPES`

Describe los tipos como layouts fisicos:

- campos;
- slots de campos;
- metodos;
- slots de metodos;
- herencia ya aplanada;
- funcion concreta que implementa cada metodo.

El objetivo es que la VM no tenga que entender una clase fuente como estructura de arbol. La informacion ya viene preparada.

Ejemplo conceptual:

```text
type Dog parent Animal {
  field name slot 0
  method speak -> hulk_method_Dog_speak slot 0
}
```

### `.DATA`

Guarda datos estaticos, sobre todo strings literales:

```text
s0 = "hola"
s1 = "mundo"
```

En ejecucion, esos strings se cargan como referencias al heap de la VM.

### `.CODE`

Contiene funciones con parametros, locales e instrucciones.

Ejemplo:

```text
function hulk_main() ; kind=entry
{
  local hulk_tmp_number_0
  local hulk_tmp_number_1
  local hulk_tmp_arith_2
  hulk_tmp_number_0 = const_number 2
  hulk_tmp_number_1 = const_number 3
  hulk_tmp_arith_2 = add hulk_tmp_number_0 hulk_tmp_number_1
  return hulk_tmp_arith_2
}
```

Decision de diseno:

BannerIR conserva nombres legibles para poder imprimirla y depurarla. Antes de ejecutar, la VM compila esos nombres a slots numericos internos.

## 6. HulkIRToBanner: lowering de IR media a IR baja

`HulkIRToBanner` convierte HulkIR a BannerIR.

No hace analisis semantico. Su trabajo es mecanico:

- copiar funciones;
- copiar locales;
- bajar instrucciones una por una;
- bajar llamadas con argumentos a `PARAM` + `CALL`;
- convertir `NewObject` a `ALLOCATE`;
- convertir `GetField` a `GETATTR`;
- convertir `SetField` y `DefineField` a `SETATTR`;
- convertir `VCall` a `PARAM*` + `VCALL`;
- convertir `SCallMethod` a `PARAM*` + `SCALL`;
- aplanar tipos.

Ejemplo de llamada:

En HulkIR puede existir:

```text
t2 = call f(t0, t1)
```

En BannerIR se baja a:

```text
param t0
param t1
t2 = call f
```

Decision de diseno:

Usar `PARAM` separa el paso de argumentos de la instruccion `CALL`. Esto se parece mas a una convencion de llamada de maquina virtual: primero se preparan argumentos, luego se transfiere control a la funcion.

## 7. Aplanamiento de tipos

La herencia es un concepto de alto nivel. Una VM no deberia recorrer un arbol de clases cada vez que accede a un campo o llama un metodo.

Por eso el lowerer aplana la jerarquia.

Si existe:

```text
Animal {
  field name
  method speak
}

Dog : Animal {
  method speak
}
```

el layout de `Dog` queda como:

```text
fields:
  slot 0 -> name

vtable:
  slot 0 -> Dog.speak
```

El override no crea un slot nuevo. Reemplaza la funcion en el mismo slot.

Decision de diseno:

El slot de un metodo debe ser estable en toda la jerarquia. Si `Animal.speak` esta en slot 0, `Dog.speak` tambien debe estar en slot 0. Asi una llamada virtual puede hacer:

```text
function_id = object.type.vtable[0]
```

sin preguntar si el objeto es `Animal`, `Dog`, `Cat`, etc.

## 8. BannerVM: modelo general

`BannerVM` es la maquina virtual propia del backend.

Tiene:

- un heap;
- un stack de frames;
- un program counter por frame;
- slots locales por frame;
- un buffer de parametros por frame;
- una tabla de funciones compiladas;
- una tabla de tipos compilados;
- instrucciones compiladas a indices numericos.

El loop principal tiene esta forma:

```cpp
while (!stack.empty()) {
    Frame& frame = stack.back();
    const auto& instr = code[frame.pc++];

    switch (instr.op) {
        case Add:
            ...
        case Jump:
            ...
        case Call:
            ...
        case Return:
            ...
    }
}
```

Esto es importante: la VM no visita nodos del AST. Tampoco llama recursivamente a funciones de HULK usando la pila de C++. Las llamadas HULK se ejecutan con frames propios de la VM.

## 9. CompiledProgram: fase de carga interna

BannerIR textual es legible, pero todavia contiene nombres:

- nombres de locales;
- nombres de labels;
- nombres de funciones;
- nombres de tipos;
- nombres de campos;
- nombres de metodos.

Antes de ejecutar, `BannerVM` crea un `CompiledProgram`.

Esta fase convierte:

```text
local name -> slot numerico
label name -> pc numerico
function name -> function_id
type name -> type_id
data label -> string_ref
```

Por ejemplo:

```text
hulk_tmp_number_0 -> slot 0
hulk_tmp_number_1 -> slot 1
L_if_end_0        -> pc 17
hulk_fn_f_0       -> function_id 2
```

Decision de diseno:

La IR impresa sigue siendo facil de leer, pero el loop de ejecucion trabaja con indices. Esto mantiene buen balance entre depuracion y bajo nivel.

## 10. Frames y llamadas

Un frame representa una llamada activa a una funcion.

Contiene:

```text
function
pc
slots
param_buffer
return_slot
```

### Slots

Cada parametro, local y temporal vive en un slot numerico.

Antes de ejecutar:

```text
hulk_param_x_0 -> slot 0
hulk_tmp_add_1 -> slot 1
```

Durante ejecucion:

```text
slots[0]
slots[1]
```

La VM ya no necesita buscar el nombre de la variable en cada operacion aritmetica.

### CALL

Una llamada normal funciona asi:

```text
param arg0
param arg1
call function
```

La VM:

1. guarda los valores de `PARAM` en el `param_buffer`;
2. encuentra la funcion destino por `function_id`;
3. crea un nuevo frame;
4. copia los argumentos a los slots de parametros del nuevo frame;
5. apila el frame;
6. al hacer `RETURN`, desapila el frame y escribe el resultado en el slot de retorno del caller.

Decision de diseno:

La recursion de HULK no usa recursion de C++. Usa el stack de la VM. Eso hace que la VM controle sus frames, parametros y retornos.

## 11. Control de flujo

`if` y `while` ya llegaron convertidos a saltos.

La VM solo ejecuta:

- `JUMP`;
- `JUMP_IF_TRUE`;
- `JUMP_IF_FALSE`;
- `LABEL` como marcador.

Ejemplo de `while`:

```text
label L_start
cond = ...
jump_if_false cond L_end
body = ...
jump L_start
label L_end
```

Durante la fase de compilacion interna:

```text
L_start -> pc 3
L_end   -> pc 12
```

Durante ejecucion:

```text
frame.pc = 3
```

No hay un nodo `While` dentro de la VM.

## 12. Word: representacion uniforme de valores

La VM usa:

```cpp
using Word = std::uint64_t;
```

Una `Word` puede representar:

- number;
- nil;
- bool;
- string_ref;
- object_ref.

Los numeros se guardan como bits de `double`. Los otros valores usan tags internos y un payload.

Ejemplo conceptual:

```text
Word(number 3.14)
Word(bool true)
Word(nil)
Word(string_ref 4)
Word(object_ref 2)
```

Decision de diseno:

Antes la VM usaba `std::variant`, `std::string` y `shared_ptr`. Eso era facil de programar, pero no era tan parecido a una maquina virtual de bajo nivel.

Con `Word`:

- los slots son `vector<Word>`;
- los campos son `vector<Word>`;
- los argumentos son `vector<Word>`;
- los retornos son `Word`;
- copiar valores es copiar 64 bits;
- objetos y strings viven en el heap.

Esto separa mejor el valor runtime de las estructuras de C++.

## 13. Heap de la VM

El heap actual guarda:

```text
objects_: vector<VMObject>
strings_: vector<VMString>
```

Un objeto es:

```text
type_id
fields: vector<Word>
```

Un string es:

```text
value: std::string
```

### Objetos

`ALLOCATE` crea un objeto:

```text
obj = allocate type_id, field_count
```

La VM reserva un `VMObject`, inicializa sus campos a `nil` y devuelve una `Word` de tipo `object_ref`.

### Strings

`LOAD_DATA` carga un string estatico desde `.DATA` y devuelve un `string_ref`.

`CONCAT` crea un string nuevo en heap:

```text
result = concat a b
```

La VM convierte los operandos a texto, concatena y guarda el resultado como nuevo `VMString`.

### Limpieza de memoria

Actualmente no hay GC.

La politica es arena de vida completa:

```text
durante la ejecucion: no se libera nada individualmente
al terminar run(): se libera todo el heap junto
```

Esto es suficiente para los programas actuales. Para programas largos, el siguiente paso seria un GC, probablemente mark-and-sweep.

Decision de diseno:

No implementar GC todavia reduce riesgo. Pero al tener objetos y strings centralizados en `VMHeap`, queda una base clara para agregar recoleccion despues.

## 14. Objetos y campos

Un objeto no guarda un mapa de campos por nombre.

Guarda un vector:

```text
fields[0]
fields[1]
fields[2]
```

El significado de cada indice lo define el layout del tipo.

Ejemplo:

```text
Point {
  x slot 0
  y slot 1
}
```

Entonces:

```text
point.fields[0] -> x
point.fields[1] -> y
```

`GETATTR` toma:

- el objeto;
- el slot del campo;
- el slot destino.

Y hace:

```text
dest = object.fields[field_slot]
```

Decision de diseno:

Usar slots hace que el acceso a campos sea mucho mas cercano a una VM real. Evita depender de `unordered_map<string, value>` dentro de cada objeto.

Nota tecnica:

La VM todavia conserva un fallback por nombre para algunos casos donde no puede probar que un nombre de campo tiene siempre el mismo slot global. Ese fallback no recorre un arbol de clases; busca en el layout del tipo runtime. Es una parte mejorable si se quiere una VM aun mas estricta.

## 15. Metodos virtuales y vtables

Una llamada virtual no usa dispatch virtual de C++.

El objeto de la VM no es una clase C++ con metodos virtuales. Es solo:

```text
type_id
fields
```

Para llamar un metodo virtual, la VM hace:

```text
receiver -> type_id -> runtime_type -> vtable[method_slot] -> function_id
```

Luego crea un frame para esa funcion.

Ejemplo:

```hulk
animal.speak()
```

se baja a una instruccion `VCALL`.

En ejecucion:

1. la VM toma el receiver;
2. lee su `type_id`;
3. obtiene el tipo runtime;
4. usa el slot del metodo;
5. lee la funcion concreta desde la vtable;
6. crea un frame;
7. pasa `self` como primer argumento.

Decision de diseno:

La llamada virtual se implementa manualmente. No depende de C++ ni de un recorrido de herencia en ejecucion. La herencia ya fue aplanada antes.

Nota tecnica:

Igual que con campos, todavia hay fallback por nombre para metodos cuando no se pudo precalcular un slot seguro. El caso ideal final es que todo `VCALL` llegue con `method_slot` numerico.

## 16. `base` y llamadas estaticas

`base()` no debe hacer dispatch virtual normal.

Si un metodo de una clase hija llama a `base()`, la llamada debe ir al metodo del padre, no al override del hijo.

Por eso existe `SCALL`.

`SCALL` usa:

- el receiver actual;
- el tipo desde el cual empezar;
- el slot del metodo;
- la vtable de ese tipo base.

Decision de diseno:

Separar `VCALL` y `SCALL` evita mezclar dispatch virtual normal con llamada explicita a implementacion de padre.

## 17. `is` y `as`

`IS_TYPE` responde si un valor pertenece a un tipo.

Para primitivos:

```text
Number
String
Boolean
```

la VM revisa el tag de la `Word`.

Para objetos:

1. toma el `type_id` del objeto;
2. busca el tipo runtime;
3. sube por `parent_type_id`;
4. compara nombres de tipo.

`AS_TYPE` hace lo mismo, pero si falla lanza error runtime.

Decision de diseno:

Aunque la semantica intenta garantizar casts plausibles, la VM conserva una validacion runtime porque `as` puede fallar dependiendo del tipo dinamico del objeto.

## 18. Builtins

Los builtins soportados por la VM incluyen:

- `print`;
- `sqrt`;
- `sin`;
- `cos`;
- `exp`;
- `log`;
- `rand`.

Estos se ejecutan como instrucciones de VM:

```text
PRINT
SQRT
SIN
COS
EXP
LOG
RAND
```

Decision de diseno:

No se implementan como funciones HULK normales porque son primitivas del runtime. Aun asi, desde BannerIR se ven como instrucciones explicitas, no como llamadas a C++ generadas.

## 19. Que no hace la VM

La VM no hace:

- parsing;
- analisis lexico;
- analisis sintactico;
- resolucion de nombres del lenguaje fuente;
- inferencia de tipos;
- type checking completo;
- ejecucion de AST;
- transformacion de expresiones complejas;
- generacion de temporales;
- conversion de `if` o `while` desde AST.

Todo eso ocurre antes.

La VM solo ejecuta BannerIR compilada a una forma interna mas numerica.

## 20. Por que hay dos IRs

Podria parecer mas simple tener solo una IR, pero dos niveles ayudan:

```text
AST -> HulkIR -> BannerIR -> VM
```

`HulkIR` es conveniente para el compilador:

- conserva nombres legibles;
- se conecta bien con el AST;
- facilita imprimir snapshots;
- expresa operaciones del lenguaje sin estar atada a una VM concreta.

`BannerIR` es conveniente para ejecucion:

- usa instrucciones mas cercanas a VM;
- baja llamadas a `PARAM` + `CALL`;
- contiene layouts aplanados;
- se puede compilar a slots e ids numericos;
- podria servir despues para otro backend de bajo nivel.

Decision de diseno:

Separar IR media e IR baja evita meter detalles de la VM dentro de `IRGen`.

## 21. Estado actual y limites conocidos

El backend actual cumple el flujo propio:

```text
HULK -> HulkIR -> BannerIR -> BannerVM
```

Y soporta:

- C4: literales, aritmetica, logica, strings, `if`, `while`, builtins;
- C5: variables, funciones, llamadas, recursion;
- C6: objetos, atributos, metodos, herencia, `base`, `is`, `as`;
- heap propio;
- frames propios;
- vtables;
- valores `Word`.

Limites conocidos:

- no hay GC;
- algunos `GETATTR` y `VCALL` pueden caer a fallback por nombre;
- `CONCAT` y operaciones de strings usan primitivas C++ internas;
- `ALLOCATE` usa `std::vector` para campos;
- BannerIR textual todavia imprime nombres legibles, no todos los slots numericos internos.

Estos limites no rompen el modelo actual. Solo indican que aun se puede bajar mas el nivel si se quiere una VM mas estricta.

## 22. Decisiones principales tomadas

### Retirar el backend C++20

Se elimino la ruta donde HULK se ejecutaba generando C++.

Razon:

El objetivo era tener un backend propio, no depender de otro compilador como target final.

### Mantener HulkIR

Se mantuvo HulkIR como IR media.

Razon:

Ya resolvia muchas transformaciones utiles: temporales, labels, saltos, funciones, objetos y metadatos de tipos.

### Agregar BannerIR

Se agrego una IR baja.

Razon:

Permite separar una IR comoda para el frontend de una IR mas adecuada para ejecucion en VM.

### Implementar frames propios

Las llamadas HULK usan frames de VM, no recursion C++.

Razon:

Una VM debe controlar su propia pila de llamadas.

### Implementar heap propio

Objetos y strings viven en `VMHeap`.

Razon:

Centraliza memoria runtime y prepara el camino para GC.

### Migrar a `Word`

Los valores runtime ahora son palabras de 64 bits.

Razon:

Hace que slots, campos, parametros y retornos sean uniformes y mas cercanos a una VM real.

### Usar vtables

Los metodos virtuales se resuelven por slots.

Razon:

Evita recorrer jerarquias en ejecucion y evita depender de dispatch virtual de C++.

## 23. Como leer un `.banner`

Un `.banner` es una vista textual de la IR baja.

Si ves:

```text
.TYPES
```

estas viendo layouts de objetos y metodos.

Si ves:

```text
.DATA
```

estas viendo strings estaticos.

Si ves:

```text
.CODE
```

estas viendo funciones e instrucciones ejecutables.

Si ves:

```text
param x
y = call f
```

significa:

1. preparar `x` como argumento;
2. llamar `f`;
3. guardar el retorno en `y`.

Si ves:

```text
y = vcall obj.speak()
```

significa:

1. tomar `obj`;
2. mirar su tipo runtime;
3. buscar `speak` por slot de vtable;
4. crear frame para la funcion concreta.

## 24. Resumen final

El backend actual implementa una cadena real de compilacion a VM:

```text
AST validado semanticamente
  -> HulkIR lineal con temporales
  -> BannerIR de tres direcciones
  -> CompiledProgram con slots e ids
  -> BannerVM con frames, heap, Word y vtables
```

La VM no ejecuta arboles del AST. Ejecuta instrucciones. Las expresiones ya fueron desenrolladas, los ciclos son saltos, las llamadas son frames, los objetos son referencias de heap y los metodos virtuales son entradas de vtable.

Esto hace que el backend sea propio, ejecutable y suficientemente bajo nivel para defenderlo como una maquina virtual de HULK, con espacio claro para seguir bajando el nivel si se eliminan los fallbacks por nombre y se agrega GC.
