# FORJA: Entorno Integral de Compilación WebAssembly

Bienvenido a la **Forja**. Este documento es la guía definitiva para preparar tu máquina (Windows, Linux o macOS) como una estación de trabajo capaz de compilar código fuente de múltiples lenguajes pesados hacia el estándar web de alto rendimiento: **WebAssembly (WASM)**.

Actualmente, WebAssembly es un formato agnóstico. Aunque nació fuertemente ligado a C y C++, hoy en día soporta un ecosistema gigantesco. Aquí te mostramos cómo forjar tu entorno para **C/C++**, **Rust** y **Java**.

---

## 1. El Horno Principal: C y C++ (Emscripten)

Emscripten es el compilador estándar de la industria (basado en LLVM) para portar proyectos C/C++ (como este proyecto de Raylib) a la web.

### Instalación (Windows / Linux / macOS)
Abre tu terminal y ejecuta paso a paso:

```bash
# 1. Clona el repositorio oficial del SDK de Emscripten
git clone https://github.com/emscripten-core/emsdk.git

# 2. Entra al directorio
cd emsdk

# 3. Descarga e instala la última versión de las herramientas
# (En Windows usa `emsdk.bat` en lugar de `./emsdk`)
./emsdk install latest

# 4. Activa la versión recién instalada
./emsdk activate latest

# 5. Inyecta las variables de entorno a tu terminal actual
# En Linux/macOS:
source ./emsdk_env.sh
# En Windows (CMD):
emsdk_env.bat
# En Windows (PowerShell):
.\emsdk_env.ps1
```

**Verificación:** 
Ejecuta `emcc -v`. Si muestra la versión de Emscripten, tu forja para C/C++ está lista.

---

## 2. El Martillo Moderno: Rust

Rust tiene un soporte de primera clase (Tier 1/2) para WebAssembly. Es el lenguaje preferido para crear librerías puras de WASM debido a su estricto control de memoria sin recolector de basura (Garbage Collector).

### Instalación
Si no tienes Rust instalado, instálalo con *rustup*:
```bash
# Linux/macOS
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# En Windows, descarga rustup-init.exe desde: https://rustup.rs/
```

Una vez tengas el comando `cargo`, añade el "target" (objetivo) de compilación de WebAssembly y la herramienta de empaquetado web:

```bash
# 1. Añade la arquitectura WASM al compilador de Rust
rustup target add wasm32-unknown-unknown

# 2. Instala wasm-pack (El orquestador oficial para empaquetar Rust hacia NPM/Web)
cargo install wasm-pack
```

**Comando de Compilación Rápida (Ejemplo):**
Para compilar un proyecto de Rust a WASM listo para la web, simplemente te sitúas en la carpeta de tu proyecto de Rust y ejecutas:
```bash
wasm-pack build --target web
```

---

## 3. El Yunque Empresarial: Java

Llevar Java a WebAssembly es un desafío mayor debido a la necesidad de transpilar o emular la JVM (Java Virtual Machine) y su Recolector de Basura (GC). Existen tres grandes aproximaciones, pero la más recomendada para aplicaciones web modernas es **TeaVM** o **CheerpJ**. A continuación, los pasos para usar **TeaVM** (la opción Open Source nativa más robusta).

### Instalación (Basada en Maven)
Para compilar Java a WebAssembly/JS, no instalas un compilador en tu sistema operativo, sino que integras un "Plugin" en tu gestor de dependencias (Maven o Gradle).

1. Asegúrate de tener instalado el **JDK 11 o superior** y **Maven**.
2. En tu archivo `pom.xml` de Java, añade el plugin de compilación de TeaVM:

```xml
<build>
    <plugins>
        <plugin>
            <groupId>org.teavm</groupId>
            <artifactId>teavm-maven-plugin</artifactId>
            <version>0.9.0</version>
            <executions>
                <execution>
                    <goals>
                        <goal>compile</goal>
                    </goals>
                    <configuration>
                        <!-- Define la clase que contiene tu public static void main -->
                        <mainClass>com.miempresa.juego.Main</mainClass>
                        <!-- Le decimos a TeaVM que genere binarios WebAssembly -->
                        <targetType>WEBASSEMBLY</targetType>
                        <targetDirectory>${project.build.directory}/wasm</targetDirectory>
                    </configuration>
                </execution>
            </executions>
        </plugin>
    </plugins>
</build>
```

**Comando de Compilación Rápida (Ejemplo):**
Para compilar tu proyecto de Java hacia WebAssembly, abres la terminal en tu proyecto y ejecutas:
```bash
mvn clean package
```
Esto generará un archivo `.wasm` y un archivo `classes.js` (el pegamento) en tu carpeta de salida, listos para ser consumidos en una página HTML de la misma forma que Emscripten.

---

## Resumen de Comandos Rápidos de la Forja

| Lenguaje | Comando de Compilación hacia WASM |
|----------|------------------------------------|
| **C/C++** | `emcc archivo.c -o salida.js -s ASYNCIFY` |
| **Rust** | `wasm-pack build --target web` |
| **Java** | `mvn clean package` (con plugin TeaVM config. en WEBASSEMBLY) |
