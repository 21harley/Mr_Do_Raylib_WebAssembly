import React from 'react';
import GameComponent from './GameComponent.jsx';

function App() {
  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', backgroundColor: '#222', minHeight: '100vh', color: 'white', fontFamily: 'sans-serif', paddingTop: '2rem' }}>
      <h1>React + WebAssembly (Mr. Do!)</h1>
      <p style={{ marginBottom: '2rem', color: '#ccc' }}>Ejemplo de integración autocontenida usando React 18 y Vite.</p>
      
      {/* Componente que renderiza el Canvas de Emscripten */}
      <GameComponent />
      
    </div>
  );
}

export default App;
