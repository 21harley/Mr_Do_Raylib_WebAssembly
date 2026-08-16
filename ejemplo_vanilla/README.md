# Integración Bare-Metal: WebAssembly en Vanilla JS

## 🚀 Cómo poner en marcha este proyecto

Este es un proyecto HTML5/JS puro (Vanilla). No necesitas instalar dependencias de Node.js ni usar empaquetadores.

1. **Abre una terminal** en esta misma carpeta (`ejemplo_vanilla`).
2. **Inicia un servidor web local** para evitar problemas de CORS al cargar el binario. Puedes usar Python:
   ```bash
   python -m http.server 8080
   ```
   *(Alternativas: `npx serve` o la extensión Live Server de VSCode).*
3. **Abre tu navegador** y visita: [http://localhost:8080](http://localhost:8080).
4. Haz clic en el botón de la pantalla inicial para desbloquear el contexto de audio y arrancar el motor.

---

## 📂 Anatomía de los Archivos

Este entorno autocontenido está compuesto por los siguientes archivos esenciales:

* **`index.html`**: Es el punto de entrada de la aplicación. Contiene la etiqueta `<canvas>` donde Raylib dibuja los gráficos usando WebGL. Además, maneja la lógica de la Interfaz de Usuario (UI) inicial para cumplir con las políticas de *Autoplay* del navegador, inyectando dinámicamente el motor lógico solo tras una interacción explícita del jugador.
* **`juego.js`** *(Generado por Emscripten)*: Es el código puente o *glue code*. Este script inicializa el entorno de ejecución, prepara el sistema de archivos virtual (VFS) y conecta las APIs nativas del navegador (DOM, WebAudio, WebGL) con las llamadas de sistema que espera el binario compilado en C.
* **`juego.wasm`** *(Generado por Emscripten)*: El binario de WebAssembly precompilado y altamente optimizado. Contiene la lógica pura, las matemáticas, las físicas y el comportamiento de la Inteligencia Artificial del juego.
* **`juego.data`** *(Generado por Emscripten)*: Un archivo empaquetado que contiene todos los recursos binarios (imágenes `.png` y sonidos `.wav`). El archivo `juego.js` lo lee y lo inyecta en la memoria RAM del navegador, simulando un disco duro virtual en la ruta raíz `/`.

## 🏗️ Propuesta de Arquitectura de Carpetas

Si deseas escalar este enfoque de Vanilla JS para un ecosistema más grande, te recomendamos aislar el motor de la lógica de interfaz web usando la siguiente estructura de directorios:

```text
mi-juego-vanilla/
├── assets/
│   ├── css/
│   │   └── estilos.css         # Estilos para menús HTML, el HUD y el canvas
│   └── img/
│       └── logo-web.png        # Imágenes usadas solo por el DOM, ajenas al canvas
├── engine/
│   ├── juego.js                # Binarios compilados arrojados por Emscripten
│   ├── juego.wasm
│   └── juego.data
├── js/
│   ├── main.js                 # Lógica global, inyección del motor y control de menús HTML
│   └── interop.js              # Envoltorios (Wrappers) usando ccall/cwrap para comunicarse con C
└── index.html                  # Punto de entrada principal
```

---

## Análisis Técnico: ¿Cuándo utilizar el enfoque Vanilla JS?

En escenarios de alto rendimiento y latencia crítica, la aproximación *Bare-Metal* de Vanilla JS es arquitectónicamente superior por las siguientes razones:

### 1. Control Estricto del Event Loop
Emscripten, en combinación con Raylib, depende de la API nativa `requestAnimationFrame` del navegador para sincronizar el bucle iterativo del juego (*Game Loop*) con la tasa de refresco del monitor (generalmente 60Hz o 144Hz). Al utilizar Vanilla JS, se elimina cualquier capa intermedia de abstracción (como un Virtual DOM) que pueda introducir *jitter* o micro-pausas debido a la recolección de basura (*Garbage Collection*). La invocación del contexto de WebGL ocurre de forma directa e ininterrumpida.

### 2. Eliminación del Overhead de Reconciliación
Los frameworks modernos basados en componentes (React, Vue, Svelte) ejecutan ciclos constantes de comparación de estado (*Diffing*) en el hilo principal (*Main Thread*). Dado que WebAssembly bloquea el hilo principal de JavaScript durante el renderizado de cada cuadro, cualquier sobrecarga algorítmica inducida por un framework externo corre el riesgo de causar caídas de cuadros (*Frame Drops*). Vanilla JS garantiza que el 100% del ciclo de CPU asignado a la pestaña se devore exclusivamente por la lógica computacional en C.

### 3. Sistemas Embebidos Simples
Si el producto web interactivo no requiere de interfaces de usuario HTML superpuestas masivas (sistemas de autenticación complejos, paneles de administración, chats asíncronos en vivo), aislar el motor en Vanilla JS evita cargar pesadas bibliotecas frontend, optimizando el tiempo de carga inicial y reduciendo drásticamente la huella de memoria en dispositivos móviles.
