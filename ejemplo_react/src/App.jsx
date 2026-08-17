import React, { useState, useEffect } from 'react';
import GameComponent from './GameComponent.jsx';

/* ============================================================
 * Variables de la paleta del portafolio (dark mode azul-violeta)
 * Se usan como constantes JS para los estilos inline de React.
 * La fuente global se aplica en index.css.
 * ============================================================ */
const C = {
  bgCero:  '#0f1014',
  bgOne:   '#1b1d25',
  bgTwo:   '#444856',
  bgThree: '#dfe3ff',
  bgFour:  '#929ab1',
  bgFive:  '#2c303a',
  red:     'rgba(231, 76, 60, 0.8)',
  font:    "'Courier New', Courier, monospace",
};

function App() {
  const [isMobile, setIsMobile] = useState(false);

  useEffect(() => {
    const checkMobile = () => {
      const mobileRegex = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i;
      return mobileRegex.test(navigator.userAgent) || window.innerWidth < 768;
    };
    setIsMobile(checkMobile());
  }, []);

  return (
    <div style={{
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      backgroundColor: C.bgCero,
      minHeight: '100vh',
      color: C.bgThree,
      fontFamily: C.font,
      paddingTop: '2rem',
    }}>
      <h1 style={{ letterSpacing: '0.06em', marginBottom: '0.5rem' }}>
        React + WebAssembly (Mr. Do!)
      </h1>
      <p style={{ marginBottom: '2rem', color: C.bgFour, letterSpacing: '0.04em' }}>
        Ejemplo de integración autocontenida usando React 18 y Vite.
      </p>

      {isMobile ? (
        <div style={{
          backgroundColor: C.bgOne,
          padding: '2rem',
          borderRadius: '4px',
          textAlign: 'center',
          maxWidth: '400px',
          border: `1px solid ${C.red}`,
          boxShadow: '0 10px 30px rgba(0,0,0,0.6)',
          fontFamily: C.font,
        }}>
          <h2 style={{ color: C.red, marginBottom: '1rem', letterSpacing: '0.06em' }}>
            Dispositivo Móvil Detectado
          </h2>
          <p style={{ fontSize: '1rem', lineHeight: '1.6', color: C.bgFour }}>
            Lo sentimos, este juego requiere teclado y no está optimizado para dispositivos táctiles.<br/><br/>
            Por favor, ingresa desde una PC.
          </p>
        </div>
      ) : (
        <GameComponent />
      )}

    </div>
  );
}

export default App;
