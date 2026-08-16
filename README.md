# Mr. Do! - Proyecto Raylib a WebAssembly (WASM)

Este proyecto contiene el código fuente, los recursos y la infraestructura necesaria para compilar el clásico juego "Mr. Do" (desarrollado originalmente en C para Windows/PC) hacia la web usando Raylib y Emscripten.

## Estructura del Proyecto

A continuación se detalla la función de cada documento y directorio principal en la raíz del proyecto:

### Archivos Principales
* **`main.c`**: El código fuente en C de toda la lógica del juego (físicas, renderizado, IA de los enemigos, y bucle principal adaptado a la web con `ASYNCIFY`).
* **`compilar_wasm.bat`**: El script de línea de comandos maestro que invoca a Emscripten (`emcc`). Contiene todas las banderas de compilación (flags) exactas y seguras para generar la versión WebAssembly sin errores.
* **`highscore.dat`**: Archivo de texto plano (persistente en el Virtual FS) que guarda de forma local la mejor puntuación histórica obtenida.
* **Archivos Dev-C++** (`Proyecto1.dev`, `Makefile.win`, `Proyecto1.layout`, `Proyecto1.exe`): Archivos heredados del entorno de desarrollo original usado para compilar la versión de escritorio para Windows.

### Carpetas de Recursos y SDK
* **`doc/`**: Contiene la documentación técnica del proyecto. 
  - El archivo `resumen_bugs_wasm.md` guarda un registro de todos los problemas y soluciones aplicadas durante la adaptación web.
  - El archivo `forja.md` contiene la guía maestra de herramientas e instalación de compiladores WebAssembly para C/C++, Rust y Java.
* **`imagenes/`**: Directorio donde se almacenan las texturas PNG (personajes, enemigos, frutas).
* **`sonido/`**: Almacena todos los efectos en formato corto (`.wav`) y la música de fondo (`.mp3`).
* **`raylib-src/`**: Carpeta que contiene el código fuente puro de la librería Raylib, por si fuera necesario recompilarla en el futuro.
* **`raylib-web/`**: Contiene la versión pre-compilada estática (`libraylib.a`) para WebAssembly y sus cabeceras (`include`), emparejadas para el entorno Emscripten.

### Carpetas de Ejemplos de Integración (Web)
* **`ejemplo_vanilla/`**: Contiene una implementación en **HTML5 puro** (Vanilla JS) usando el `index.html` modificado con inyección dinámica para soportar las políticas de Autoplay.
* **`ejemplo_react/`**: Contiene la estructura y un componente moderno de muestra para montar el motor WebAssembly dentro del ecosistema de componentes de **React**.

---

## Despliegue en Entornos Web (Vanilla JS y React)

Para correr el compilado final del juego en **cualquier** entorno web moderno, Emscripten genera siempre tres archivos esenciales. **Estos son los únicos archivos que necesitas alojar en tu servidor público (`/public`, `htdocs`, etc.):**

1. **`juego.wasm`**: El binario puro súper rápido y optimizado (contiene la lógica).
2. **`juego.data`**: Un archivo empaquetado que contiene todas las imágenes y sonidos (Virtual File System).
3. **`juego.js`**: El código JavaScript "pegamento" o *glue code*. Este archivo le enseña al navegador web cómo hablar e interactuar con el `.wasm` e inicializa el mapa de memoria.

### Integración en Proyecto Vanilla JS (HTML Puro)
Solo necesitas mover esos tres archivos a una misma carpeta pública y crear un archivo `index.html`.
* En tu archivo HTML debes tener una etiqueta `<canvas id="canvas"></canvas>`.
* La forma correcta de iniciar el juego es inyectar el archivo `juego.js` dinámicamente **después** de que el usuario haga clic en la página (para evitar que el navegador bloquee el audio por sus reglas de Autoplay).
* *Revisa la carpeta `ejemplo_vanilla` para ver el código en acción.*

### Integración en Proyecto React
Mueve los tres archivos (`juego.js`, `juego.wasm`, `juego.data`) a la carpeta `public/` de tu proyecto React (fuera de `src/`).
* Dentro de React, no debes importar `juego.js` como un módulo de Node. En su lugar, debes crear un componente visual que renderice el `<canvas>`.
* Luego, usas un bloque `useEffect()` vacío (`[]`) para crear un `<script>` programáticamente y agregarlo al DOM una sola vez al montar el componente.
* *Revisa la carpeta `ejemplo_react` para ver y copiar el componente base `GameComponent.jsx` y su guía paso a paso.*

---

## 🤝 Guía para Contribuidores (Cómo hacer un Fork)

Este repositorio está optimizado usando **NPM** como orquestador de proyectos para mantener un control de versiones limpio y profesional. Los archivos binarios precompilados y el código fuente pesado de la librería en C (Raylib) han sido ignorados en Git (`.gitignore`) a propósito.

Si deseas clonar este proyecto en tu propia computadora o realizar un fork, sigue estos pasos:

1. **Clona el repositorio** en tu máquina:
   ```bash
   git clone <url-de-tu-repositorio>
   cd mrdo-wasm-project
   ```

2. **Inicializa el Entorno (Node.js)**:
   Este proyecto utiliza `npm` para gestionar scripts cruzados. Asegúrate de tener Node.js instalado y ejecuta:
   ```bash
   npm install
   ```

3. **Descarga las Dependencias en C (Raylib)**:
   En lugar de lidiar manualmente con GitHub, usa nuestro script automático que descargará el código fuente faltante en C:
   ```bash
   npm run setup
   ```
   *(Este comando creará la carpeta `raylib-src` necesaria para compilar)*.

4. **Compila y Juega**:
   Si ya tienes instalado Emscripten (`emcc`), puedes compilar el juego a WebAssembly automáticamente escribiendo:
   ```bash
   npm run build
   ```
   
Y finalmente, levanta el servidor web que prefieras para probar:
```bash
npm run start:vanilla
# O si prefieres React:
npm run start:react
```
