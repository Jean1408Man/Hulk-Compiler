# HULK Compiler

Compilador para el lenguaje **HULK**: tipado estático con inferencia de tipos, orientado a objetos y con características funcionales.

> **Requisito de entorno**: los binarios compilados son ELF Linux. Todos los comandos `make` deben ejecutarse desde **WSL** (Windows Subsystem for Linux) o una terminal Linux/macOS nativa. 

---

## Tabla de Contenidos

- [Inicio Rápido](#-inicio-rápido)
- [Compilar el Proyecto](#-compilar-el-proyecto)
- [Ejecutar Programas](#-ejecutar-programas)
  - [Via Evaluador (árbol)](#1-via-evaluador-de-árbol)
  - [Via compilación a ejecutable](#2-via-compilación-a-ejecutable-modo-producción)
  - [Via VM directa](#3-via-vm-directa-sin-ejecutable)
  - [Emitir Banner IR](#4-emitir-banner-ir)
- [Modelo de Ejecución del Compilador](#-modelo-de-ejecución-del-compilador)
- [Arquitectura del Compilador](#-arquitectura-del-compilador)
- [Referencia de Comandos Make](#-referencia-de-comandos-make)
- [Comandos de Test](#-comandos-de-test)
- [Opciones Avanzadas del Backend](#-opciones-avanzadas-del-backend)
- [Estructura del Proyecto](#-estructura-del-proyecto)

---

## Inicio Rápido

```bash
# 1. Compilar todos los binarios
make compile

# 2. Editar o crear un archivo de entrada
#    (ya hay un ejemplo listo en examples/example.hulk)

# 3. Ejecutar via VM
make run-vm FILE=examples/example.hulk
```

El archivo de entrada por defecto es `examples/example.hulk`. Puedes modificarlo o crear cualquier archivo `.hulk` y pasarlo con `FILE=tu_archivo.hulk`.

---

## Compilar el Proyecto

```bash
make compile
```

Construye los dos binarios principales:

| Binario | Descripción |
|---|---|
| `hulk_eval` | Evaluador de árbol (sin backend, para desarrollo rápido) |
| `hulk_backend` | Compilador completo — produce ejecutables nativos con VM embebida |

También existe el target `make build` que compila únicamente `hulk_backend` y lo expone como `./hulk`, el binario de producción.

**Requisitos**: `g++` con soporte C++20, `bison`, `make`.

> Si solo necesitas uno:
> ```bash
> make eval      # solo hulk_eval
> make backend   # solo hulk_backend
> make build     # hulk_backend como ./hulk
> ```

---

## Ejecutar Programas

Los comandos de ejecución toman el archivo de entrada via la variable `FILE`. El valor por defecto es `examples/example.hulk`.

### 1. Via Evaluador de Árbol

```bash
make run-eval FILE=examples/example.hulk
```

Ejecuta el programa con el **evaluador de árbol** (`hulk_eval`): pasa por Lexer → Parser → Análisis Semántico → Evaluador. Útil para prototipado rápido y depuración temprana.

```bash
# Equivalente manual
./hulk_eval examples/example.hulk
```

### 2. Via Compilación a Ejecutable (modo producción)

```bash
make run-vm FILE=examples/example.hulk
```

Compila el archivo `.hulk` y produce un ejecutable `./output` con la BannerVM embebida, luego lo ejecuta automáticamente. Es el **modo de producción**.

```bash
# Equivalentes manuales (dos pasos)
./hulk examples/example.hulk   # compila → genera ./output
./output                        # ejecuta el programa
```

### 3. Via VM Directa (sin ejecutable)

Ejecuta en la BannerVM sin generar `./output`. Útil para depuración interna del compilador:

```bash
./hulk_backend --run-banner examples/example.hulk
```

### 4. Emitir Banner IR

```bash
make emit-banner FILE=examples/example.hulk
```

Compila y **guarda el Banner IR** en `outputs/<nombre>.banner`. Útil para inspeccionar la representación de bajo nivel.

```bash
# Equivalente manual
./hulk_backend --emit-banner -o outputs/example.banner examples/example.hulk
cat outputs/example.banner
```

El directorio `outputs/` se crea automáticamente si no existe.

---

## Modelo de Ejecución del Compilador

El compilador HULK produce ejecutables **autocontenidos** en dos pasos:

```
./hulk programa.hulk   →   genera ./output
./output               →   ejecuta el programa, imprime resultados
```

El ejecutable `./output` es un binario nativo que lleva el Banner IR del programa embebido junto con la BannerVM. Al ejecutarse, la VM desempaqueta el IR y lo corre directamente — no necesita el compilador ni archivos externos.

Este modelo de dos pasos es el que usan las suites de test (`tests/hulk/`, `tests/end-to-end/`) y la bandera `-o` del compilador permite cambiar el nombre del ejecutable de salida:

```bash
./hulk programa.hulk -o mi_programa
./mi_programa
```

---

## Arquitectura del Compilador

```
Fuente (.hulk)
  │
  ▼
Lexer  (src/lexer/)
  │  Tokenización y keywords
  ▼
Parser / Bison  (src/parser/)
  │  Gramática LALR(1); produce AST
  ▼
AST  (src/ast/)
  │
  ▼
SemanticAnalyzer  (src/semantic/)
  ├── SymbolResolver  (src/binding/)    — 3 pasadas: registrar, validar, enlazar
  ├── TypeInferencer  (src/inference/)  — inferencia de tipos 
  └── TypeChecker     (src/typecheck/)  — compatibilidad de tipos
  │
  ▼
HulkIR  (src/ir/)
  │  Representación intermedia estructurada de nivel medio
  ▼
HulkIRToBanner  (src/backend/hulkir_to_banner.cpp)
  │  Lowering: IR estructurado → IR linealizado
  ▼
BannerIR  (src/banner/)
  │  IR de bajo nivel (3-address, stack-based)
  ▼
OutputPackager  (src/backend/output_packager.cpp)
  │  Empaqueta BannerIR + BannerVM en un ejecutable nativo (./output)
  ▼
./output
     Ejecutable autocontenido: BannerVM + IR embebido
```

También existe un **evaluador de árbol** independiente (`src/eval/`) que recorre el AST directamente sin pasar por el backend. Se usa para las primeras etapas de prueba.

---

## Referencia de Comandos Make

### Comandos Principales

| Comando | Descripción |
|---|---|
| `make compile` | Compila `hulk_eval` y `hulk_backend` |
| `make build` | Compila el backend como `./hulk` (binario) |
| `make run-eval FILE=<ruta>` | Ejecuta un archivo via evaluador de árbol |
| `make run-vm FILE=<ruta>` | Compila + ejecuta via BannerVM |
| `make emit-banner FILE=<ruta>` | Emite Banner IR a `outputs/` e imprime en terminal |

La variable `FILE` tiene como valor por defecto `examples/example.hulk`.

#### Modo restringido (`--restricted-inference`)

Ambos pipelines soportan inferencia restringida. En este modo el compilador **exige anotaciones de tipo explícitas en todos los parámetros de función**.

```bash
# Via evaluador de árbol
./hulk_eval --restricted-inference examples/example.hulk

# Via pipeline completo + VM
./hulk_backend --run-banner --restricted-inference examples/example.hulk
```

### Comandos de Compilación Individuales

| Comando | Binario generado | Descripción |
|---|---|---|
| `make eval` | `hulk_eval` | Evaluador de árbol |
| `make semantic` | `hulk_semantic` | Analizador semántico |
| `make backend` | `hulk_backend` | Compilador completo + BannerVM |
| `make build` | `hulk` | Backend como binario de producción (`./hulk`) |
| `make lexer` | `hulk_lexer` | Analizador léxico |
| `make parser-demo` | `hulk_parser_demo` | Demo del parser |
| `make parser-gen` | — | Regenera `parser.cpp` desde `grammar.y` |
| `make clean` | — | Elimina artefactos de compilación |

---

## Comandos de Test

> Todos los comandos de test deben ejecutarse desde WSL o Linux.

```bash
make run-tests          # Eval + semántico + typecheck (usa hulk_eval y hulk_semantic)
make backend-tests      # Tests del backend (compila + ejecuta ./output)
make end-to-end-tests   # Tests end-to-end completos (A.2–A.9)
make hulk-tests         # Suite de calificación externa (tests/hulk/)
make update-expected    # Regenera archivos .expected tras cambios intencionales
```

Los tests end-to-end soportan filtrado por carpeta:

```bash
make end-to-end-tests FOLDER=06_objects
make end-to-end-tests FOLDER=08_inference
```

---

## Opciones Avanzadas del Backend

El binario `hulk_backend` (o `hulk`) acepta las siguientes flags:

```bash
./hulk_backend <archivo.hulk> [opciones]
```

| Opción | Descripción |
|---|---|
| _(ninguna)_ | Compila a ejecutable `./output` (modo por defecto) |
| `-o <ruta>` | Cambia el nombre del ejecutable de salida |
| `--run-banner` | Ejecuta en la BannerVM sin generar ejecutable |
| `--emit-ir` | Guarda el HulkIR en `<ruta>.hir` |
| `--emit-banner` | Guarda el BannerIR en `<ruta>.banner` |
| `--restricted-inference` | Requiere anotaciones explícitas en parámetros |

**Ejemplos:**

```bash
# Compilar a ejecutable y correr (dos pasos)
./hulk examples/example.hulk
./output

# Compilar con nombre de salida personalizado
./hulk examples/example.hulk -o mi_programa
./mi_programa

# Ejecutar directamente en la VM (sin generar ejecutable)
./hulk_backend --run-banner examples/example.hulk

# Inspeccionar IR
./hulk_backend --emit-ir -o outputs/example.hir examples/example.hulk
./hulk_backend --emit-banner -o outputs/example.banner examples/example.hulk

# Modo restringido
./hulk_backend --run-banner --restricted-inference examples/example.hulk
```

---

## Estructura del Proyecto

```
Hulk/
├── src/
│   ├── ast/
│   ├── lexer/
│   ├── parser/
│   ├── binding/
│   ├── inference/
│   ├── typecheck/
│   ├── semantic/
│   ├── eval/
│   ├── ir/
│   ├── backend/
│   ├── banner/
│   ├── vm/
│   ├── objects/
│   └── common/
├── tests/
│   ├── eval/
│   ├── semantic/
│   ├── typecheck/
│   ├── backend/
│   ├── end-to-end/
│   ├── hulk/      
│   └── vm/
├── examples/
│   └── example.hulk
├── outputs/
├── lib/
└── Makefile
```

### Archivos de Ejemplo

El archivo `examples/example.hulk` es el punto de entrada de referencia. Modifícalo o crea nuevos archivos en esa carpeta para experimentar:

```hulk
let x = 42 in print(x);
```
