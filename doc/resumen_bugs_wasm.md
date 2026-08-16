# Reporte Técnico: Portabilidad a WebAssembly (Raylib)

Este documento resume los problemas técnicos encontrados durante la compilación del proyecto "Mr. Do" de escritorio hacia la web (HTML5/WASM), cómo se solucionaron, y las buenas prácticas para evitar que ocurran en el futuro.

## 1. Ausencia del Entorno Emscripten (Command not found)
* **El Problema:** Al ejecutar el script por lotes, la terminal no reconocía el comando `emcc`.
* **La Causa:** Emscripten no se instala como un programa tradicional en Windows. Requiere configurar variables de entorno temporales por cada sesión de terminal.
* **La Solución:** Instalamos el `emsdk` (Emscripten SDK) y establecimos que siempre que vayas a compilar, la consola debe tener inyectadas las variables usando el script `emsdk_env.ps1`.
* **Prevención:** Si abres una consola nueva, recuerda siempre inicializar el entorno de Emscripten antes de intentar usar `emcc`.

## 2. Código Obsoleto (APIs de Raylib 4.x)
* **El Problema:** El código de C no compilaba y arrojaba `error: call to undeclared function 'IsMusicValid'`.
* **La Causa:** El código original fue escrito para una versión más antigua de Raylib. En Raylib 5.0, los creadores de la librería renombraron varias funciones relacionadas al estado de los recursos para estandarizarlas.
* **La Solución:** Se aplicó una refactorización masiva en `main.c` cambiando funciones como `IsSoundValid` por `IsSoundReady`, y `IsMusicValid` por `IsMusicReady`.
* **Prevención:** Al actualizar el motor gráfico, revisa el archivo `CHANGELOG` oficial para conocer las funciones que han sido renombradas o eliminadas.

## 3. Desajuste de Memoria y Colapso del Decodificador (OOM)
* **El Problema:** El juego compilaba pero la consola del navegador mostraba `RuntimeError: memory access out of bounds` justo al cargar los efectos de sonido.
* **La Causa:** La arquitectura WebAssembly es muy estricta con la memoria (por defecto solo asigna 16 MB). Además, el decodificador de MP3 interno de Raylib (`miniaudio`) tiene un *bug* conocido en la web cuando intenta descomprimir archivos `.mp3` pequeños de golpe a la RAM usando `LoadSound()`.
* **La Solución:** 
  1. Convertimos los efectos de sonido cortos de `.mp3` a `.wav` (formato de onda puro), lo que evita por completo usar el descompresor en tiempo real.
  2. Aumentamos la memoria inicial a 128 MB (`-s INITIAL_MEMORY=134217728`) y permitimos el crecimiento dinámico (`-s ALLOW_MEMORY_GROWTH=1`).
* **Prevención:** **Regla de Oro en WASM:** Usa `.mp3` (o `.ogg`) SÓLO para música larga de fondo cargada mediante `LoadMusicStream()`. Para todo lo demás (efectos cortos como saltos, explosiones, monedas), **usa siempre `.wav`** y cárgalos con `LoadSound()`.

## 4. Incompatibilidad de Compiladores (ABI Mismatch)
* **El Problema:** Tras arreglar la memoria, surgió un `RuntimeError: table index is out of bounds` al momento en que el navegador intentaba darle permisos de audio al juego.
* **La Causa:** Descargamos una versión pre-compilada de Raylib (`libraylib.a`) de internet que fue construida con una versión de Emscripten más vieja que la que teníamos instalada en la computadora. Los índices de las tablas de funciones de JavaScript no coincidían.
* **La Solución:** Usamos tu propio compilador Emscripten recién instalado para compilar el código fuente completo de Raylib 5.0 desde cero, creando un `libraylib.a` 100% compatible con tu entorno local.
* **Prevención:** Nunca mezcles binarios compilados estáticos de WebAssembly hechos por distintas versiones de `emcc`. Si actualizas Emscripten en el futuro, recompila tus librerías externas.

## 5. Congelamiento del Bucle Principal en el Navegador
* **El Problema:** El juego carga todo correctamente pero la pantalla se queda gris o en blanco, "congelada".
* **La Causa:** En PC, el bucle de juego es un ciclo infinito `while(!WindowShouldClose())`. Sin embargo, los navegadores de internet (Google Chrome, Firefox) funcionan en un solo hilo (Single-threaded). Si entras en un ciclo infinito en C, el navegador jamás recibe "permiso" o "tiempo" para dibujar el Frame en la pantalla de la página web.
* **La Solución:** Agregamos la poderosa bandera `-s ASYNCIFY` al momento de compilar. Esto es una tecnología de WebAssembly que inyecta pausas invisibles en el ciclo `while` de C, devolviéndole el control al navegador 60 veces por segundo de manera automática, sin tener que cambiar la estructura de tu código.
* **Prevención:** En proyectos más estrictos, en lugar de usar `ASYNCIFY` (que hace el juego ligeramente más pesado), se recomienda estructurar el bucle principal dentro de una función independiente y utilizar `emscripten_set_main_loop()`. Sin embargo, para este proyecto de prueba, `ASYNCIFY` es la solución ideal.

## 6. Bloqueo de Audio por Políticas del Navegador (Autoplay)
* **El Problema:** El juego cargaba correctamente pero no emitía ningún sonido o música. 
* **La Causa:** Los navegadores modernos (Chrome, Edge, Firefox) tienen estrictas políticas de seguridad que impiden que cualquier página web reproduzca audio automáticamente sin que el usuario haya interactuado físicamente con ella primero (haciendo un clic o presionando una tecla).
* **La Solución:** Retrasamos la carga del motor. Creamos una capa inicial (`#start-overlay`) en el `index.html` con un botón de "Activar Sonido y Jugar". El archivo WebAssembly (`juego.js`) no se inyecta dinámicamente hasta que el usuario hace clic. Esto garantiza que el entorno de audio (AudioContext) nazca con permisos totales desde el primer milisegundo.
* **Prevención:** Al portar juegos a la web, jamás inicies la lógica de `InitAudioDevice()` silenciosamente en segundo plano. Siempre condiciona la ejecución a una interacción explícita del usuario.

## 7. Error de Memoria en el Audio de WebAssembly (`reading 'buffer'`)
* **El Problema:** Al cargar el juego, la consola del navegador se inundaba de errores rojos arrojando: `Uncaught TypeError: Cannot read properties of undefined (reading 'buffer') at device.scriptNode.onaudioprocess` y el audio colapsaba.
* **La Causa:** Incompatibilidad entre el motor de audio interno de Raylib (`miniaudio`) y las nuevas versiones de Emscripten al usar `-s ALLOW_MEMORY_GROWTH=1`. El código incrustado de Raylib espera encontrar en JavaScript el mapa de memoria en `Module.HEAPF32.buffer`. Sin embargo, por optimización de espacio, las nuevas versiones de Emscripten ya no "exportan" la variable global `HEAPF32` dentro del objeto `Module` de manera automática.
* **La Solución:** Añadimos un pequeño "parche" dentro de la función `onRuntimeInitialized` en el archivo `index.html`: `if (typeof HEAPF32 !== 'undefined') Module.HEAPF32 = HEAPF32;`. Esto reconecta manualmente la memoria cruda al motor, devolviéndole a Raylib la capacidad de leer sus audios.
* **Prevención:** Si trabajas con Emscripten moderno y Raylib, recuerda que los búferes de memoria (`HEAPF32`, `HEAP8`) son privados por defecto. Debes vincularlos manualmente o configurar Emscripten para que los exporte si notas que fallan librerías que dependen de la memoria directa en JS.

---

### Recomendación de Compilación Final
A partir de ahora, puedes seguir modificando tu `main.c` como siempre. Solo asegúrate de mantener el comando de compilación en `compilar_wasm.bat` intacto, ya que contiene toda la "medicina" necesaria para un WebAssembly perfecto.
