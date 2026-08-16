import React, { useState, useEffect } from 'react';
import GameComponent from './GameComponent.jsx';

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
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', backgroundColor: '#222', minHeight: '100vh', color: 'white', fontFamily: 'sans-serif', paddingTop: '2rem' }}>
      <h1>React + WebAssembly (Mr. Do!)</h1>
      <p style={{ marginBottom: '2rem', color: '#ccc' }}>Ejemplo de integración autocontenida usando React 18 y Vite.</p>
      
      {isMobile ? (
        <div style={{ backgroundColor: '#1a1a1a', padding: '2rem', borderRadius: '8px', textAlign: 'center', maxWidth: '400px', border: '1px solid #e74c3c', boxShadow: '0 10px 30px rgba(0,0,0,0.5)' }}>
          <h2 style={{ color: '#e74c3c', marginBottom: '1rem' }}>Dispositivo Móvil Detectado</h2>
          <p style={{ fontSize: '1.1rem', lineHeight: '1.5', color: '#ccc' }}>
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
