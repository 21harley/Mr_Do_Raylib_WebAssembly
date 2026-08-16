@echo off
echo ====================================================
echo Pipeline de Compilacion WebAssembly (Emscripten)
echo ====================================================

REM FASE 1: Verificacion de Entorno
REM Se asume que el usuario ejecutara o ya ejecuto su entorno EMSDK (ej. emsdk_env.bat)
where emcc >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] No se encuentra 'emcc' en el sistema.
    echo Por favor, abre la consola de Emscripten o ejecuta 'emsdk_env.bat' 
    echo en tu instalacion de EMSDK antes de ejecutar este script.
    pause
    exit /b 1
)

echo [INFO] Iniciando transpilacion cruzada (C a WebAssembly)...

REM FASE 2: Transpilacion Cruzada
REM Se compila hacia juego.js para permitir la integracion en nuestro index.html
emcc main.c -o juego.js -O2 ^
    -I raylib-web/include ^
    -L raylib-web/lib ^
    -lraylib ^
    -s USE_GLFW=3 ^
    -s FORCE_FILESYSTEM=1 ^
    -s ASYNCIFY ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s INITIAL_MEMORY=134217728 ^
    --preload-file imagenes ^
    --preload-file sonido

if %errorlevel% equ 0 (
    echo.
    echo [EXITO] Compilacion completada satisfactoriamente.
    echo Archivos generados:
    echo  - juego.js   ^(Script de Enlace^)
    echo  - juego.wasm ^(Binario Optimizado^)
    echo  - juego.data ^(Paquete de Recursos Virtual FS^)
    echo.
    
    echo [INFO] Propagando binarios a los entornos de ejemplo...
    copy /Y juego.js ejemplo_vanilla\ >nul
    copy /Y juego.wasm ejemplo_vanilla\ >nul
    copy /Y juego.data ejemplo_vanilla\ >nul
    
    if not exist ejemplo_react\public mkdir ejemplo_react\public
    copy /Y juego.js ejemplo_react\public\ >nul
    copy /Y juego.wasm ejemplo_react\public\ >nul
    copy /Y juego.data ejemplo_react\public\ >nul
    echo [EXITO] Archivos actualizados en ejemplo_vanilla y ejemplo_react/public.
    echo.

    echo [INFO] Para probar el juego, inicia un servidor web local en la carpeta del ejemplo que desees.
    echo Ejemplo: cd ejemplo_vanilla ^&^& python -m http.server 8080
    echo Y abre http://localhost:8080 en tu navegador web.
) else (
    echo.
    echo [ERROR] Ocurrio un fallo durante la compilacion.
)

pause
