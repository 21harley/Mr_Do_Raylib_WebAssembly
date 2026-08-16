# Inversión de Control: WebAssembly encapsulado en React

## 🚀 Cómo poner en marcha este proyecto

Este es un proyecto React completo impulsado por Vite. Los archivos binarios de WebAssembly generados por el motor en C siempre deben situarse dentro de la carpeta `public/`.

1. **Abre una terminal** en esta misma carpeta (`ejemplo_react`).
2. **Instala las dependencias** de Node.js (React y Vite):
   ```bash
   npm install
   ```
3. **Inicia el servidor de desarrollo**:
   ```bash
   npm run dev
   ```
4. **Abre tu navegador** en la dirección que arroje la terminal (generalmente `http://localhost:5173`).
5. Haz clic en el botón verde de inicio dentro del componente React para inyectar Emscripten y disfrutar del juego.

---

## 📂 Anatomía de los Archivos

Este entorno está configurado bajo el estándar de un empaquetador moderno (Vite). Cada archivo tiene una responsabilidad estricta:

* **`package.json`** y **`vite.config.js`**: Definen las dependencias del proyecto (React 18) y le dictan al empaquetador Vite cómo levantar el servidor local de desarrollo y cómo empaquetar el código para producción.
* **`index.html`**: La raíz principal del proyecto donde Vite inyecta el código JavaScript de React (montado en `<div id="root"></div>`).
* **`public/juego.js`, `juego.wasm`, `juego.data`**: Son los artefactos intocables producidos por el compilador en C (`emcc`). Al estar en `public/`, Vite ignora su contenido y los sirve exactamente como están (como rutas absolutas `/juego.js`), evitando corromper la memoria WebAssembly durante el proceso de *Bundling*.
* **`src/main.jsx`**: El punto de entrada clásico de React. Invoca `ReactDOM.createRoot` y monta el componente principal.
* **`src/App.jsx`**: El componente orquestador. Aquí podrías definir el layout global (menú superior, footer, barra lateral) y dentro de él, renderizar de forma aislada el canvas del juego.
* **`src/GameComponent.jsx`**: La obra maestra arquitectónica. Es un envoltorio (*Wrapper*) especializado que maneja de manera segura el ciclo de vida de WebAssembly dentro de React. Resuelve problemas comunes como la fuga de memoria (*Memory Leak*) al usar React 18 `StrictMode`, asegurando mediante referencias (`useRef`) que el script en C se inyecte de manera asíncrona una sola vez. También provee una interfaz para superar la política de Autoplay (Audio) del navegador.

## 🏗️ Propuesta de Arquitectura de Carpetas

A medida que el proyecto React crezca, podrías adoptar una arquitectura de **Micro-Frontends** donde el juego en C es un widget que se comunica con una tienda de estados global (como Redux o Zustand):

```text
mi-juego-react/
├── public/                     # Servido directamente al navegador
│   ├── juego.js
│   ├── juego.wasm
│   └── juego.data
├── src/
│   ├── store/                  
│   │   └── gameStore.js        # Estado global (Ej. Puntos guardados en Base de Datos)
│   ├── components/
│   │   ├── HUD.jsx             # Interfaz de Vida y Puntos en HTML/CSS superpuesta al canvas
│   │   ├── Inventory.jsx       # Inventario del jugador en DOM tradicional
│   │   └── GameComponent.jsx   # El envoltorio WebGL de WebAssembly
│   ├── App.jsx                 # Conecta el HUD, el Inventory y el GameComponent
│   └── main.jsx
├── package.json
└── vite.config.js
```

---

## Análisis Técnico: ¿Cuándo utilizar el enfoque React?

Tratar el binario de C como un componente más dentro del Virtual DOM de React acarrea desafíos, pero otorga beneficios supremos bajo las siguientes condiciones:

### 1. Desacoplamiento de Interfaces Complejas (UI Híbrido)
La mayor ventaja surge cuando el motor gráfico (`juego.wasm`) es solo un pilar dentro de un ecosistema web mucho más grande. Construir y estilizar interfaces de usuario sofisticadas (formularios, chats en vivo, listas dinámicas, pasarelas de pago) desde cero dentro de C++ usando WebGL es arquitectónicamente deficiente y terrible para la accesibilidad web (Lectores de Pantalla y SEO). 
Al encapsular WASM en React, puedes delegar todo este meta-juego a la asombrosa potencia declarativa del HTML/CSS, dibujando las ventanas de inventario y los menús *por encima* del `<canvas>`. De este modo, el núcleo en C únicamente se ocupa de la renderización matemática y gráfica del mundo 2D/3D.

### 2. Sincronización de Estado Global Moderno
Si tu plataforma requiere persistir información compleja en bases de datos a través de peticiones HTTP/GraphQL, o mantener un chat abierto mediante WebSockets, React y su inmenso ecosistema de librerías simplifican enormemente esta labor de red. Utilizando técnicas de interoperabilidad (*Interop ccall/cwrap*), React puede observar el estado asíncrono y enviar los datos directamente a la memoria de C, creando un flujo de información unidireccional altamente robusto.

### Desafíos Arquitectónicos: Políticas y StrictMode
Este modelo exige que el desarrollador gestione explícitamente el ciclo de vida del *Heap* de Emscripten. React 18 monta y desmonta constantemente los componentes durante el desarrollo. Puesto que WebAssembly no puede ser fácilmente "desmontado" y restaurado de la memoria del navegador, este componente implementa guardias de control estrictos para impedir dobles inicializaciones o el colapso del contexto activo de WebGL. Del mismo modo, el bloqueo imperativo (el botón de "Play") es la solución universal para instanciar el Audio Contexto del navegador de forma segura.
