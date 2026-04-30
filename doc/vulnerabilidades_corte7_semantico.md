# Vulnerabilidades del corte 7 — Análisis semántico base HULK

Este documento resume las vulnerabilidades detectadas en los archivos del corte 7:

- `static_scope.h`
- `symbol_resolver.h`
- `symbol_resolver.cpp`
- `semantic_tables.h`
- `semantic_tables.cpp`
- `semantic_type_info.h`
- `semantic_func_info.h`
- `analyzer.h`
- `analyzer.cpp`
- `semantic.cpp`
- `binding.cpp`
- `main_semantic.cpp`

El objetivo del corte 7 no es hacer inferencia ni type checking completo, sino construir una infraestructura estática confiable para los cortes 8 y 9:

1. registrar funciones globales;
2. registrar tipos, constructores, atributos y métodos;
3. resolver símbolos en scopes léxicos;
4. anotar referencias del AST con su entidad declarativa;
5. detectar errores semánticos previos a tipos: nombres no declarados, duplicados, herencia inválida, ciclos, aridad incorrecta y usos inválidos de `self`/`base`.

## Estado general

La estructura está bien encaminada: `SemanticAnalyzer` delega en `SymbolResolver`, `SemanticTables` separa funciones y tipos, y `resolution_map_` permite entregar al corte 8 un AST parcialmente enlazado.

Sin embargo, todavía hay varios huecos que pueden producir dos clases de fallos:

- **falsos negativos:** programas inválidos que el semántico acepta;
- **resolución incompleta:** referencias que no quedan anotadas, dejando trabajo ambiguo al corte 8.

---

# 1. Resolución incorrecta por shadowing entre `Param` y `VariableBinding`

## Dónde ocurre

Archivo: `static_scope.h`  
Métodos afectados:

- `contains`
- `get_binding`
- `get_param`

En `VariableReference`, el resolver hace:

```cpp
VariableBinding* b = scope_->get_binding(n.GetName());
if (b) {
    resolution_map_[&n] = ResolutionResult::from_binding(b);
} else {
    const Param* p = scope_->get_param(n.GetName());
    if (p) resolution_map_[&n] = ResolutionResult::from_param(p);
}
```

El problema es que `get_binding()` sube por los padres antes de que se consulte si en el scope actual hay un parámetro con el mismo nombre. Por tanto, un parámetro local puede quedar ocultado incorrectamente por un binding externo.

## Caso que falla

```hulk
let x = 1 in
    ((x) => x)(2);
```

El `x` dentro de la lambda debe resolver al parámetro `x`, no al `let x` externo.

Otro caso:

```hulk
let value = 10 in {
    function-like-lambda = (value) => value;
    print(function-like-lambda(99));
};
```

El `value` del cuerpo de la lambda debe ser el parámetro.

## Resultado incorrecto actual

El resolver puede encontrar primero el `VariableBinding` exterior y enlazar mal la referencia interna.

## Arreglo recomendado

No separar la búsqueda en `get_binding()` y `get_param()` con ascenso independiente. Crear un único método de lookup que respete el orden correcto por scope:

```cpp
struct ScopeSymbol {
    enum class Kind { Binding, Param, Synthetic } kind;
    VariableBinding* binding = nullptr;
    const Param* param = nullptr;
    SyntheticSymbol* synthetic = nullptr;
};

std::optional<ScopeSymbol> StaticScope::lookup(const std::string& name) const {
    auto bit = bindings_.find(name);
    if (bit != bindings_.end()) {
        return ScopeSymbol{ScopeSymbol::Kind::Binding, bit->second, nullptr, nullptr};
    }

    auto pit = params_.find(name);
    if (pit != params_.end()) {
        return ScopeSymbol{ScopeSymbol::Kind::Param, nullptr, pit->second, nullptr};
    }

    if (parent_) return parent_->lookup(name);
    return std::nullopt;
}
```

Luego `VariableReference` debe usar solo `lookup()`.

## Test mínimo

```hulk
let x = 1 in ((x) => x)(2);
```

Debe resolver el `x` del cuerpo al parámetro de la lambda.

---

# 2. `self` se permite dentro de inicializadores de atributos

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Zona: resolución de miembros de `TypeDecl` dentro de `SymbolResolver::run`.

Actualmente, para atributos se hace:

```cpp
push_scope();
for (const auto& p : td->GetCtorParams())
    scope_->define_param(p.name, &p);
scope_->define_param("self", nullptr); // self disponible en atributos
resolve(attr->GetInitializer());
pop_scope();
```

Pero en HULK los inicializadores de atributos no deben ver `self`. Los atributos se inicializan usando globales y parámetros del constructor, pero no la instancia parcialmente construida.

## Caso que falla

```hulk
type A {
    x = self;
}

new A();
```

También debería fallar:

```hulk
type A {
    x = 1;
    y = self.x;
}

new A();
```

## Resultado incorrecto actual

El semántico puede aceptar `self` dentro de atributos porque lo mete explícitamente en el scope del inicializador.

## Arreglo recomendado

Introducir un contexto semántico explícito:

```cpp
enum class ResolverContext {
    Global,
    Function,
    TypeAttributeInit,
    Method
};

ResolverContext context_ = ResolverContext::Global;
```

Al resolver atributos:

```cpp
auto old = context_;
context_ = ResolverContext::TypeAttributeInit;

push_scope();
for (const auto& p : td->GetCtorParams())
    scope_->define_param(p.name, &p);
// NO definir self aquí.
resolve(attr->GetInitializer());
pop_scope();

context_ = old;
```

Y en `visit(SelfRef&)`:

```cpp
void SymbolResolver::visit(SelfRef& n) {
    if (context_ != ResolverContext::Method) {
        report_raw(n.span, "'self' solo puede usarse dentro de métodos.");
        resolution_map_[&n] = ResolutionResult{};
        return;
    }
    // resolver self como símbolo sintético o como Param especial
}
```

## Test mínimo

```hulk
type A {
    x = self;
}
new A();
```

Debe reportar error semántico.

---

# 3. Los métodos ven parámetros del constructor aunque no deberían

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Zona: resolución de métodos dentro de `SymbolResolver::run`.

Actualmente, para métodos se hace:

```cpp
push_scope();
for (const auto& p : td->GetCtorParams())
    scope_->define_param(p.name, &p);
for (const auto& p : method->GetParams())
    scope_->define_param(p.name, &p);
scope_->define_param("self", nullptr);
resolve(method->GetBody());
pop_scope();
```

Los parámetros del constructor deben estar disponibles en inicializadores de atributos y en argumentos enviados al padre, pero no automáticamente dentro de métodos.

## Caso que falla

```hulk
type A(x) {
    f() => x;
}

let a = new A(10) in print(a.f());
```

Ese `x` no debería existir dentro de `f`. La forma correcta es almacenar `x` como atributo:

```hulk
type A(x) {
    x = x;
    f() => self.x;
}
```

## Resultado incorrecto actual

El método puede resolver `x` contra el parámetro del constructor, aceptando un programa inválido.

## Arreglo recomendado

En métodos, definir solo:

- `self`;
- parámetros propios del método;
- símbolos globales accesibles mediante tablas globales.

Código sugerido:

```cpp
push_scope();
scope_->define_param("self", nullptr);
for (const auto& p : method->GetParams())
    scope_->define_param(p.name, &p);
resolve(method->GetBody());
pop_scope();
```

Si se necesita acceder a argumentos del constructor, deben haberse guardado como atributos.

## Test mínimo

```hulk
type A(x) {
    f() => x;
}
new A(1).f();
```

Debe reportar variable no declarada `x` dentro de `f`.

---

# 4. `base()` está permitido en contextos incorrectos

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Método: `SymbolResolver::visit(BaseCall&)`.

Actualmente solo se revisa:

```cpp
if (current_type_name_.empty()) {
    report_raw(n.span, "'base()' solo puede usarse dentro de un método de tipo.");
}
```

Pero `current_type_name_` se establece durante todo el procesamiento del tipo, incluyendo atributos. Además, no se verifica si el tipo realmente tiene padre ni si el método existe en la cadena de herencia.

## Casos que fallan

### Caso A: `base()` en atributo

```hulk
type A inherits B {
    x = base();
}
```

Debe fallar. `base()` solo tiene sentido dentro de un método que redefine uno heredado, no en un atributo.

### Caso B: `base()` en tipo sin padre

```hulk
type A {
    f() => base();
}

new A().f();
```

Debe fallar porque `A` no tiene padre explícito con implementación de `f`.

### Caso C: `base()` en método que no existe en el padre

```hulk
type A {
    f() => 1;
}

type B inherits A {
    g() => base();
}
```

Debe fallar porque `base()` dentro de `g` tendría que referirse a `A.g`, que no existe.

## Resultado incorrecto actual

El semántico solo detecta `base()` fuera de cualquier tipo, pero no distingue atributo vs método ni verifica la existencia del método base.

## Arreglo recomendado

Agregar estado de método actual:

```cpp
std::string current_method_name_;
bool inside_method_ = false;
```

Al resolver un método:

```cpp
auto old_method = current_method_name_;
auto old_inside = inside_method_;

inside_method_ = true;
current_method_name_ = method->GetName();
resolve(method->GetBody());

current_method_name_ = old_method;
inside_method_ = old_inside;
```

En `visit(BaseCall&)`:

```cpp
void SymbolResolver::visit(BaseCall& n) {
    if (!inside_method_) {
        report_raw(n.span, "'base()' solo puede usarse dentro de métodos.");
        return;
    }

    const auto* type = tables_.lookup_type(current_type_name_);
    if (!type || type->parent_name.empty()) {
        report_raw(n.span, "'base()' usado en un tipo sin padre.");
        return;
    }

    const auto* parent_method = tables_.find_method(type->parent_name, current_method_name_);
    if (!parent_method) {
        report_raw(n.span,
            "'base()' no puede usarse porque no existe método base para '" +
            current_method_name_ + "'.");
        return;
    }

    if (n.GetArgs().size() != parent_method->params.size()) {
        report_raw(n.span, "Aridad incorrecta en llamada a base().");
    }

    for (auto& arg : n.GetArgs())
        resolve(arg.get());
}
```

## Test mínimo

```hulk
type A {
    f() => base();
}
new A().f();
```

Debe reportar uso inválido de `base()`.

---

# 5. Redefinición de métodos heredados sin validación de firma

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Método: `SymbolResolver::check_methods()`.

Actualmente solo se revisan métodos cuyo `SemanticMethodInfo::is_override` sea `true`. Pero ese campo nunca se llena desde el AST en `visit(TypeDecl&)`; queda siempre en `false`.

Además, HULK no necesita necesariamente una palabra `override`: los métodos son virtuales y pueden redefinirse siempre que respeten la firma.

## Casos que fallan

### Caso A: override con aridad distinta

```hulk
type A {
    f(x: Number): Number => x;
}

type B inherits A {
    f(): Number => 0;
}
```

Debe fallar: `B.f` redefine `A.f`, pero con distinta cantidad de parámetros.

### Caso B: override con retorno incompatible

```hulk
type A {
    f(): Number => 1;
}

type B inherits A {
    f(): String => "bad";
}
```

La compatibilidad exacta o conformidad de retorno puede quedar para el corte 9, pero al menos debe registrarse que existe una redefinición y que debe verificarse.

### Caso C: override con parámetros distintos

```hulk
type A {
    f(x: Number): Number => x;
}

type B inherits A {
    f(x: String): Number => 0;
}
```

Debe fallar por firma incompatible.

## Resultado incorrecto actual

`check_methods()` no detecta estos casos si `is_override` nunca se activa.

## Arreglo recomendado

En `check_methods()`, buscar automáticamente si un método del tipo existe en algún ancestro:

```cpp
void SymbolResolver::check_methods() {
    for (auto& [type_name, type_info] : tables_.all_types()) {
        if (type_info.parent_name.empty()) continue;

        for (auto& [method_name, method_info] : type_info.methods) {
            const auto* parent_method = tables_.find_method(type_info.parent_name, method_name);
            if (!parent_method) continue;

            if (method_info.params.size() != parent_method->params.size()) {
                report_raw(method_info.body ? method_info.body->span : hulk::common::Span{},
                    "Método '" + method_name + "' redefine un método heredado con aridad distinta.");
                continue;
            }

            for (size_t i = 0; i < method_info.params.size(); ++i) {
                const auto& a = method_info.params[i].type_annotation;
                const auto& b = parent_method->params[i].type_annotation;
                if (!a.empty() && !b.empty() && a != b) {
                    report_raw(method_info.body ? method_info.body->span : hulk::common::Span{},
                        "Método '" + method_name + "' redefine un parámetro con tipo distinto.");
                }
            }

            const auto& child_ret = method_info.return_type_annotation;
            const auto& parent_ret = parent_method->return_type_annotation;
            if (!child_ret.empty() && !parent_ret.empty() && child_ret != parent_ret) {
                // En corte 9 puede relajarse a conformidad/covarianza.
                report_raw(method_info.body ? method_info.body->span : hulk::common::Span{},
                    "Método '" + method_name + "' redefine retorno con tipo distinto.");
            }
        }
    }
}
```

Si luego el corte 9 implementa conformidad real, esta revisión puede pasar de igualdad textual a `conforms(child_ret, parent_ret)`.

## Test mínimo

```hulk
type A { f(x: Number): Number => x; }
type B inherits A { f(): Number => 0; }
new B().f();
```

Debe reportar override inválido por aridad.

---

# 6. `MemberAccess` y `MethodCall` no se anotan en `resolution_map_`

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Métodos:

- `visit(MemberAccess&)`
- `visit(MethodCall&)`

Actualmente solo se resuelve el objeto y los argumentos:

```cpp
void SymbolResolver::visit(MemberAccess& n) {
    resolve(n.GetObject());
}

void SymbolResolver::visit(MethodCall& n) {
    resolve(n.GetObject());
    for (auto& arg : n.GetArgs())
        resolve(arg.get());
}
```

Esto deja sin anotación accesos a atributos y métodos. Es cierto que `obj.x` depende del tipo inferido de `obj`, pero hay casos que sí se pueden resolver en el corte 7, como `self.x` y `self.f()`.

## Casos que fallan

### Caso A: atributo inexistente en `self`

```hulk
type A {
    x = 1;
    f() => self.y;
}
```

`self.y` debería reportar atributo inexistente en el corte 7, porque el tipo de `self` es conocido: `A`.

### Caso B: método inexistente en `self`

```hulk
type A {
    f() => self.g();
}
```

`self.g()` debería reportar método inexistente en `A` o sus ancestros.

### Caso C: aridad incorrecta en método sobre `self`

```hulk
type A {
    g(x: Number) => x;
    f() => self.g();
}
```

Debe reportar aridad incorrecta.

## Resultado incorrecto actual

No se reporta nada en corte 7. Se delega todo a inferencia/type checking, pero eso deja el `resolution_map_` incompleto.

## Arreglo recomendado

Resolver parcialmente los miembros cuyo receptor tenga tipo conocido por contexto.

Ejemplo de detección:

```cpp
bool is_self_expr(Expr* e) {
    return dynamic_cast<SelfRef*>(e) != nullptr;
}
```

Para `MemberAccess`:

```cpp
void SymbolResolver::visit(MemberAccess& n) {
    resolve(n.GetObject());

    if (is_self_expr(n.GetObject())) {
        const auto* type = tables_.lookup_type(current_type_name_);
        const auto* attr = find_attribute_in_type_or_ancestors(type, n.GetMemberName());
        if (!attr) {
            report_raw(n.span,
                "Atributo '" + n.GetMemberName() + "' no existe en tipo '" + current_type_name_ + "'.");
            resolution_map_[&n] = ResolutionResult{};
            return;
        }
        resolution_map_[&n] = ResolutionResult::from_attr(attr);
    }
}
```

Para `MethodCall`:

```cpp
void SymbolResolver::visit(MethodCall& n) {
    resolve(n.GetObject());
    for (auto& arg : n.GetArgs())
        resolve(arg.get());

    if (is_self_expr(n.GetObject())) {
        const auto* method = tables_.find_method(current_type_name_, n.GetMethodName());
        if (!method) {
            report_raw(n.span,
                "Método '" + n.GetMethodName() + "' no existe en tipo '" + current_type_name_ + "'.");
            resolution_map_[&n] = ResolutionResult{};
            return;
        }

        if (n.GetArgs().size() != method->params.size()) {
            report_raw(n.span, "Aridad incorrecta en llamada a método.");
        }

        resolution_map_[&n] = ResolutionResult::from_method(method);
    }
}
```

Para receptores generales (`obj.x`, `obj.m()`), se puede dejar una segunda resolución para después de inferencia.

## Test mínimo

```hulk
type A {
    f() => self.noExiste();
}
new A().f();
```

Debe reportar método inexistente.

---

# 7. Variables sintéticas de `for` y `VectorGenerator` no quedan resueltas

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Métodos:

- `visit(For&)`
- `visit(VectorGenerator&)`

Actualmente se hace:

```cpp
scope_->define_binding(n.GetVarName(), nullptr);
```

Esto evita el error de variable no declarada, pero cuando se visita una referencia a esa variable, `get_binding()` devuelve `nullptr` y `get_param()` también. Por tanto, no se inserta una entrada válida en `resolution_map_`.

## Casos que fallan

### Caso A: variable del `for`

```hulk
for (x in range(0, 10)) print(x);
```

La referencia `x` dentro del cuerpo debe quedar resuelta a la variable sintética del `for`.

### Caso B: variable del generador de vector

```hulk
let squares = [x * x | x in range(1, 10)] in print(squares);
```

Las referencias `x` dentro del generador deben resolverse a la variable sintética del generador.

## Resultado incorrecto actual

No se reporta error, pero tampoco queda binding real en `resolution_map_`.

## Arreglo recomendado

Crear una entidad semántica para símbolos sintéticos:

```cpp
enum class SyntheticKind {
    ForVariable,
    VectorGeneratorVariable,
    Self
};

struct SyntheticSymbol {
    std::string name;
    SyntheticKind kind;
    hulk::common::Span span;
};
```

Extender `ResolutionKind`:

```cpp
enum class ResolutionKind {
    Variable,
    Param,
    Synthetic,
    Function,
    Type,
    Method,
    Attribute,
    Unresolved
};
```

Y agregar almacenamiento en `StaticScope`:

```cpp
void define_synthetic(const std::string& name, SyntheticSymbol* symbol);
```

En `visit(For&)`:

```cpp
auto synthetic = std::make_unique<SyntheticSymbol>(
    SyntheticSymbol{n.GetVarName(), SyntheticKind::ForVariable, n.span}
);
SyntheticSymbol* ptr = synthetic.get();
synthetic_symbols_.push_back(std::move(synthetic));

scope_->define_synthetic(n.GetVarName(), ptr);
resolve(n.GetBody());
```

## Test mínimo

```hulk
for (x in range(0, 3)) print(x);
```

Debe haber una entrada en `resolution_map_` para el `VariableReference(x)`.

---

# 8. No se validan anotaciones de tipo desconocidas

## Dónde ocurre

Varios archivos:

- `symbol_resolver.cpp`
- `semantic_type_info.h`
- `semantic_func_info.h`

Se guardan anotaciones como `std::string`, pero no hay un pase general que valide si esos tipos existen.

Actualmente se valida el tipo en `is` y `as`, pero no en:

- parámetros de función;
- retorno de función;
- parámetros de constructor;
- atributos;
- parámetros de método;
- retorno de método;
- bindings `let` anotados;
- lambdas anotadas.

## Casos que fallan

### Caso A: función con parámetro desconocido

```hulk
function f(x: Missing): Number => 1;
print(f(0));
```

Debe reportar `Missing` como tipo no declarado.

### Caso B: retorno desconocido

```hulk
function f(): Missing => 1;
print(f());
```

Debe reportar `Missing` como tipo no declarado.

### Caso C: atributo desconocido

```hulk
type A {
    x: Missing = 1;
}
new A();
```

Debe reportar `Missing` como tipo no declarado.

### Caso D: variable `let` anotada con tipo desconocido

```hulk
let x: Missing = 1 in print(x);
```

Debe reportar `Missing` como tipo no declarado.

## Resultado incorrecto actual

Estas anotaciones pueden pasar sin error y provocar fallos o ambigüedad en el corte 8.

## Arreglo recomendado

Agregar helper:

```cpp
bool SymbolResolver::is_known_type_name(const std::string& name) const {
    if (name.empty()) return true;
    if (name == "Object" || name == "Number" || name == "String" || name == "Boolean")
        return true;
    return tables_.lookup_type(name) != nullptr;
}

void SymbolResolver::check_type_annotation(const hulk::common::Span& span,
                                           const std::string& type_name) {
    if (!type_name.empty() && !is_known_type_name(type_name)) {
        report(span, "SEM_UNDECLARED_TYPE", type_name);
    }
}
```

Usarlo en:

```cpp
visit(FunctionDecl&)
visit(TypeDecl&)
visit(VariableBinding&)
visit(Lambda&)
```

Ejemplo para funciones:

```cpp
for (const auto& p : n.GetParams()) {
    check_type_annotation(n.span, p.type_annotation);
}
check_type_annotation(n.span, n.GetReturnTypeAnnotation());
```

Ejemplo para atributos:

```cpp
check_type_annotation(attr->span, attr->GetTypeAnnotation());
```

## Test mínimo

```hulk
function f(x: Missing): Number => 1;
print(f(0));
```

Debe reportar tipo no declarado `Missing`.

---

# 9. No se registran tipos builtin ni `Object` en `SemanticTables`

## Dónde ocurre

Archivos:

- `semantic_tables.h`
- `semantic_tables.cpp`

La tabla de tipos solo registra `TypeDecl` del programa. No se inicializa con:

- `Object`
- `Number`
- `String`
- `Boolean`

Esto afecta `is_subtype`, LCA futuro y conformidad en los cortes 8/9.

## Casos que fallan

### Caso A: conformidad con `Object`

```hulk
type Dog {}
```

En el sistema de tipos, `Dog <= Object` debe ser verdadero. Si `Dog.parent_name == ""`, `is_subtype("Dog", "Object")` puede devolver falso.

### Caso B: heredar de builtin

```hulk
type Bad inherits Number {}
```

Debe fallar. Los tipos builtin no deben ser padres explícitos.

### Caso C: redefinir builtin

```hulk
type Number {}
```

Debe fallar. No debe permitirse declarar un tipo con nombre builtin.

## Resultado incorrecto actual

`Object` no existe como raíz real de la jerarquía. Además, si alguien declara `Number`, podría registrarse como tipo de usuario salvo que se agregue una protección.

## Arreglo recomendado

Inicializar `SemanticTables` con builtins:

```cpp
SemanticTables::SemanticTables() {
    register_builtin_type("Object", "");
    register_builtin_type("Number", "Object");
    register_builtin_type("String", "Object");
    register_builtin_type("Boolean", "Object");
}
```

Modificar `SemanticTypeInfo`:

```cpp
bool is_builtin = false;
```

Y `register_type()` debe rechazar duplicados contra builtins:

```cpp
bool SemanticTables::register_type(SemanticTypeInfo info) {
    if (types_.count(info.name)) return false;
    types_.emplace(info.name, std::move(info));
    return true;
}
```

Además, en `check_inheritance()`:

```cpp
if (info.parent_name == "Number" || info.parent_name == "String" || info.parent_name == "Boolean") {
    report_raw(span, "No se puede heredar de tipo builtin '" + info.parent_name + "'.");
}
```

Si un tipo no declara padre explícito, asignar `Object` como padre lógico:

```cpp
info.parent_name = n.HasParent() ? n.GetParentName() : "Object";
```

Excepto para `Object` mismo.

## Test mínimo

```hulk
type Bad inherits Number {}
new Bad();
```

Debe reportar que no se puede heredar de `Number`.

---

# 10. No se detectan atributos duplicados dentro de un tipo

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Método: `visit(TypeDecl&)`.

Los métodos se guardan en `unordered_map` y sí se detecta duplicado, pero los atributos se guardan en `vector` y no se revisa si un nombre se repite.

## Caso que falla

```hulk
type A {
    x = 1;
    x = 2;
}

new A();
```

Debe reportar atributo duplicado.

También conviene decidir si se permite que un atributo y un método compartan nombre:

```hulk
type A {
    x = 1;
    x() => 2;
}
```

Recomendación: rechazarlo para evitar ambigüedad en acceso por punto.

## Resultado incorrecto actual

El tipo puede quedar con dos atributos llamados igual.

## Arreglo recomendado

En `visit(TypeDecl&)`, usar sets locales:

```cpp
std::unordered_set<std::string> attr_names;
std::unordered_set<std::string> member_names;
```

Al registrar atributo:

```cpp
if (!attr_names.insert(ai.name).second) {
    report_raw(attr->span,
        "Atributo '" + ai.name + "' duplicado en tipo '" + n.GetName() + "'.");
}

if (!member_names.insert(ai.name).second) {
    report_raw(attr->span,
        "Miembro '" + ai.name + "' duplicado en tipo '" + n.GetName() + "'.");
}
```

Al registrar método:

```cpp
if (!member_names.insert(mi.name).second) {
    report_raw(method->span,
        "Miembro '" + mi.name + "' duplicado en tipo '" + n.GetName() + "'.");
}
```

## Test mínimo

```hulk
type A { x = 1; x = 2; }
new A();
```

Debe reportar atributo duplicado.

---

# 11. Builtins y constantes globales no están integrados al semántico

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Métodos afectados:

- `visit(BuiltinCall&)`
- `visit(VariableReference&)`
- `visit(FunctionCall&)`

`BuiltinCall` solo resuelve argumentos. No valida nombre ni aridad. Además, si `PI` y `E` llegan como `VariableReference`, se reportarán como variables no declaradas salvo que el parser los represente con otro nodo.

## Casos que fallan

### Caso A: builtin inexistente

```hulk
unknown_builtin(1);
```

Si el parser produce `BuiltinCall`, debería reportar builtin desconocido.

### Caso B: aridad incorrecta

```hulk
sqrt(2);
log(10, 5);
rand();
```

Deben fallar por aridad.

### Caso C: constantes globales

```hulk
print(PI);
print(E);
```

Deben resolverse como constantes builtin, no como variables no declaradas.

## Resultado incorrecto actual

Los builtins no tienen una tabla semántica unificada. La resolución queda dependiente de cómo el parser modeló cada builtin.

## Arreglo recomendado

Crear tabla de símbolos builtin:

```cpp
struct BuiltinFuncInfo {
    std::string name;
    size_t arity;
};

struct BuiltinConstInfo {
    std::string name;
    std::string type;
};
```

Registrar como mínimo:

```text
print(Object): Object
sqrt(Number): Number
sin(Number): Number
cos(Number): Number
exp(Number): Number
log(Number, Number): Number
rand(): Number
range(Number, Number): Range / Iterable
PI: Number
E: Number
```

Extender `ResolutionKind`:

```cpp
BuiltinFunction,
BuiltinConstant,
```

En `visit(BuiltinCall&)`:

```cpp
auto* builtin = tables_.lookup_builtin_func(n.GetName());
if (!builtin) {
    report_raw(n.span, "Función builtin no declarada: '" + n.GetName() + "'.");
} else if (n.GetArgs().size() != builtin->arity) {
    report_raw(n.span, "Aridad incorrecta en builtin '" + n.GetName() + "'.");
}
```

En `visit(VariableReference&)`, antes de reportar variable no declarada:

```cpp
if (auto* c = tables_.lookup_builtin_const(n.GetName())) {
    resolution_map_[&n] = ResolutionResult::from_builtin_const(c);
    return;
}
```

## Test mínimo

```hulk
print(PI);
sqrt(2);
```

`PI` debe resolverse; `sqrt(1, 2)` debe reportar aridad incorrecta.

---

# 12. No se detectan parámetros duplicados en funciones, métodos, lambdas o constructores

## Dónde ocurre

Archivos:

- `symbol_resolver.cpp`
- `semantic_func_info.h`
- `semantic_type_info.h`

Los parámetros se copian como `std::vector<Param>`, pero no se verifica repetición de nombres.

## Casos que fallan

### Función

```hulk
function f(x: Number, x: Number): Number => x;
print(f(1, 2));
```

Debe fallar por parámetro duplicado.

### Método

```hulk
type A {
    f(x: Number, x: Number): Number => x;
}
new A().f(1, 2);
```

Debe fallar.

### Constructor

```hulk
type A(x: Number, x: Number) {
    y = x;
}
new A(1, 2);
```

Debe fallar.

### Lambda

```hulk
let f = (x, x) => x in f(1, 2);
```

Debe fallar.

## Resultado incorrecto actual

El último parámetro insertado en `StaticScope` puede sobrescribir al anterior, ocultando el error.

## Arreglo recomendado

Crear helper:

```cpp
void SymbolResolver::check_duplicate_params(const std::vector<Param>& params,
                                            const hulk::common::Span& span,
                                            const std::string& owner) {
    std::unordered_set<std::string> seen;
    for (const auto& p : params) {
        if (!seen.insert(p.name).second) {
            report_raw(span,
                "Parámetro '" + p.name + "' duplicado en " + owner + ".");
        }
    }
}
```

Usarlo en:

- `visit(FunctionDecl&)`;
- `visit(TypeDecl&)` para constructor;
- registro de cada método;
- `visit(Lambda&)`.

## Test mínimo

```hulk
function f(x, x) => x;
print(f(1, 2));
```

Debe reportar parámetro duplicado.

---

# 13. `self` se modela como `Param` nulo y no queda resuelto

## Dónde ocurre

Archivo: `symbol_resolver.cpp`.

Se usa:

```cpp
scope_->define_param("self", nullptr);
```

Y `visit(SelfRef&)` solo valida contexto, pero no guarda resolución en `resolution_map_`.

## Caso que falla

```hulk
type A {
    f() => self;
}
new A().f();
```

El `SelfRef` debería quedar anotado como símbolo especial con tipo estático `A`.

## Resultado incorrecto actual

No queda entrada útil en `resolution_map_`. El corte 8 tendrá que inferir `self` por contexto global externo en vez de leerlo de la resolución.

## Arreglo recomendado

Modelar `self` como símbolo sintético:

```cpp
SyntheticSymbol self_symbol{ "self", SyntheticKind::Self, method->span };
scope_->define_synthetic("self", &self_symbol);
```

Y en `visit(SelfRef&)`:

```cpp
if (context_ != ResolverContext::Method) {
    report_raw(n.span, "'self' solo puede usarse dentro de métodos.");
    resolution_map_[&n] = ResolutionResult{};
    return;
}

resolution_map_[&n] = ResolutionResult::from_synthetic(current_self_symbol_);
```

## Test mínimo

```hulk
type A { f() => self; }
new A().f();
```

Debe quedar resuelto `SelfRef` en `resolution_map_`.

---

# 14. Asignación destructiva a `self` no se prohíbe explícitamente

## Dónde ocurre

Archivo: `symbol_resolver.cpp`  
Método: `visit(DestructiveAssign&)`.

Actualmente solo se revisa si el nombre existe:

```cpp
if (!scope_->contains(n.GetName()))
    report(n.span, "SEM_UNDECLARED_VAR", n.GetName());
resolve(n.GetValue());
```

En HULK, `self` no debe ser target válido de asignación destructiva.

## Caso que falla

```hulk
type A {
    f() => self := new A();
}
new A().f();
```

Debe fallar.

## Resultado incorrecto actual

Si `self` está en el scope, `self := ...` puede pasar.

## Arreglo recomendado

En `visit(DestructiveAssign&)`:

```cpp
if (n.GetName() == "self") {
    report_raw(n.span, "'self' no es un target válido de asignación destructiva.");
}
```

Luego resolver el valor para evitar cascadas incompletas:

```cpp
resolve(n.GetValue());
```

## Test mínimo

```hulk
type A { f() => self := new A(); }
new A().f();
```

Debe reportar error semántico.

---

# 15. Riesgo de linkeo por incluir `.cpp` dentro de `.cpp`

## Dónde ocurre

Archivos:

- `binding.cpp`
- `semantic.cpp`

Contenido actual:

```cpp
// binding.cpp
#include "symbol_resolver.cpp"
```

```cpp
// semantic.cpp
#include "semantic_tables.cpp"
#include "analyzer.cpp"
```

Esto puede funcionar como unity build, pero es frágil. Si el sistema de build compila también `symbol_resolver.cpp`, `semantic_tables.cpp` o `analyzer.cpp` por separado, aparecerán errores de símbolos duplicados en linkeo.

## Caso que falla

Un `CMakeLists.txt` típico podría tener:

```cmake
add_library(hulk_semantic
    semantic.cpp
    analyzer.cpp
    semantic_tables.cpp
)

add_library(hulk_binding
    binding.cpp
    symbol_resolver.cpp
)
```

Eso compilaría las mismas definiciones dos veces.

## Resultado incorrecto actual

Riesgo de error de linkeo por One Definition Rule.

## Arreglo recomendado

Opción recomendada: no incluir `.cpp` dentro de `.cpp`.

```cmake
add_library(hulk_semantic
    analyzer.cpp
    semantic_tables.cpp
)

add_library(hulk_binding
    symbol_resolver.cpp
)
```

Y eliminar `semantic.cpp`/`binding.cpp`, o dejarlos vacíos si se usan como agregadores solo de headers.

Opción alternativa: si se quiere unity build, entonces documentarlo y excluir explícitamente los `.cpp` incluidos del target.

---

# Orden recomendado de arreglo

## Prioridad crítica

1. Arreglar `StaticScope::lookup()` para respetar shadowing real.
2. Separar contextos: global, función, atributo, método.
3. Corregir reglas de `self` y `base()`.
4. Registrar builtins y tipos base (`Object`, `Number`, `String`, `Boolean`).
5. Validar anotaciones de tipo desconocidas.

## Prioridad alta

6. Detectar atributos duplicados y parámetros duplicados.
7. Validar redefinición de métodos heredados.
8. Crear símbolos sintéticos para `for`, vector generator y `self`.
9. Resolver miembros sobre `self` en el corte 7.

## Prioridad media

10. Integrar builtins como resoluciones formales.
11. Reorganizar build para evitar incluir `.cpp` dentro de `.cpp`.

---

# Checklist de pruebas sugeridas

Usar estos programas como tests negativos/positivos para cerrar el corte 7.

## 1. Shadowing correcto

Debe pasar y resolver `x` interno como parámetro:

```hulk
let x = 1 in ((x) => x)(2);
```

## 2. `self` prohibido en atributo

Debe fallar:

```hulk
type A {
    x = self;
}
new A();
```

## 3. Parámetro de constructor no visible en método

Debe fallar:

```hulk
type A(x) {
    f() => x;
}
new A(1).f();
```

## 4. Uso correcto de atributo guardado

Debe pasar:

```hulk
type A(x) {
    x = x;
    f() => self.x;
}
new A(1).f();
```

## 5. `base()` en tipo sin padre

Debe fallar:

```hulk
type A {
    f() => base();
}
new A().f();
```

## 6. Override con aridad inválida

Debe fallar:

```hulk
type A {
    f(x: Number): Number => x;
}

type B inherits A {
    f(): Number => 0;
}
new B().f();
```

## 7. Miembro inexistente en `self`

Debe fallar:

```hulk
type A {
    f() => self.noExiste();
}
new A().f();
```

## 8. Variable sintética del `for`

Debe pasar y dejar `x` resuelto:

```hulk
for (x in range(0, 3)) print(x);
```

## 9. Tipo anotado desconocido

Debe fallar:

```hulk
function f(x: Missing): Number => 1;
print(f(0));
```

## 10. Herencia desde builtin

Debe fallar:

```hulk
type Bad inherits Number {}
new Bad();
```

## 11. Atributo duplicado

Debe fallar:

```hulk
type A {
    x = 1;
    x = 2;
}
new A();
```

## 12. Builtin con aridad incorrecta

Debe fallar:

```hulk
sqrt(1, 2);
```

## 13. Constante builtin

Debe pasar:

```hulk
print(PI);
print(E);
```

## 14. Parámetro duplicado

Debe fallar:

```hulk
function f(x, x) => x;
print(f(1, 2));
```

## 15. Asignación a `self`

Debe fallar:

```hulk
type A {
    f() => self := new A();
}
new A().f();
```

---

# Propuesta mínima de cambios estructurales

## `StaticScope`

Reemplazar búsquedas separadas por un lookup unificado:

```cpp
enum class ScopeSymbolKind { Binding, Param, Synthetic };

struct ScopeSymbol {
    ScopeSymbolKind kind;
    VariableBinding* binding = nullptr;
    const Param* param = nullptr;
    SyntheticSymbol* synthetic = nullptr;
};

std::optional<ScopeSymbol> lookup(const std::string& name) const;
```

## `ResolutionKind`

Extender:

```cpp
enum class ResolutionKind {
    Variable,
    Param,
    Synthetic,
    Function,
    BuiltinFunction,
    BuiltinConstant,
    Type,
    Method,
    Attribute,
    Unresolved
};
```

## `SymbolResolver`

Agregar contexto:

```cpp
enum class ResolverContext {
    Global,
    Function,
    TypeAttributeInit,
    Method
};

ResolverContext context_ = ResolverContext::Global;
std::string current_type_name_;
std::string current_func_name_;
std::string current_method_name_;
```

Agregar helpers:

```cpp
bool is_known_type_name(const std::string& name) const;
void check_type_annotation(const Span& span, const std::string& type_name);
void check_duplicate_params(const std::vector<Param>& params,
                            const Span& span,
                            const std::string& owner);
const SemanticAttrInfo* find_attribute(const std::string& type_name,
                                       const std::string& attr_name) const;
```

## `SemanticTables`

Agregar inicialización builtin:

```cpp
SemanticTables();
void register_builtin_type(const std::string& name, const std::string& parent);
const BuiltinFuncInfo* lookup_builtin_func(const std::string& name) const;
const BuiltinConstInfo* lookup_builtin_const(const std::string& name) const;
```

## `SemanticTypeInfo`

Agregar:

```cpp
bool is_builtin = false;
```

Y considerar cambiar atributos de `vector` a `unordered_map` si se quiere lookup rápido:

```cpp
std::unordered_map<std::string, SemanticAttrInfo> attributes;
```

Si se mantiene `vector`, agregar helpers de búsqueda.

---

# Conclusión

El corte 7 está bien orientado, pero no debe pasar a inferencia todavía sin estos arreglos mínimos:

- lookup léxico correcto;
- `self`/`base()` con contexto real;
- tipos builtin y `Object` en tablas;
- validación de anotaciones;
- resolución real para símbolos sintéticos;
- detección de duplicados;
- verificación básica de overrides;
- resolución parcial de miembros sobre `self`.

Si estos puntos no se corrigen, el corte 8 tendrá que inferir sobre un AST con referencias incompletas o erróneas, y el corte 9 terminará reportando errores tarde, con spans menos precisos o con diagnósticos en cascada.
