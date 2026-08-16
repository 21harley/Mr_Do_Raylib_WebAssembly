import React, { useEffect, useRef, useState } from 'react';

/**
 * Componente de React para montar el motor WebAssembly (Raylib).
 * 
 * ATENCIÓN: Este componente requiere que los archivos:
 *  - juego.js
 *  - juego.wasm
 *  - juego.data
 * estén ubicados en la carpeta `public/` de tu proyecto React.
 */
const GameComponent = () => {
  const canvasRef = useRef(null);
  const scriptInjectedRef = useRef(false);
  const [isAudioUnlocked, setIsAudioUnlocked] = useState(false);

  useEffect(() => {
    // 1. Asignamos el canvas al objeto global "Module" que Emscripten busca por defecto.
    window.Module = {
      canvas: canvasRef.current,
      // Opcional: silenciar la consola de depuración del motor
      print: (text) => console.log(text),
      printErr: (text) => console.error(text),
    };

    // NOTA PARA DESARROLLO EN REACT STRICT MODE:
    // En React 18+, useEffect se ejecuta dos veces en desarrollo. 
    // Usamos scriptInjectedRef para asegurar que el motor WebAssembly no se cargue dos veces en el mismo DOM.
    return () => {
        // Limpieza básica (opcional, dependiendo del ciclo de vida del juego)
    };
  }, []);

  const handleStartJuego = () => {
    if (scriptInjectedRef.current) return;
    setIsAudioUnlocked(true);

    // 2. Inyectamos dinámica y explícitamente el script principal después 
    // del clic del usuario para asegurar que el navegador nos dé permiso de Audio.
    const script = document.createElement('script');
    script.src = '/juego.js'; // Ruta al archivo ubicado en public/juego.js
    script.async = true;
    
    // Opcional: puente manual para exponer HEAPF32 a miniaudio (bug de memoria en Emscripten)
    script.onload = () => {
      // Si onRuntimeInitialized no dispara a tiempo, puedes forzar la inicialización aquí
      // (dependiendo de la configuración de compilación de Emscripten).
    };

    document.body.appendChild(script);
    scriptInjectedRef.current = true;
  };

  return (
    <div style={{ position: 'relative', width: '800px', margin: '0 auto', fontFamily: 'sans-serif' }}>
      
      {/* CAPA DE DESBLOQUEO DE AUDIO (Autoplay Policy) */}
      {!isAudioUnlocked && (
        <div style={{
          position: 'absolute',
          top: 0, left: 0, right: 0, bottom: 0,
          backgroundColor: 'rgba(0, 0, 0, 0.8)',
          color: 'white',
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'center',
          alignItems: 'center',
          zIndex: 10,
        }}>
          <h2>Mr. Do! - React Port</h2>
          <p>Haz clic para habilitar el motor de WebAssembly y el audio.</p>
          <button 
            onClick={handleStartJuego} 
            style={{
              padding: '10px 20px', fontSize: '18px', cursor: 'pointer',
              backgroundColor: '#4CAF50', color: 'white', border: 'none', borderRadius: '5px'
            }}
          >
            Activar Sonido y Jugar
          </button>
        </div>
      )}

      {/* LIENZO DONDE SE DIBUJA EL JUEGO (WebGL) */}
      <canvas 
        ref={canvasRef} 
        id="canvas" 
        width={800} 
        height={450} 
        onContextMenu={(e) => e.preventDefault()}
        style={{
            border: '2px solid black',
            boxShadow: '0 4px 8px rgba(0,0,0,0.5)',
            display: 'block'
        }}
      />
    </div>
  );
};

export default GameComponent;
