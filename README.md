# HULK Compiler

Compilador completo para el lenguaje **HULK**: tipado dinámico, orientado a objetos y con características funcionales. El pipeline va de código fuente hasta ejecución en una máquina virtual de stack propia (BannerVM).

---

## Tabla de Contenidos

- [Inicio Rápido](#-inicio-rápido)
- [Compilar el Proyecto](#-compilar-el-proyecto)
- [Ejecutar Programas](#-ejecutar-programas)
  - [Via Evaluador (árbol)](#1-via-evaluador-de-árbol)
  - [Via VM completa](#2-via-pipeline-completo--bannervm)
  - [Emitir Banner IR](#3-emitir-banner-ir)
- [Arquitectura del Compilador](#-arquitectura-del-compilador)
- [Referencia de Comandos Make](#-referencia-de-comandos-make)
- [Comandos de Test](#-comandos-de-test)
- [Opciones Avanzadas del Backend](#-opciones-avanzadas-del-backend)
- [Estructura del Proyecto](#-estructura-del-proyecto)

---

## Inicio Rápido

```bash
# 1. Clonar el repositorio
git clone <url-del-repositorio>
cd Hulk

# 2. Compilar todos los binarios principales
make compile

# 3. Editar o crear un archivo de entrada
#    (ya hay un ejemplo listo en examples/example.hulk)

# 4. Ejecutar via VM
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
| `hulk_backend` | Compilador completo con BannerVM |

**Requisitos**: `g++` con soporte C++20, `bison`, `make`.

> Si solo necesitas uno de los dos:
> ```bash
> make eval      # solo hulk_eval
> make backend   # solo hulk_backend
> ```

---

## Ejecutar Programas

Los tres comandos principales de ejecución toman el archivo de entrada via la variable `FILE`. El valor por defecto es `examples/example.hulk`.

### 1. Via Evaluador de Árbol

```bash
make run-eval FILE=examples/example.hulk
```

Ejecuta el programa con el **evaluador de árbol** (`hulk_eval`): pasa por Lexer → Parser → Análisis Semántico → Evaluador. Útil para prototipado rápido y depuración temprana. No requiere que el backend esté compilado.

```bash
# Equivalente manual
./hulk_eval examples/example.hulk
```

### 2. Via Pipeline Completo + BannerVM

```bash
make run-vm FILE=examples/example.hulk
```

Ejecuta el programa pasando por el **pipeline completo**: análisis semántico → generación de IR → lowering a BannerIR → ejecución en la BannerVM (máquina virtual de stack con GC). Es el modo de producción.

```bash
# Equivalente manual
./hulk_backend --run-banner examples/example.hulk
```

### 3. Emitir Banner IR

```bash
make emit-banner FILE=examples/example.hulk
```

Compila el archivo, **guarda el Banner IR generado** en `outputs/<nombre>.banner` e imprime su contenido en la terminal. Útil para inspeccionar la representación de bajo nivel antes de la ejecución en la VM.

```bash
# Equivalente manual (solo a archivo)
./hulk_backend --emit-banner -o outputs/example.banner examples/example.hulk

# Ver el contenido después
cat outputs/example.banner
```

El directorio `outputs/` se crea automáticamente si no existe.

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
  │  IR de bajo nivel 
  ▼
BannerVM  (src/vm/)
     Máquina virtual de stack con GC
```

También existe un **evaluador de árbol** independiente (`src/eval/`) que recorre el AST directamente sin pasar por el backend. Se usa para las primeras etapas de prueba.

---

## Referencia de Comandos Make

### Comandos Principales

| Comando | Descripción |
|---|---|
| `make compile` | Compila `hulk_eval` y `hulk_backend` |
| `make run-eval FILE=<ruta>` | Ejecuta un archivo via evaluador de árbol |
| `make run-vm FILE=<ruta>` | Ejecuta un archivo via pipeline completo + BannerVM |
| `make emit-banner FILE=<ruta>` | Emite Banner IR a `outputs/` e imprime en terminal |

La variable `FILE` tiene como valor por defecto `examples/example.hulk`.

#### Modo restringido (`--restricted-inference`)

Ambos pipelines soportan inferencia restringida. En este modo el compilador **exige anotaciones de tipo explícitas en todos los parámetros de función**; si alguno carece de anotación, la compilación falla con un error de diagnóstico.

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
| `make lexer` | `hulk_lexer` | Analizador léxico |
| `make parser-demo` | `hulk_parser_demo` | Demo del parser |
| `make parser-gen` | — | Regenera `parser.cpp` desde `grammar.y` |
| `make clean` | — | Elimina artefactos de compilación |

---

## Comandos de Test

```bash
make run-tests          # Ejecuta todas las suites
make eval-tests         # Tests del evaluador
make semantic-tests     # Tests del analizador semántico
make backend-tests      # Tests del backend
make end-to-end-tests   # Tests end-to-end (A.2–A.9 de la documentación)
make vm-tests           # Tests unitarios de la BannerVM
make err-tests          # Tests que deben fallar con diagnóstico
make extension-tests    # Tests de extensiones de lenguaje
make update-expected    # Regenera archivos .expected tras cambios intencionales
```

Los tests end-to-end soportan filtrado por carpeta:

```bash
make end-to-end-tests FOLDER=06_objects
make end-to-end-tests FOLDER=08_inference
```

Las suites disponibles son: `01_arithmetic`, `02_strings_builtins`, `03_variables`, `04_control_flow`, `05_functions`, `06_objects`, `07_type_check`, `08_inference`, `09_restricted`.

---

## Opciones Avanzadas del Backend

El binario `hulk_backend` acepta las siguientes flags directamente:

```bash
./hulk_backend <archivo.hulk> [opciones]
```

| Opción | Descripción |
|---|---|
| `--run-banner` | Ejecuta el programa en la BannerVM (modo producción) |
| `--emit-ir` | Guarda el HulkIR (IR de nivel medio) en `<archivo>.hir` |
| `--emit-banner` | Guarda el BannerIR en `<archivo>.banner` |
| `--emit-banner-compiled` | Guarda la vista compilada del BannerIR |
| `-o <ruta>` | Especifica la ruta de salida para los modos `--emit-*` |
| `--restricted-inference` | Requiere anotaciones explícitas en parámetros de funciones |

**Ejemplos:**

```bash
# Ejecutar via VM
./hulk_backend --run-banner examples/example.hulk

# Emitir HulkIR a un archivo específico
./hulk_backend --emit-ir -o outputs/example.hir examples/example.hulk

# Emitir BannerIR
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
│   └── vm/
├── examples/
│   └── example.hulk
├── outputs/
├── lib/
└── Makefile
```

### Archivos de Ejemplo

El archivo `examples/example.hulk` es el punto de entrada de referencia para los comandos `run-eval`, `run-vm` y `emit-banner`. Modifícalo o crea nuevos archivos en esa carpeta para experimentar:

```hulk
let x = 42 in print(x);
function id(x) => x;
x := x + 1;
if (x >= 10) print("ok") else print("no");
```