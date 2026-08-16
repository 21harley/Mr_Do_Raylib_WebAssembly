const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

console.log("==========================================");
console.log("   Instalando Dependencias de C (Raylib)  ");
console.log("==========================================");

const RAYLIB_SRC_DIR = path.join(__dirname, '..', 'raylib-src');

// 1. Descargar el código fuente de Raylib
if (!fs.existsSync(RAYLIB_SRC_DIR)) {
    console.log("-> Descargando codigo fuente de Raylib (raylib-src)...");
    try {
        execSync('git clone --depth 1 https://github.com/raysan5/raylib.git raylib-src', { stdio: 'inherit' });
        console.log("[EXITO] raylib-src instalado.");
    } catch (e) {
        console.error("[ERROR] No se pudo clonar raylib-src. Asegurate de tener Git instalado.");
        process.exit(1);
    }
} else {
    console.log("-> raylib-src ya existe. Omitiendo descarga.");
}

// Nota: La carpeta raylib-web usualmente contiene la libreria libraylib.a precompilada.
// En un entorno de CI/CD avanzado, podriamos descargar las Releases estáticas desde el GitHub de Raylib:
// https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_webassembly.zip
// Por simplicidad del script asíncrono, si raylib-web falta, le indicamos al usuario que lo obtenga,
// aunque se puede expandir este script para usar el comando 'curl' y 'tar -xf' si fuera necesario.

console.log("\n[INFO] Dependencias nativas verificadas. Ya puedes correr: npm run build");
