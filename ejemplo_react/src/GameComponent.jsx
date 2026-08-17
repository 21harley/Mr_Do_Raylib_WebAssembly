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

/* Paleta del portafolio — dark mode azul-violeta */
const C = {
  bgCero:  '#0f1014',
  bgOne:   '#1b1d25',
  bgTwo:   '#444856',
  bgThree: '#dfe3ff',
  bgFour:  '#929ab1',
  bgFive:  '#2c303a',
  font:    "'Courier New', Courier, monospace",
};

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
    <div style={{ position: 'relative', width: '800px', margin: '0 auto', fontFamily: C.font }}>

      {/* CAPA DE DESBLOQUEO DE AUDIO (Autoplay Policy) */}
      {!isAudioUnlocked && (
        <div style={{
          position: 'absolute',
          top: 0, left: 0, right: 0, bottom: 0,
          backgroundColor: 'rgba(15, 16, 20, 0.92)',   /* bgCero con opacidad */
          color: C.bgThree,
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'center',
          alignItems: 'center',
          gap: '1.25rem',
          zIndex: 10,
          fontFamily: C.font,
        }}>
          <h2 style={{ margin: 0, letterSpacing: '0.06em', color: C.bgThree }}>
            Mr. Do! — React Port
          </h2>
          <p style={{ margin: 0, color: C.bgFour, letterSpacing: '0.04em', textAlign: 'center' }}>
            Haz clic para habilitar el motor de WebAssembly y el audio.
          </p>
          <button
            onClick={handleStartJuego}
            style={{
              padding: '10px 24px',
              fontSize: '1rem',
              cursor: 'pointer',
              backgroundColor: C.bgFive,
              color: C.bgThree,
              border: `1px solid ${C.bgTwo}`,
              borderRadius: '4px',
              fontFamily: C.font,
              fontWeight: 'bold',
              letterSpacing: '0.06em',
              transition: 'background-color 0.25s ease, border-color 0.25s ease',
            }}
            onMouseEnter={e => {
              e.currentTarget.style.backgroundColor = C.bgTwo;
              e.currentTarget.style.borderColor = C.bgThree;
            }}
            onMouseLeave={e => {
              e.currentTarget.style.backgroundColor = C.bgFive;
              e.currentTarget.style.borderColor = C.bgTwo;
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
            border: `1px solid ${C.bgOne}`,
            boxShadow: `0 8px 32px rgba(0,0,0,0.8), 0 0 0 1px ${C.bgOne}`,
            display: 'block',
            backgroundColor: C.bgCero,
        }}
      />
    </div>
  );
};

export default GameComponent;
