#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "raylib.h"

#define FILAS 9
#define COLUMNAS 16
#define TAM_CELDA 40
#define OFFSET_X 80
#define OFFSET_Y 45
#define MAX_ENEMIGOS 5
#define MAX_MANZANAS 6
#define MAX_CEREZAS 56
#define MAX_PARTICULAS 64
#define MAX_TEXTOS_FLOTANTES 10
#define TIEMPO_RECARGA_PELOTA 10.0f
#define DISTANCIA_DETECCION 7
#define RADIO_PELOTA 6.0f

// --- CONSTANTES DE VELOCIDAD Y RITMO ---
#define TIEMPO_PASO_JUGADOR_VACIO   0.12f
#define TIEMPO_PASO_JUGADOR_TIERRA  0.22f
#define TIEMPO_PASO_ENEMIGO_TIERRA  (TIEMPO_PASO_JUGADOR_TIERRA * 2.0f)
#define VEL_PELOTA_MOD              260.0f
#define TIEMPO_PASO_ENEMIGO_BASE    0.38f
#define TIEMPO_PASO_ENEMIGO_MIN     0.22f
#define DECREMENTO_POR_NIVEL        0.02f
#define TIEMPO_TAMBALEO_MANZANA     0.30f
#define TIEMPO_CAIDA_PASO           0.10f

typedef enum { IDIOMA_ES = 0, IDIOMA_EN } Idioma;
typedef enum { MODO_ALEATORIO = 0, MODO_HISTORIA } ModoJuego;

typedef enum {
    pantalla_intro = 0,
    pantalla_menu,
    pantalla_config,
    pantalla_registro,
    pantalla_instrucciones,
    pantalla_juego,
    pantalla_pausa,
    pantalla_stats,
    pantalla_creditos,
    pantalla_controles,
    pantalla_salir,
    pantalla_gameover,
    pantalla_victoria_final
} estadopantalla;

typedef enum { TIERRA = 0, VACIO = 1 } TipoCelda;

typedef struct { int x, y; } Punto;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Color col;
    float vida;
    float vidaMax;
    bool activa;
} Particula;

typedef struct {
    Vector2 pos;
    int puntos;
    float vida;
    bool activo;
} TextoFlotante;

typedef struct {
    Music menu;
    Sound item;
    Sound dinero;
    Sound muerte;
    bool audioValido;
} sonidos_juego;

typedef struct {
    Texture2D mrdo;
    Texture2D enemigo;
    Texture2D manzana;
    Texture2D cereza;
    bool cargadas;
} texturas_juego;

typedef struct {
    int x, y;
    int dirX, dirY;
    int vidas;
    int score;
    char nombre[16];
    float timerMov;
} Jugador;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    bool activa;
    bool disponible;
    float timerRecarga;
    bool animandoConvergencia;
    float timerConvergencia;
} Pelota;

typedef struct {
    int x, y;
    bool cayendo;
    bool tambaleando;
    float timerCaida;
    bool destruida;
    bool rodando;
    int dirRodar;
    float timerRodar;
    bool vacioInicialAbajo;
} Manzana;

typedef struct {
    int x, y;
    bool recogida;
} Cereza;

typedef struct {
    int x, y;
    int dirX, dirY;
    bool vivo;
    bool enJuego;
    bool excavador;
    float timerPaso;
    int targetExploracionX;
    int targetExploracionY;
    float timerRespawn;
} Enemigo;

typedef struct {
    TipoCelda grid[FILAS][COLUMNAS];
    int visitadoPorEnemigos[FILAS][COLUMNAS];
    Jugador jugador;
    Pelota pelota;
    Particula particulas[MAX_PARTICULAS];
    TextoFlotante textos[MAX_TEXTOS_FLOTANTES];
    Manzana manzanas[MAX_MANZANAS];
    Cereza cerezas[MAX_CEREZAS];
    Enemigo enemigos[MAX_ENEMIGOS];
    int nivel;
    int totalCerezas;
    int numManzanas;
    int numEnemigos;
    float timerSpawnEnemigo;
} EstadoJuego;

typedef struct {
    Idioma idioma;
    ModoJuego modo;
} Configuracion;

// --- PROTOTIPOS ---
void IniciarNivel(EstadoJuego *j, int nivel, ModoJuego modo);
void ActualizarJuego(EstadoJuego *j, estadopantalla *pantalla, sonidos_juego *sonidos, Configuracion *cfg);
void DibujarJuego(EstadoJuego *j, texturas_juego *tex, Configuracion *cfg);
bool BFSCaminoLibre(EstadoJuego *j, int origenX, int origenY, int destX, int destY, int *pasoX, int *pasoY);
void GenerarNivelHistoria(EstadoJuego *j, int nivel);
void GenerarNivelHistoria01(EstadoJuego *j);
void GenerarNivelHistoria02(EstadoJuego *j);
void GenerarNivelHistoria03(EstadoJuego *j);
void GenerarNivelHistoria04(EstadoJuego *j);
void GenerarNivelHistoria05(EstadoJuego *j);
void GenerarNivelHistoria06(EstadoJuego *j);
void GenerarNivelHistoria07(EstadoJuego *j);
void GenerarNivelHistoria08(EstadoJuego *j);
void GenerarNivelHistoria09(EstadoJuego *j);
void GenerarNivelHistoria10(EstadoJuego *j);
void GenerarTunelesAleatorios(EstadoJuego *j, int plantillaId);
void GenerarElementosAleatoriosAleatorio(EstadoJuego *j);
bool EsZonaNido(int x, int y);
bool EsPosicionValidaManzana(EstadoJuego *j, int x, int y);
bool HayManzanaEn(EstadoJuego *j, int x, int y);
bool HayEnemigoEn(EstadoJuego *j, int x, int y, int idExcluir);
bool ManzanaTieneSoporte(EstadoJuego *j, int idManzana);
void CalcularObjetivoEnemigo(EstadoJuego *j, int idEnemigo, int *targetX, int *targetY, bool *modoCaza);
void GenerarParticulasExcavacion(EstadoJuego *j, int gx, int gy);
void GenerarExplosionPelota(EstadoJuego *j, Vector2 origen);
void GenerarConvergenciaPelota(EstadoJuego *j);
void AgregarTextoFlotante(EstadoJuego *j, int gx, int gy, int puntos);
void ReaparecerEnemigo(EstadoJuego *j, int i);
void GuardarRecord(int score, const char *nombre);
int CargarRecord(char *nombreOut);

bool EsZonaNido(int x, int y) {
    return (y == 4 && x == 7);
}

bool EsPosicionValidaManzana(EstadoJuego *j, int x, int y) {
    if (y + 1 >= FILAS) return false;
    return (j->grid[y + 1][x] == TIERRA);
}

bool HayManzanaEn(EstadoJuego *j, int x, int y) {
    for (int i = 0; i < j->numManzanas; i++) {
        if (!j->manzanas[i].destruida && j->manzanas[i].x == x && j->manzanas[i].y == y) {
            return true;
        }
    }
    return false;
}

bool HayEnemigoEn(EstadoJuego *j, int x, int y, int idExcluir) {
    for (int i = 0; i < j->numEnemigos; i++) {
        if (i != idExcluir && j->enemigos[i].vivo && j->enemigos[i].enJuego && j->enemigos[i].x == x && j->enemigos[i].y == y) {
            return true;
        }
    }
    return false;
}

bool ManzanaTieneSoporte(EstadoJuego *j, int idManzana) {
    if (j->manzanas[idManzana].cayendo) return false;

    int mx = j->manzanas[idManzana].x;
    int my = j->manzanas[idManzana].y;

    if (my + 1 >= FILAS) return true;

    if (j->grid[my + 1][mx] == TIERRA) {
        return true;
    }

    if (HayManzanaEn(j, mx, my + 1)) {
        return true;
    }

    if (!j->manzanas[idManzana].vacioInicialAbajo) {
        return false;
    }

    bool tierraIzquierda = (mx - 1 >= 0) && (j->grid[my][mx - 1] == TIERRA);
    bool tierraDerecha   = (mx + 1 < COLUMNAS) && (j->grid[my][mx + 1] == TIERRA);
    bool tierraArriba    = (my - 1 >= 0) && (j->grid[my - 1][mx] == TIERRA);

    if (tierraIzquierda || tierraDerecha || tierraArriba) {
        return true;
    }

    return false;
}

void GenerarTunelesAleatorios(EstadoJuego *j, int plantillaId) {
    j->grid[4][7] = VACIO;

    switch (plantillaId) {
        case 0:
            for (int c = 2; c <= 13; c++) j->grid[4][c] = VACIO;
            for (int f = 1; f <= 4; f++) { j->grid[f][4] = VACIO; j->grid[f][11] = VACIO; }
            for (int f = 4; f <= 8; f++) j->grid[f][8] = VACIO;
            break;
        case 1:
            for (int c = 3; c <= 12; c++) { j->grid[3][c] = VACIO; j->grid[6][c] = VACIO; }
            for (int f = 1; f <= 6; f++) { j->grid[f][3] = VACIO; j->grid[f][12] = VACIO; }
            for (int f = 6; f <= 8; f++) j->grid[f][8] = VACIO;
            break;
        default:
            for (int f = 1; f <= 7; f++) { j->grid[f][4] = VACIO; j->grid[f][11] = VACIO; }
            for (int c = 3; c <= 12; c++) j->grid[3][c] = VACIO;
            for (int c = 4; c <= 11; c++) j->grid[7][c] = VACIO;
            break;
    }
}

void GenerarElementosAleatoriosAleatorio(EstadoJuego *j) {
    Punto libresTierra[FILAS * COLUMNAS];
    int totalLibres = 0;

    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            if (EsZonaNido(c, f) || (c == 7 && f == 8)) continue;
            if (j->grid[f][c] == TIERRA) {
                libresTierra[totalLibres++] = (Punto){ c, f };
            }
        }
    }

    for (int i = totalLibres - 1; i > 0; i--) {
        int r = GetRandomValue(0, i);
        Punto temp = libresTierra[i];
        libresTierra[i] = libresTierra[r];
        libresTierra[r] = temp;
    }

    int idx = 0;

    j->numManzanas = GetRandomValue(4, 5);
    for (int m = 0; m < j->numManzanas && idx < totalLibres; m++) {
        j->manzanas[m] = (Manzana){ libresTierra[idx].x, libresTierra[idx].y, false, false, 0.0f, false, false, 0, 0.0f, false };
        idx++;
    }

    j->totalCerezas = 0;
    int maxCerezasDeseadas = GetRandomValue(24, 32);
    while (idx < totalLibres && j->totalCerezas < maxCerezasDeseadas && j->totalCerezas < MAX_CEREZAS) {
        j->cerezas[j->totalCerezas++] = (Cereza){ libresTierra[idx].x, libresTierra[idx].y, false };
        idx++;
    }

    for (int m = 0; m < j->numManzanas; m++) {
        int mx = j->manzanas[m].x;
        int my = j->manzanas[m].y;
        if (my + 1 < FILAS) {
            j->manzanas[m].vacioInicialAbajo = (j->grid[my + 1][mx] == VACIO);
        }
    }
}

int main(void) {
    const int anchopantalla = 800;
    const int altopantalla = 450;
    
    SetTraceLogLevel(LOG_NONE); // Deshabilitar logs de Raylib para limpiar la consola y mejorar el rendimiento
    InitWindow(anchopantalla, altopantalla, "Juego Dr. OH / Mr. Do");
    InitAudioDevice();
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    sonidos_juego sonidos = { 0 };
    sonidos.audioValido = IsAudioDeviceReady();
    if (sonidos.audioValido) {
        if (FileExists("sonido/sonidoMenu.mp3")) sonidos.menu = LoadMusicStream("sonido/sonidoMenu.mp3");
        if (FileExists("sonido/itemEncontrado.wav")) sonidos.item = LoadSound("sonido/itemEncontrado.wav");
        if (FileExists("sonido/sonidoDinero.wav")) sonidos.dinero = LoadSound("sonido/sonidoDinero.wav");
        if (FileExists("sonido/sonidoMuerte.wav")) sonidos.muerte = LoadSound("sonido/sonidoMuerte.wav");
    }

    texturas_juego tex = { 0 };
    if (FileExists("imagenes/mrdo.png") && FileExists("imagenes/enemigo.png") && 
        FileExists("imagenes/manzana.png") && FileExists("imagenes/cereza.png")) {
        tex.mrdo = LoadTexture("imagenes/mrdo.png");
        tex.enemigo = LoadTexture("imagenes/enemigo.png");
        tex.manzana = LoadTexture("imagenes/manzana.png");
        tex.cereza = LoadTexture("imagenes/cereza.png");
        tex.cargadas = (tex.mrdo.id > 0 && tex.enemigo.id > 0 && tex.manzana.id > 0 && tex.cereza.id > 0);
    }

    EstadoJuego juego = { 0 };
    Configuracion cfg = { IDIOMA_ES, MODO_HISTORIA };
    estadopantalla pantalla = pantalla_intro;
    float tiempo = 0.0f;
    int opcionMenu = 0;
    int opcionConfig = 0;
    char max_nombre[16] = "NADIE";
    int max_score = CargarRecord(max_nombre);
    bool debe_cerrar = false;
    int letterCount = 0;

    while (!WindowShouldClose() && !debe_cerrar) {
        if (sonidos.audioValido && IsMusicReady(sonidos.menu)) {
            if (pantalla == pantalla_menu || pantalla == pantalla_config || pantalla == pantalla_stats || pantalla == pantalla_creditos || pantalla == pantalla_controles) {
                if (!IsMusicStreamPlaying(sonidos.menu)) PlayMusicStream(sonidos.menu);
                UpdateMusicStream(sonidos.menu);
            } else {
                if (IsMusicStreamPlaying(sonidos.menu)) StopMusicStream(sonidos.menu);
            }
        }

        switch (pantalla) {
            case pantalla_intro:
                tiempo += GetFrameTime();
                if (tiempo >= 2.5f) pantalla = pantalla_menu;
                break;

            case pantalla_menu:
                if (IsKeyPressed(KEY_DOWN)) { 
                    opcionMenu = (opcionMenu + 1) % 6; 
                    if (IsSoundReady(sonidos.item)) PlaySound(sonidos.item); 
                }
                if (IsKeyPressed(KEY_UP)) { 
                    opcionMenu = (opcionMenu - 1 + 6) % 6; 
                    if (IsSoundReady(sonidos.item)) PlaySound(sonidos.item); 
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    if (IsSoundReady(sonidos.dinero)) PlaySound(sonidos.dinero);
                    if (opcionMenu == 0) { pantalla = pantalla_registro; letterCount = 0; juego.jugador.nombre[0] = '\0'; }
                    if (opcionMenu == 1) { pantalla = pantalla_config; opcionConfig = 0; }
                    if (opcionMenu == 2) pantalla = pantalla_stats;
                    if (opcionMenu == 3) pantalla = pantalla_controles;
                    if (opcionMenu == 4) pantalla = pantalla_creditos;
                    if (opcionMenu == 5) pantalla = pantalla_salir;
                }
                break;

            case pantalla_config:
                if (IsKeyPressed(KEY_DOWN)) { 
                    opcionConfig = (opcionConfig + 1) % 3; 
                    if (IsSoundReady(sonidos.item)) PlaySound(sonidos.item); 
                }
                if (IsKeyPressed(KEY_UP)) { 
                    opcionConfig = (opcionConfig - 1 + 3) % 3; 
                    if (IsSoundReady(sonidos.item)) PlaySound(sonidos.item); 
                }
                if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                    if (opcionConfig == 0) cfg.idioma = (cfg.idioma == IDIOMA_ES) ? IDIOMA_EN : IDIOMA_ES;
                    if (opcionConfig == 1) cfg.modo = (cfg.modo == MODO_ALEATORIO) ? MODO_HISTORIA : MODO_ALEATORIO;
                    if (IsSoundReady(sonidos.item)) PlaySound(sonidos.item); 
                }
                if (IsKeyPressed(KEY_ENTER) && opcionConfig == 2) {
                    if (IsSoundReady(sonidos.dinero)) PlaySound(sonidos.dinero);
                    pantalla = pantalla_menu;
                }
                break;

            case pantalla_registro: {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125) && (letterCount < 15)) {
                        juego.jugador.nombre[letterCount] = (char)key;
                        juego.jugador.nombre[letterCount + 1] = '\0';
                        letterCount++;
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    letterCount--;
                    if (letterCount < 0) letterCount = 0;
                    juego.jugador.nombre[letterCount] = '\0';
                }
                if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
                    pantalla = pantalla_instrucciones;
                }
                break;
            }

            case pantalla_instrucciones:
                if (IsKeyPressed(KEY_ENTER)) {
                    juego.jugador.vidas = 3;
                    juego.jugador.score = 0;
                    juego.jugador.timerMov = 0.0f;
                    IniciarNivel(&juego, 1, cfg.modo);
                    pantalla = pantalla_juego;
                }
                break;

            case pantalla_juego:
                if (IsKeyPressed(KEY_P)) {
                    pantalla = pantalla_pausa;
                } else {
                    estadopantalla pantalla_anterior = pantalla;
                    ActualizarJuego(&juego, &pantalla, &sonidos, &cfg);
                    if (juego.jugador.score > max_score) {
                        max_score = juego.jugador.score;
                        strncpy(max_nombre, juego.jugador.nombre, 15);
                        max_nombre[15] = '\0';
                    }
                    // Solo guardamos el record en disco al terminar la partida para evitar lag
                    if (pantalla != pantalla_anterior && (pantalla == pantalla_gameover || pantalla == pantalla_victoria_final)) {
                        GuardarRecord(max_score, max_nombre);
                    }
                }
                break;

            case pantalla_pausa:
                if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER)) {
                    pantalla = pantalla_juego;
                }
                break;

            case pantalla_stats:
            case pantalla_creditos:
            case pantalla_controles:
            case pantalla_gameover:
            case pantalla_victoria_final:
                if (IsKeyPressed(KEY_ENTER)) pantalla = pantalla_menu;
                break;

            case pantalla_salir:
                if (IsKeyPressed(KEY_S)) pantalla = pantalla_menu;
                if (IsKeyPressed(KEY_N)) debe_cerrar = true;
                break;
        }

        BeginDrawing();
            ClearBackground(BLACK);

            switch (pantalla) {
                case pantalla_intro:
                    DrawText("DR. OH / MR. DO", 260, 160, 36, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Cargando matriz..." : "Loading matrix...", 330, 230, 20, GRAY);
                    DrawRectangle(250, 270, (int)((tiempo / 2.5f) * 300.0f), 15, RED);
                    DrawRectangleLines(250, 270, 300, 15, WHITE);
                    break;

                case pantalla_menu: {
                    DrawText((cfg.idioma == IDIOMA_ES) ? "MENU PRINCIPAL" : "MAIN MENU", 280, 40, 32, GOLD);
                    const char *opts_es[] = { "1. Nuevo Juego", "2. Configuracion", "3. Estadisticas", "4. Controles", "5. Creditos", "6. Salir" };
                    const char *opts_en[] = { "1. New Game", "2. Settings", "3. High Scores", "4. Controls", "5. Credits", "6. Exit" };
                    for (int i = 0; i < 6; i++) {
                        Color col = (opcionMenu == i) ? RED : WHITE;
                        const char* txt = (cfg.idioma == IDIOMA_ES) ? opts_es[i] : opts_en[i];
                        if (opcionMenu == i) {
                            DrawText(TextFormat("-> %s", txt), 250, 110 + i * 40, 22, col);
                        } else {
                            DrawText(txt, 290, 110 + i * 40, 22, col);
                        }
                    }
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Usa Flechas ARRIBA/ABAJO para moverte y ENTER para seleccionar" : "Use UP/DOWN arrows to move, ENTER to select", 110, 400, 16, GRAY);
                    break;
                }

                case pantalla_config: {
                    DrawText((cfg.idioma == IDIOMA_ES) ? "CONFIGURACION" : "SETTINGS", 280, 40, 32, GOLD);
                    DrawRectangle(150, 95, 500, 240, Fade(DARKGRAY, 0.35f));
                    DrawRectangleLines(150, 95, 500, 240, PURPLE);

                    Color col0 = (opcionConfig == 0) ? RED : WHITE;
                    Color col1 = (opcionConfig == 1) ? RED : WHITE;
                    Color col2 = (opcionConfig == 2) ? RED : WHITE;

                    DrawText((cfg.idioma == IDIOMA_ES) ? "IDIOMA / LANGUAGE:" : "LANGUAGE / IDIOMA:", 180, 125, 20, col0);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "< ESPANOL >" : "< ENGLISH >", 450, 125, 20, YELLOW);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "MODO DE JUEGO:" : "GAME MODE:", 180, 185, 20, col1);
                    if (cfg.modo == MODO_ALEATORIO) {
                        DrawText((cfg.idioma == IDIOMA_ES) ? "< ALEATORIO (10 Niveles) >" : "< RANDOM (10 Levels) >", 380, 185, 18, YELLOW);
                    } else {
                        DrawText((cfg.idioma == IDIOMA_ES) ? "< HISTORIA (Niveles 1-10) >" : "< STORY (Levels 1-10) >", 380, 185, 18, GREEN);
                    }

                    DrawText((cfg.idioma == IDIOMA_ES) ? "[ GUARDAR Y VOLVER ]" : "[ SAVE & BACK ]", 290, 265, 20, col2);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Usa Flechas IZQ/DER para cambiar y ENTER para salir" : "Use LEFT/RIGHT arrows to switch, ENTER to save", 185, 360, 15, GRAY);
                    break;
                }

                case pantalla_registro:
                    DrawText((cfg.idioma == IDIOMA_ES) ? "INGRESA TU NOMBRE:" : "ENTER YOUR NAME:", 260, 140, 26, YELLOW);
                    DrawRectangle(250, 200, 300, 45, DARKGRAY);
                    DrawRectangleLines(250, 200, 300, 45, WHITE);
                    DrawText(juego.jugador.nombre, 270, 212, 24, RAYWHITE);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Presiona [ENTER] para continuar" : "Press [ENTER] to continue", 240, 280, 18, GRAY);
                    break;

                case pantalla_instrucciones:
                    DrawText((cfg.idioma == IDIOMA_ES) ? "GUIA DE CONTROLES" : "CONTROLS GUIDE", 285, 40, 26, GOLD);
                    DrawRectangle(150, 95, 500, 265, Fade(DARKGRAY, 0.4f));
                    DrawRectangleLines(150, 95, 500, 265, ORANGE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "MOVIMIENTO:" : "MOVEMENT:", 180, 120, 18, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Flechas [ARRIBA, ABAJO, IZQ, DER]" : "Arrows [UP, DOWN, LEFT, RIGHT]", 330, 120, 16, RAYWHITE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "DISPARO:" : "SHOOTING:", 180, 180, 18, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Barra Espaciadora [SPACE]" : "Spacebar [SPACE]", 330, 180, 16, RAYWHITE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "PAUSA:" : "PAUSE:", 180, 240, 18, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Tecla [P]" : "Key [P]", 330, 240, 16, RAYWHITE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "PRESIONA [ENTER] PARA COMENZAR LA PARTIDA" : "PRESS [ENTER] TO START GAME", 200, 385, 16, GREEN);
                    break;

                case pantalla_controles:
                    DrawText((cfg.idioma == IDIOMA_ES) ? "GUIA DE CONTROLES" : "CONTROLS GUIDE", 285, 40, 26, GOLD);
                    DrawRectangle(150, 95, 500, 265, Fade(DARKGRAY, 0.4f));
                    DrawRectangleLines(150, 95, 500, 265, ORANGE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "MOVIMIENTO:" : "MOVEMENT:", 180, 120, 18, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Flechas [ARRIBA, ABAJO, IZQ, DER]" : "Arrows [UP, DOWN, LEFT, RIGHT]", 330, 120, 16, RAYWHITE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "DISPARO:" : "SHOOTING:", 180, 180, 18, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Barra Espaciadora [SPACE]" : "Spacebar [SPACE]", 330, 180, 16, RAYWHITE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "PAUSA:" : "PAUSE:", 180, 240, 18, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Tecla [P]" : "Key [P]", 330, 240, 16, RAYWHITE);

                    DrawText((cfg.idioma == IDIOMA_ES) ? "Enter: Volver al menu" : "Enter: Back to menu", 300, 320, 16, GRAY);
                    break;

                case pantalla_juego:
                    DibujarJuego(&juego, &tex, &cfg);
                    break;

                case pantalla_pausa:
                    DibujarJuego(&juego, &tex, &cfg);
                    DrawRectangle(0, 0, anchopantalla, altopantalla, Fade(BLACK, 0.65f));
                    DrawText((cfg.idioma == IDIOMA_ES) ? "PAUSA" : "PAUSE", 350, 180, 32, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Presiona [P] o [ENTER] para continuar" : "Press [P] or [ENTER] to continue", 255, 230, 16, RAYWHITE);
                    break;

                case pantalla_stats:
                    DrawText((cfg.idioma == IDIOMA_ES) ? "ESTADISTICAS (HIGH SCORE)" : "STATISTICS (HIGH SCORE)", 200, 100, 26, PURPLE);
                    DrawText(TextFormat((cfg.idioma == IDIOMA_ES) ? "CAMPEON: %s" : "CHAMPION: %s", max_nombre), 240, 170, 20, GOLD);
                    DrawText(TextFormat((cfg.idioma == IDIOMA_ES) ? "RECORD ACTUAL: %05d PTS" : "CURRENT RECORD: %05d PTS", max_score), 240, 210, 20, WHITE);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Enter: Volver al menu" : "Enter: Back to menu", 280, 300, 16, GRAY);
                    break;

                case pantalla_creditos:
                    DrawText((cfg.idioma == IDIOMA_ES) ? "CREDITOS" : "CREDITS", 340, 100, 28, SKYBLUE);
                    DrawText("Inspirado en el clasico Mr. Do! (1982)", 235, 180, 18, RAYWHITE);
                    DrawText("Desarrollado con Raylib & C", 270, 220, 18, RAYWHITE);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Enter: Volver al menu" : "Enter: Back to menu", 300, 320, 16, GRAY);
                    break;

                case pantalla_gameover:
                    DrawText("GAME OVER", 295, 150, 38, RED);
                    DrawText(TextFormat((cfg.idioma == IDIOMA_ES) ? "Puntaje Final: %d" : "Final Score: %d", juego.jugador.score), 310, 220, 20, WHITE);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Presiona [ENTER] para volver al menu" : "Press [ENTER] to return to menu", 240, 290, 16, LIGHTGRAY);
                    break;

                case pantalla_victoria_final:
                    DrawText((cfg.idioma == IDIOMA_ES) ? "¡FELICITACIONES!" : "CONGRATULATIONS!", 230, 130, 36, GOLD);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "¡Has completado todos los 10 niveles de la Historia!" : "You have completed all 10 Story levels!", 150, 190, 18, RAYWHITE);
                    DrawText(TextFormat((cfg.idioma == IDIOMA_ES) ? "Puntaje Total: %05d PTS" : "Total Score: %05d PTS", juego.jugador.score), 270, 240, 22, YELLOW);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "Presiona [ENTER] para volver al menu" : "Press [ENTER] to return to menu", 240, 310, 16, LIGHTGRAY);
                    break;

                case pantalla_salir:
                    DrawText((cfg.idioma == IDIOMA_ES) ? "¿DESEAS SALIR?" : "EXIT GAME?", 320, 160, 24, RED);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "[S] Cancelar y volver al menu" : "[S] Cancel and back to menu", 280, 220, 18, WHITE);
                    DrawText((cfg.idioma == IDIOMA_ES) ? "[N] Confirmar salida" : "[N] Confirm exit", 280, 260, 18, WHITE);
                    break;
            }
        EndDrawing();
    }

    if (tex.cargadas) {
        UnloadTexture(tex.mrdo);
        UnloadTexture(tex.enemigo);
        UnloadTexture(tex.manzana);
        UnloadTexture(tex.cereza);
    }
    if (sonidos.audioValido) {
        if (IsMusicReady(sonidos.menu)) UnloadMusicStream(sonidos.menu);
        if (IsSoundReady(sonidos.item)) UnloadSound(sonidos.item);
        if (IsSoundReady(sonidos.dinero)) UnloadSound(sonidos.dinero);
        if (IsSoundReady(sonidos.muerte)) UnloadSound(sonidos.muerte);
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void AgregarTextoFlotante(EstadoJuego *j, int gx, int gy, int puntos) {
    for (int i = 0; i < MAX_TEXTOS_FLOTANTES; i++) {
        if (!j->textos[i].activo) {
            j->textos[i].pos = (Vector2){ (float)(OFFSET_X + gx * TAM_CELDA + 6), (float)(OFFSET_Y + gy * TAM_CELDA + 10) };
            j->textos[i].puntos = puntos;
            j->textos[i].vida = 1.2f;
            j->textos[i].activo = true;
            break;
        }
    }
}

void GenerarParticulasExcavacion(EstadoJuego *j, int gx, int gy) {
    Vector2 centro = {
        OFFSET_X + gx * TAM_CELDA + TAM_CELDA / 2.0f,
        OFFSET_Y + gy * TAM_CELDA + TAM_CELDA / 2.0f
    };
    Color coloresTierra[3] = { DARKBROWN, BROWN, (Color){ 139, 69, 19, 255 } };

    int creadas = 0;
    for (int i = 0; i < MAX_PARTICULAS && creadas < 8; i++) {
        if (!j->particulas[i].activa) {
            float angulo = (float)GetRandomValue(0, 360) * DEG2RAD;
            float vel = (float)GetRandomValue(40, 110);
            j->particulas[i].pos = centro;
            j->particulas[i].vel = (Vector2){ cosf(angulo) * vel, sinf(angulo) * vel };
            j->particulas[i].col = coloresTierra[GetRandomValue(0, 2)];
            j->particulas[i].vidaMax = (float)GetRandomValue(25, 45) / 100.0f;
            j->particulas[i].vida = j->particulas[i].vidaMax;
            j->particulas[i].activa = true;
            creadas++;
        }
    }
}

bool BFSCaminoLibre(EstadoJuego *j, int origenX, int origenY, int destX, int destY, int *pasoX, int *pasoY) {
    if (origenX == destX && origenY == destY) return false;

    bool visitado[FILAS][COLUMNAS] = { 0 };
    Punto padre[FILAS][COLUMNAS];
    Punto cola[FILAS * COLUMNAS];
    int frente = 0, final = 0;

    cola[final++] = (Punto){ origenX, origenY };
    visitado[origenY][origenX] = true;
    padre[origenY][origenX] = (Punto){ -1, -1 };

    int dx[4] = { 1, -1, 0, 0 };
    int dy[4] = { 0, 0, 1, -1 };
    bool encontrado = false;

    while (frente < final) {
        Punto actual = cola[frente++];
        if (actual.x == destX && actual.y == destY) {
            encontrado = true;
            break;
        }

        for (int d = 0; d < 4; d++) {
            int nx = actual.x + dx[d];
            int ny = actual.y + dy[d];
            if (nx >= 0 && nx < COLUMNAS && ny >= 0 && ny < FILAS &&
                !visitado[ny][nx] && j->grid[ny][nx] == VACIO) {
                if (HayManzanaEn(j, nx, ny) && !(nx == destX && ny == destY)) continue;
                
                visitado[ny][nx] = true;
                padre[ny][nx] = actual;
                cola[final++] = (Punto){ nx, ny };
            }
        }
    }

    if (!encontrado) return false;

    Punto nodo = { destX, destY };
    while (true) {
        Punto p = padre[nodo.y][nodo.x];
        if (p.x == origenX && p.y == origenY) {
            if (pasoX) *pasoX = nodo.x;
            if (pasoY) *pasoY = nodo.y;
            return true;
        }
        nodo = p;
    }
}

void CalcularObjetivoEnemigo(EstadoJuego *j, int idEnemigo, int *targetX, int *targetY, bool *modoCaza) {
    int ex = j->enemigos[idEnemigo].x;
    int ey = j->enemigos[idEnemigo].y;
    int px = j->jugador.x;
    int py = j->jugador.y;

    int distanciaAlJugador = abs(ex - px) + abs(ey - py);

    if (distanciaAlJugador <= DISTANCIA_DETECCION) {
        *targetX = px;
        *targetY = py;
        *modoCaza = true;
        return;
    }

    *modoCaza = false;
    int tx = j->enemigos[idEnemigo].targetExploracionX;
    int ty = j->enemigos[idEnemigo].targetExploracionY;

    if (tx < 0 || ty < 0 || (ex == tx && ey == ty)) {
        int minVisitas = 999999;
        Punto mejoresPuntos[FILAS * COLUMNAS];
        int numMejores = 0;

        for (int f = 0; f < FILAS; f++) {
            for (int c = 0; c < COLUMNAS; c++) {
                if (EsZonaNido(c, f)) continue;
                int visitas = j->visitadoPorEnemigos[f][c];
                if (visitas < minVisitas) {
                    minVisitas = visitas;
                    numMejores = 0;
                    mejoresPuntos[numMejores++] = (Punto){ c, f };
                } else if (visitas == minVisitas && numMejores < FILAS * COLUMNAS) {
                    mejoresPuntos[numMejores++] = (Punto){ c, f };
                }
            }
        }

        if (numMejores > 0) {
            int sel = GetRandomValue(0, numMejores - 1);
            j->enemigos[idEnemigo].targetExploracionX = mejoresPuntos[sel].x;
            j->enemigos[idEnemigo].targetExploracionY = mejoresPuntos[sel].y;
            *targetX = mejoresPuntos[sel].x;
            *targetY = mejoresPuntos[sel].y;
            return;
        }
    }

    *targetX = tx;
    *targetY = ty;
}

// --- GENERACION DE NIVELES HISTORIA (1 AL 10) ---
void GenerarNivelHistoria01(EstadoJuego *j) {
    for (int f = 0; f <= 8; f++) j->grid[f][7] = VACIO;
    for (int c = 6; c <= 12; c++) j->grid[0][c] = VACIO;
    j->grid[1][12] = VACIO; j->grid[1][13] = VACIO;
    j->grid[2][13] = VACIO; j->grid[2][14] = VACIO;
    j->grid[3][14] = VACIO; j->grid[4][14] = VACIO; j->grid[5][14] = VACIO;
    j->grid[6][14] = VACIO; j->grid[6][13] = VACIO;
    j->grid[7][13] = VACIO; j->grid[7][12] = VACIO;
    j->grid[8][12] = VACIO;
    for (int c = 1; c <= 12; c++) j->grid[8][c] = VACIO;
    for (int f = 6; f <= 8; f++) j->grid[f][1] = VACIO;
    for (int c = 1; c <= 3; c++) j->grid[6][c] = VACIO;
    for (int f = 6; f <= 7; f++) j->grid[f][3] = VACIO;

    j->numManzanas = 4;
    j->manzanas[0] = (Manzana){ 4, 0, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 6, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 12, 3, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 10, 4, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 0; f <= 3; f++) {
        for (int c = 1; c <= 4; c++) {
            if (f == 0 && c == 4) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 2; f <= 3; f++) {
        for (int c = 8; c <= 11; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 5; f <= 6; f++) {
        for (int c = 1; c <= 4; c++) {
            if (f == 6 && c <= 3) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 5; f <= 7; f++) {
        for (int c = 10; c <= 12; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
}

void GenerarNivelHistoria02(EstadoJuego *j) {
    for (int c = 3; c <= 12; c++) j->grid[0][c] = VACIO;
    j->grid[1][3] = VACIO; j->grid[1][2] = VACIO;
    j->grid[2][2] = VACIO; j->grid[2][1] = VACIO;
    j->grid[3][1] = VACIO;
    j->grid[1][12] = VACIO; j->grid[1][13] = VACIO;
    j->grid[2][13] = VACIO; j->grid[2][14] = VACIO;
    j->grid[3][14] = VACIO; j->grid[4][14] = VACIO;
    j->grid[5][14] = VACIO; j->grid[5][13] = VACIO;
    j->grid[6][13] = VACIO;
    j->grid[6][1] = VACIO; j->grid[6][2] = VACIO;
    j->grid[6][3] = VACIO; j->grid[6][4] = VACIO;
    j->grid[5][4] = VACIO; j->grid[5][5] = VACIO;
    j->grid[5][6] = VACIO; j->grid[5][7] = VACIO;
    j->grid[4][7] = VACIO;
    for (int c = 7; c <= 13; c++) j->grid[5][c] = VACIO;
    for (int c = 1; c <= 14; c++) j->grid[8][c] = VACIO;
    for (int f = 6; f <= 8; f++) j->grid[f][1] = VACIO;

    j->numManzanas = 5;
    j->manzanas[0] = (Manzana){ 5, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 5, 3, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 12, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 3, 5, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[4] = (Manzana){ 12, 6, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 2; f <= 3; f++) {
        for (int c = 3; c <= 6; c++) {
            if (f == 3 && c == 5) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 4; f <= 7; f++) {
        for (int c = 1; c <= 2; c++) {
            if (f == 6 && c == 1) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 2; f <= 5; f++) {
        for (int c = 10; c <= 11; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 6; f <= 7; f++) {
        for (int c = 5; c <= 8; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 6; f <= 7; f++) {
        for (int c = 11; c <= 14; c++) {
            if (f == 6 && c == 12) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
}

void GenerarNivelHistoria03(EstadoJuego *j) {
    for (int c = 2; c <= 13; c++) j->grid[0][c] = VACIO;
    j->grid[1][2] = VACIO; j->grid[1][1] = VACIO;
    j->grid[2][1] = VACIO;
    j->grid[6][1] = VACIO; j->grid[7][1] = VACIO; j->grid[7][2] = VACIO;
    for (int f = 1; f <= 7; f++) j->grid[f][13] = VACIO;
    j->grid[7][14] = VACIO; j->grid[8][14] = VACIO;
    for (int c = 6; c <= 12; c++) j->grid[5][c] = VACIO;
    j->grid[4][7] = VACIO;
    for (int c = 2; c <= 14; c++) j->grid[8][c] = VACIO;
    j->grid[8][1] = VACIO;

    j->numManzanas = 5;
    j->manzanas[0] = (Manzana){ 6, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 3, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 12, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 3, 4, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[4] = (Manzana){ 9, 5, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 2; f <= 3; f++) {
        for (int c = 5; c <= 8; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 2; f <= 5; f++) {
        for (int c = 1; c <= 2; c++) {
            if (f == 2 && c == 1) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 4; f <= 7; f++) {
        for (int c = 4; c <= 5; c++) {
            if (f == 4 && c == 4) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 2; f <= 5; f++) {
        for (int c = 11; c <= 12; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 6; f <= 7; f++) {
        for (int c = 9; c <= 12; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
}

void GenerarNivelHistoria04(EstadoJuego *j) {
    for (int c = 6; c <= 11; c++) j->grid[0][c] = VACIO;
    for (int f = 0; f <= 7; f++) j->grid[f][11] = VACIO;
    for (int c = 2; c <= 11; c++) j->grid[5][c] = VACIO;
    j->grid[4][2] = VACIO; j->grid[4][3] = VACIO;
    j->grid[3][3] = VACIO; j->grid[3][4] = VACIO;
    j->grid[2][4] = VACIO; j->grid[2][5] = VACIO;
    j->grid[1][5] = VACIO; j->grid[1][6] = VACIO;
    for (int c = 4; c <= 7; c++) j->grid[4][c] = VACIO;
    j->grid[4][7] = VACIO;
    for (int c = 1; c <= 11; c++) j->grid[8][c] = VACIO;

    j->numManzanas = 5;
    j->manzanas[0] = (Manzana){ 5, 0, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 2, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 5, 3, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 10, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[4] = (Manzana){ 12, 4, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 0; f <= 1; f++) {
        for (int c = 1; c <= 4; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 3; f <= 5; f++) {
        for (int c = 1; c <= 2; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 1; f <= 4; f++) {
        for (int c = 12; c <= 13; c++) {
            if (f == 4 && c == 12) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 2; f <= 5; f++) {
        for (int c = 9; c <= 10; c++) {
            if (f == 2 && c == 10) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 6; f <= 7; f++) {
        for (int c = 3; c <= 6; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
}

void GenerarNivelHistoria05(EstadoJuego *j) {
    for (int c = 1; c <= 14; c++) j->grid[0][c] = VACIO;
    for (int f = 0; f <= 4; f++) j->grid[f][1] = VACIO;
    for (int c = 1; c <= 13; c++) j->grid[4][c] = VACIO;
    for (int f = 4; f <= 5; f++) j->grid[f][6] = VACIO;
    for (int c = 6; c <= 8; c++) j->grid[5][c] = VACIO;
    for (int f = 4; f <= 5; f++) j->grid[f][8] = VACIO;
    j->grid[4][7] = VACIO;
    for (int f = 4; f <= 7; f++) j->grid[f][13] = VACIO;
    for (int c = 1; c <= 13; c++) j->grid[7][c] = VACIO;
    for (int f = 7; f <= 8; f++) j->grid[f][1] = VACIO;
    for (int c = 1; c <= 14; c++) j->grid[8][c] = VACIO;

    j->numManzanas = 5;
    j->manzanas[0] = (Manzana){ 2, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 8, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 5, 5, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 9, 5, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[4] = (Manzana){ 10, 6, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 2; f <= 3; f++) {
        for (int c = 4; c <= 7; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 1; f <= 2; f++) {
        for (int c = 10; c <= 13; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 5; f <= 7; f++) {
        for (int c = 2; c <= 3; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 5; f <= 6; f++) {
        for (int c = 6; c <= 9; c++) {
            if (f == 5 && (c == 6 || c == 8)) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 5; f <= 7; f++) {
        for (int c = 11; c <= 12; c++) {
            if (f == 6 && c == 11) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
}

void GenerarNivelHistoria06(EstadoJuego *j) {
    for (int c = 3; c <= 14; c++) j->grid[0][c] = VACIO;
    j->grid[1][3] = VACIO; j->grid[1][2] = VACIO;
    j->grid[2][2] = VACIO; j->grid[2][1] = VACIO;
    for (int f = 2; f <= 7; f++) j->grid[f][1] = VACIO;
    for (int c = 7; c <= 13; c++) j->grid[4][c] = VACIO;
    j->grid[4][7] = VACIO;
    for (int f = 4; f <= 7; f++) j->grid[f][14] = VACIO;
    for (int c = 2; c <= 14; c++) j->grid[7][c] = VACIO;
    for (int c = 2; c <= 14; c++) j->grid[8][c] = VACIO;

    j->numManzanas = 5;
    j->manzanas[0] = (Manzana){ 5, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 10, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 12, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 7, 6, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[4] = (Manzana){ 11, 5, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 2; f <= 5; f++) {
        for (int c = 3; c <= 4; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 2; f <= 3; f++) {
        for (int c = 7; c <= 10; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 2; f <= 5; f++) {
        for (int c = 12; c <= 13; c++) {
            if (f == 2 && c == 12) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 5; f <= 6; f++) {
        for (int c = 2; c <= 5; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 5; f <= 6; f++) {
        for (int c = 9; c <= 12; c++) {
            if (f == 5 && c == 11) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
}

void GenerarNivelHistoria07(EstadoJuego *j) {
    for (int c = 1; c <= 13; c++) j->grid[0][c] = VACIO;
    j->grid[1][1] = VACIO;
    j->grid[1][13] = VACIO; j->grid[1][12] = VACIO;
    j->grid[2][12] = VACIO; j->grid[2][11] = VACIO;
    j->grid[3][11] = VACIO; j->grid[3][10] = VACIO;
    j->grid[3][9] = VACIO;  j->grid[3][8] = VACIO;
    j->grid[4][8] = VACIO;  j->grid[4][7] = VACIO;
    for (int f = 4; f <= 8; f++) j->grid[f][7] = VACIO;

    j->numManzanas = 5;
    j->manzanas[0] = (Manzana){ 4, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 9, 1, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 13, 3, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 11, 4, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[4] = (Manzana){ 2, 5, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 2; f <= 5; f++) {
        for (int c = 2; c <= 3; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 2; f <= 3; f++) {
        for (int c = 6; c <= 9; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 3; f <= 6; f++) {
        for (int c = 12; c <= 13; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 5; f <= 7; f++) {
        for (int c = 9; c <= 10; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
    for (int f = 6; f <= 7; f++) {
        for (int c = 1; c <= 4; c++) j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
    }
}

void GenerarNivelHistoria08(EstadoJuego *j) {
    for (int c = 4; c <= 11; c++) {
        j->grid[0][c] = VACIO;
        j->grid[4][c] = VACIO;
        j->grid[8][c] = VACIO;
    }
    for (int f = 0; f <= 4; f++) {
        j->grid[f][3] = VACIO;
        j->grid[f][12] = VACIO;
    }
    for (int f = 4; f <= 8; f++) {
        j->grid[f][3] = VACIO;
        j->grid[f][12] = VACIO;
    }

    j->numManzanas = 4;
    j->manzanas[0] = (Manzana){ 5, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 10, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 5, 6, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 10, 6, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 1; f <= 3; f++) {
        for (int c = 4; c <= 11; c++) {
            if (f == 2 && (c == 5 || c == 10)) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 5; f <= 7; f++) {
        for (int c = 4; c <= 11; c++) {
            if (f == 6 && (c == 5 || c == 10)) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
}

void GenerarNivelHistoria09(EstadoJuego *j) {
    for (int c = 4; c <= 11; c++) {
        j->grid[0][c] = VACIO;
        j->grid[4][c] = VACIO;
    }
    for (int f = 0; f <= 4; f++) {
        j->grid[f][3] = VACIO;
    }
    for (int f = 0; f <= 8; f++) {
        j->grid[f][12] = VACIO;
    }
    for (int c = 4; c <= 12; c++) {
        j->grid[8][c] = VACIO;
    }

    j->numManzanas = 4;
    j->manzanas[0] = (Manzana){ 6, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[1] = (Manzana){ 10, 2, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 6, 6, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 10, 6, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 1; f <= 3; f++) {
        for (int c = 4; c <= 11; c++) {
            if (f == 2 && (c == 6 || c == 10)) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
    for (int f = 5; f <= 7; f++) {
        for (int c = 4; c <= 11; c++) {
            if (f == 6 && (c == 6 || c == 10)) continue;
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
}

void GenerarNivelHistoria10(EstadoJuego *j) {
    for (int c = 4; c <= 11; c++) {
        j->grid[0][c] = VACIO;
        j->grid[8][c] = VACIO;
    }
    for (int f = 0; f <= 8; f++) {
        j->grid[f][3] = VACIO;
        j->grid[f][12] = VACIO;
    }

    j->numManzanas = 4;
    j->manzanas[0] = (Manzana){ 7, 1, false, false, 0.0f, false, false, 0, 0.0f, true };
    j->manzanas[1] = (Manzana){ 7, 7, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[2] = (Manzana){ 5, 4, false, false, 0.0f, false, false, 0, 0.0f, false };
    j->manzanas[3] = (Manzana){ 10, 4, false, false, 0.0f, false, false, 0, 0.0f, false };

    j->totalCerezas = 0;
    for (int f = 1; f <= 7; f++) {
        for (int c = 4; c <= 6; c++) {
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
        for (int c = 9; c <= 11; c++) {
            j->cerezas[j->totalCerezas++] = (Cereza){ c, f, false };
        }
    }
}

void GenerarNivelHistoria(EstadoJuego *j, int nivel) {
    switch (nivel) {
        case 1: GenerarNivelHistoria01(j); break;
        case 2: GenerarNivelHistoria02(j); break;
        case 3: GenerarNivelHistoria03(j); break;
        case 4: GenerarNivelHistoria04(j); break;
        case 5: GenerarNivelHistoria05(j); break;
        case 6: GenerarNivelHistoria06(j); break;
        case 7: GenerarNivelHistoria07(j); break;
        case 8: GenerarNivelHistoria08(j); break;
        case 9: GenerarNivelHistoria09(j); break;
        case 10: GenerarNivelHistoria10(j); break;
        default: GenerarNivelHistoria10(j); break;
    }

    for (int m = 0; m < j->numManzanas; m++) {
        int mx = j->manzanas[m].x;
        int my = j->manzanas[m].y;
        if (my + 1 < FILAS) {
            j->manzanas[m].vacioInicialAbajo = (j->grid[my + 1][mx] == VACIO);
        }
    }
}

void GenerarExplosionPelota(EstadoJuego *j, Vector2 origen) {
    Color paleta[4] = { YELLOW, ORANGE, GOLD, RED };
    for (int i = 0; i < MAX_PARTICULAS; i++) {
        float angulo = (float)GetRandomValue(0, 360) * DEG2RAD;
        float velocidad = (float)GetRandomValue(80, 220);
        j->particulas[i].pos = origen;
        j->particulas[i].vel = (Vector2){ cosf(angulo) * velocidad, sinf(angulo) * velocidad };
        j->particulas[i].col = paleta[GetRandomValue(0, 3)];
        j->particulas[i].vidaMax = (float)GetRandomValue(40, 80) / 100.0f;
        j->particulas[i].vida = j->particulas[i].vidaMax;
        j->particulas[i].activa = true;
    }
}

void GenerarConvergenciaPelota(EstadoJuego *j) {
    Vector2 destJugador = {
        OFFSET_X + j->jugador.x * TAM_CELDA + TAM_CELDA / 2.0f,
        OFFSET_Y + j->jugador.y * TAM_CELDA + TAM_CELDA / 2.0f
    };

    Color paleta[3] = { GOLD, YELLOW, RAYWHITE };
    for (int i = 0; i < MAX_PARTICULAS; i++) {
        float angulo = (float)GetRandomValue(0, 360) * DEG2RAD;
        float distancia = (float)GetRandomValue(120, 200);
        Vector2 spawnPos = {
            destJugador.x + cosf(angulo) * distancia,
            destJugador.y + sinf(angulo) * distancia
        };
        j->particulas[i].pos = spawnPos;
        Vector2 dir = { destJugador.x - spawnPos.x, destJugador.y - spawnPos.y };
        j->particulas[i].vel = (Vector2){ dir.x * 1.5f, dir.y * 1.5f };
        j->particulas[i].col = paleta[GetRandomValue(0, 2)];
        j->particulas[i].vidaMax = 0.65f;
        j->particulas[i].vida = j->particulas[i].vidaMax;
        j->particulas[i].activa = true;
    }
}

void ReaparecerEnemigo(EstadoJuego *j, int i) {
    j->enemigos[i].x = 7;
    j->enemigos[i].y = 4;
    j->enemigos[i].dirX = 0;
    j->enemigos[i].dirY = 1;
    j->enemigos[i].vivo = true;
    j->enemigos[i].enJuego = true;
    j->enemigos[i].timerPaso = 0.0f;
    j->enemigos[i].targetExploracionX = -1;
    j->enemigos[i].targetExploracionY = -1;
    j->enemigos[i].timerRespawn = 0.0f;

    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            j->visitadoPorEnemigos[f][c] = 0;
        }
    }
}

void IniciarNivel(EstadoJuego *j, int nivel, ModoJuego modo) {
    j->nivel = nivel;

    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            j->grid[f][c] = TIERRA;
            j->visitadoPorEnemigos[f][c] = 0;
        }
    }

    if (modo == MODO_HISTORIA) {
        GenerarNivelHistoria(j, nivel);
    } else {
        int plantilla = GetRandomValue(0, 2);
        GenerarTunelesAleatorios(j, plantilla);
        GenerarElementosAleatoriosAleatorio(j);
    }

    j->jugador.x = 7;
    j->jugador.y = 8;
    j->jugador.dirX = 0;
    j->jugador.dirY = -1;
    j->jugador.timerMov = 0.0f;
    j->grid[8][7] = VACIO;

    j->pelota.activa = false;
    j->pelota.disponible = true;
    j->pelota.timerRecarga = 0.0f;
    j->pelota.animandoConvergencia = false;
    j->pelota.timerConvergencia = 0.0f;

    for (int i = 0; i < MAX_PARTICULAS; i++) j->particulas[i].activa = false;
    for (int i = 0; i < MAX_TEXTOS_FLOTANTES; i++) j->textos[i].activo = false;

    j->numEnemigos = MAX_ENEMIGOS;
    j->timerSpawnEnemigo = 0.0f;

    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        j->enemigos[i].x = 7;
        j->enemigos[i].y = 4;
        j->enemigos[i].dirX = 0;
        j->enemigos[i].dirY = 1;
        j->enemigos[i].vivo = true;
        j->enemigos[i].excavador = (i % 2 == 0);
        j->enemigos[i].timerPaso = 0.0f;
        j->enemigos[i].targetExploracionX = -1;
        j->enemigos[i].targetExploracionY = -1;

        if (i == 0) {
            j->enemigos[i].enJuego = true;
            j->enemigos[i].timerRespawn = 0.0f;
        } else if (i == 1) {
            j->enemigos[i].enJuego = false;
            j->enemigos[i].timerRespawn = 5.0f;
        } else {
            j->enemigos[i].enJuego = false;
            j->enemigos[i].timerRespawn = 9999.0f;
        }
    }
}

// --- ACTUALIZACION Y FISICAS ---
void ActualizarJuego(EstadoJuego *j, estadopantalla *pantalla, sonidos_juego *sonidos, Configuracion *cfg) {
    float dt = GetFrameTime();
    bool eventoMuerte = false;

    int recogidas = 0;
    for (int c = 0; c < j->totalCerezas; c++) {
        if (j->cerezas[c].recogida) recogidas++;
    }
    float porcentaje = (j->totalCerezas > 0) ? ((float)recogidas / (float)j->totalCerezas) : 0.0f;

    bool puedeExcavar = (porcentaje < 0.50f);

    if (!j->enemigos[1].enJuego && j->enemigos[1].vivo) {
        j->enemigos[1].timerRespawn -= dt;
        if (j->enemigos[1].timerRespawn <= 0.0f) {
            j->enemigos[1].enJuego = true;
            j->enemigos[1].timerRespawn = 0.0f;
        }
    }
    if (porcentaje >= 0.35f && !j->enemigos[2].enJuego && j->enemigos[2].vivo) {
        j->enemigos[2].enJuego = true;
        j->enemigos[2].timerRespawn = 0.0f;
    }
    if (porcentaje >= 0.60f && !j->enemigos[3].enJuego && j->enemigos[3].vivo) {
        j->enemigos[3].enJuego = true;
        j->enemigos[3].timerRespawn = 0.0f;
    }
    if (porcentaje >= 0.80f && !j->enemigos[4].enJuego && j->enemigos[4].vivo) {
        j->enemigos[4].enJuego = true;
        j->enemigos[4].timerRespawn = 0.0f;
    }

    for (int i = 0; i < j->numEnemigos; i++) {
        if (!j->enemigos[i].vivo) {
            j->enemigos[i].timerRespawn -= dt;
            if (j->enemigos[i].timerRespawn <= 0.0f) {
                ReaparecerEnemigo(j, i);
            }
        }
    }

    // 1. Movimiento del Jugador (Restricción: No puede mover manzanas horizontalmente)
    j->jugador.timerMov += dt;
    int nx = j->jugador.x;
    int ny = j->jugador.y;
    bool intentoMovimiento = false;

    if (IsKeyDown(KEY_UP))         { ny--; j->jugador.dirX =  0; j->jugador.dirY = -1; intentoMovimiento = true; }
    else if (IsKeyDown(KEY_DOWN))  { ny++; j->jugador.dirX =  0; j->jugador.dirY =  1; intentoMovimiento = true; }
    else if (IsKeyDown(KEY_LEFT))  { nx--; j->jugador.dirX = -1; j->jugador.dirY =  0; intentoMovimiento = true; }
    else if (IsKeyDown(KEY_RIGHT)) { nx++; j->jugador.dirX =  1; j->jugador.dirY =  0; intentoMovimiento = true; }

    if (intentoMovimiento) {
        if (EsZonaNido(nx, ny)) {
            nx = j->jugador.x;
            ny = j->jugador.y;
        }

        bool destinoEsTierra = (nx >= 0 && nx < COLUMNAS && ny >= 0 && ny < FILAS && j->grid[ny][nx] == TIERRA);
        float tiempoRequerido = destinoEsTierra ? TIEMPO_PASO_JUGADOR_TIERRA : TIEMPO_PASO_JUGADOR_VACIO;

        if (j->jugador.timerMov >= tiempoRequerido) {
            if (nx >= 0 && nx < COLUMNAS && ny >= 0 && ny < FILAS) {
                // Verificamos si hay una manzana en la casilla de destino
                bool hayManzanaDestino = HayManzanaEn(j, nx, ny);

                if (!hayManzanaDestino) {
                    if (destinoEsTierra) {
                        GenerarParticulasExcavacion(j, nx, ny);
                    }
                    j->jugador.x = nx;
                    j->jugador.y = ny;
                    j->grid[ny][nx] = VACIO;
                    j->jugador.timerMov = 0.0f;
                }
                // Si hay una manzana, el jugador no puede avanzar ni moverla horizontalmente.
            }
        }
    }

    // 2. Disparo de Pelota
    if (!j->pelota.disponible && !j->pelota.activa) {
        j->pelota.timerRecarga -= dt;

        if (j->pelota.timerRecarga <= 0.65f && !j->pelota.animandoConvergencia) {
            j->pelota.animandoConvergencia = true;
            GenerarConvergenciaPelota(j);
        }

        if (j->pelota.timerRecarga <= 0.0f) {
            j->pelota.disponible = true;
            j->pelota.timerRecarga = 0.0f;
            j->pelota.animandoConvergencia = false;
            if (IsSoundReady(sonidos->item)) PlaySound(sonidos->item);
        }
    }

    if (IsKeyPressed(KEY_SPACE) && j->pelota.disponible && !j->pelota.activa) {
        j->pelota.activa = true;
        j->pelota.disponible = false;
        j->pelota.timerConvergencia = 0.0f; // Usamos el timer como tiempo de vuelo
        j->pelota.pos = (Vector2){ OFFSET_X + j->jugador.x * TAM_CELDA + TAM_CELDA / 2.0f,
                                   OFFSET_Y + j->jugador.y * TAM_CELDA + TAM_CELDA / 2.0f };
        
        float factorDiag = VEL_PELOTA_MOD * 0.7071f;
        float vx = (j->jugador.dirX != 0) ? (j->jugador.dirX * factorDiag) : factorDiag;
        float vy = (j->jugador.dirY != 0) ? (j->jugador.dirY * factorDiag) : -factorDiag;

        j->pelota.vel = (Vector2){ vx, vy };
        if (IsSoundReady(sonidos->item)) PlaySound(sonidos->item);
    }

    if (j->pelota.activa) {
        j->pelota.timerConvergencia += dt;
        
        // Timeout para que no se quede atrapada infinitamente (3 segundos)
        if (j->pelota.timerConvergencia > 3.0f) {
            j->pelota.activa = false;
            j->pelota.timerRecarga = TIEMPO_RECARGA_PELOTA;
            GenerarExplosionPelota(j, j->pelota.pos);
        } else {
            float nuevaPosX = j->pelota.pos.x + j->pelota.vel.x * dt;
            float nuevaPosY = j->pelota.pos.y + j->pelota.vel.y * dt;

            float testX = (j->pelota.vel.x > 0) ? (nuevaPosX + RADIO_PELOTA) : (nuevaPosX - RADIO_PELOTA);
            int cellX = (int)floorf((testX - OFFSET_X) / TAM_CELDA);
            int curY = (int)floorf((j->pelota.pos.y - OFFSET_Y) / TAM_CELDA);

            if (cellX < 0 || cellX >= COLUMNAS || (curY >= 0 && curY < FILAS && j->grid[curY][cellX] == TIERRA) || HayManzanaEn(j, cellX, curY)) {
                j->pelota.vel.x = -j->pelota.vel.x;
            } else {
                j->pelota.pos.x = nuevaPosX;
            }

            float testY = (j->pelota.vel.y > 0) ? (nuevaPosY + RADIO_PELOTA) : (nuevaPosY - RADIO_PELOTA);
            int curX = (int)floorf((j->pelota.pos.x - OFFSET_X) / TAM_CELDA);
            int cellY = (int)floorf((testY - OFFSET_Y) / TAM_CELDA);

            if (cellY < 0 || cellY >= FILAS || (curX >= 0 && curX < COLUMNAS && j->grid[cellY][curX] == TIERRA) || HayManzanaEn(j, curX, cellY)) {
                j->pelota.vel.y = -j->pelota.vel.y;
            } else {
                j->pelota.pos.y = nuevaPosY;
            }

            int gx = (int)floorf((j->pelota.pos.x - OFFSET_X) / TAM_CELDA);
            int gy = (int)floorf((j->pelota.pos.y - OFFSET_Y) / TAM_CELDA);

            // Permitir al jugador atrapar la pelota si vuelve a él (después de una fracción de segundo para no atraparla instantáneamente al disparar)
            if (j->pelota.timerConvergencia > 0.3f && j->jugador.x == gx && j->jugador.y == gy) {
                j->pelota.activa = false;
                j->pelota.disponible = true;
                if (IsSoundReady(sonidos->item)) PlaySound(sonidos->item);
            }

            for (int i = 0; i < j->numEnemigos; i++) {
                if (j->enemigos[i].vivo && j->enemigos[i].enJuego && j->enemigos[i].x == gx && j->enemigos[i].y == gy) {
                    j->enemigos[i].vivo = false;
                    j->enemigos[i].enJuego = false;
                    j->enemigos[i].timerRespawn = 20.0f;
                    j->pelota.activa = false;
                    j->pelota.timerRecarga = TIEMPO_RECARGA_PELOTA;
                    j->jugador.score += 500;
                    
                    GenerarExplosionPelota(j, j->pelota.pos);
                    AgregarTextoFlotante(j, gx, gy, 500);
                    if (IsSoundReady(sonidos->dinero)) PlaySound(sonidos->dinero);
                    break;
                }
            }
        }
    }

    Vector2 centroJugador = {
        OFFSET_X + j->jugador.x * TAM_CELDA + TAM_CELDA / 2.0f,
        OFFSET_Y + j->jugador.y * TAM_CELDA + TAM_CELDA / 2.0f
    };

    for (int i = 0; i < MAX_PARTICULAS; i++) {
        if (j->particulas[i].activa) {
            j->particulas[i].vida -= dt;
            if (j->particulas[i].vida <= 0.0f) {
                j->particulas[i].activa = false;
            } else {
                if (j->pelota.animandoConvergencia) {
                    Vector2 dir = { centroJugador.x - j->particulas[i].pos.x, centroJugador.y - j->particulas[i].pos.y };
                    j->particulas[i].pos.x += dir.x * 6.0f * dt;
                    j->particulas[i].pos.y += dir.y * 6.0f * dt;
                } else {
                    j->particulas[i].pos.x += j->particulas[i].vel.x * dt;
                    j->particulas[i].pos.y += j->particulas[i].vel.y * dt;
                }
            }
        }
    }

    for (int i = 0; i < MAX_TEXTOS_FLOTANTES; i++) {
        if (j->textos[i].activo) {
            j->textos[i].vida -= dt;
            j->textos[i].pos.y -= 18.0f * dt;
            if (j->textos[i].vida <= 0.0f) {
                j->textos[i].activo = false;
            }
        }
    }

    // 3. Físicas de Manzanas (Caída vertical y tambaleo)
    for (int i = 0; i < j->numManzanas; i++) {
        if (j->manzanas[i].destruida) continue;

        int mx = j->manzanas[i].x;
        int my = j->manzanas[i].y;

        bool celdaAbajoVacia = (my + 1 < FILAS && j->grid[my + 1][mx] == VACIO);
        bool tieneSoporte = ManzanaTieneSoporte(j, i);
        bool jugadorDebajo = (j->jugador.x == mx && j->jugador.y == my + 1);

        if (celdaAbajoVacia && !tieneSoporte) {
            if (HayManzanaEn(j, mx, my + 1)) {
                if (j->manzanas[i].cayendo) j->manzanas[i].destruida = true;
                j->manzanas[i].cayendo = false;
                j->manzanas[i].tambaleando = false;
                j->manzanas[i].timerCaida = 0.0f;
                continue;
            }

            if (j->manzanas[i].cayendo) {
                j->manzanas[i].timerCaida += dt;
                if (j->manzanas[i].timerCaida >= TIEMPO_CAIDA_PASO) {
                    j->manzanas[i].y++;
                    j->manzanas[i].timerCaida = 0.0f;

                    if (j->jugador.x == mx && j->jugador.y == j->manzanas[i].y) {
                        j->jugador.vidas--;
                        if (IsSoundReady(sonidos->muerte)) PlaySound(sonidos->muerte);
                        eventoMuerte = true;
                        break;
                    }

                    int enemigosAplastados = 0;
                    for (int e = 0; e < j->numEnemigos; e++) {
                        if (j->enemigos[e].vivo && j->enemigos[e].enJuego && j->enemigos[e].x == mx && j->enemigos[e].y == j->manzanas[i].y) {
                            j->enemigos[e].vivo = false;
                            j->enemigos[e].enJuego = false;
                            j->enemigos[e].timerRespawn = 30.0f;
                            enemigosAplastados++;
                        }
                    }
                    if (enemigosAplastados > 0) {
                        int pts = 1000 * enemigosAplastados;
                        j->jugador.score += pts;
                        AgregarTextoFlotante(j, mx, j->manzanas[i].y, pts);
                        if (IsSoundReady(sonidos->dinero)) PlaySound(sonidos->dinero);
                    }
                }
            } else {
                if (jugadorDebajo) {
                    j->manzanas[i].tambaleando = false;
                    j->manzanas[i].timerCaida = 0.0f;
                } else {
                    j->manzanas[i].tambaleando = true;
                    j->manzanas[i].timerCaida += dt;
                    if (j->manzanas[i].timerCaida >= TIEMPO_TAMBALEO_MANZANA) {
                        j->manzanas[i].cayendo = true;
                        j->manzanas[i].tambaleando = false;
                        j->manzanas[i].timerCaida = 0.0f;
                    }
                }
            }
        } else {
            if (j->manzanas[i].cayendo) j->manzanas[i].destruida = true;
            j->manzanas[i].cayendo = false;
            j->manzanas[i].tambaleando = false;
            j->manzanas[i].timerCaida = 0.0f;
        }
    }

    // 4. Recolección de Cerezas
    if (!eventoMuerte) {
        for (int i = 0; i < j->totalCerezas; i++) {
            if (!j->cerezas[i].recogida && j->cerezas[i].x == j->jugador.x && j->cerezas[i].y == j->jugador.y) {
                j->cerezas[i].recogida = true;
                j->jugador.score += 50;
                AgregarTextoFlotante(j, j->jugador.x, j->jugador.y, 50);
                if (IsSoundReady(sonidos->dinero)) PlaySound(sonidos->dinero);
            }
        }
    }

    // 5. IA de Enemigos
    if (!eventoMuerte) {
        for (int i = 0; i < j->numEnemigos; i++) {
            if (!j->enemigos[i].vivo || !j->enemigos[i].enJuego) continue;
            j->enemigos[i].timerPaso += dt;

            int ex = j->enemigos[i].x;
            int ey = j->enemigos[i].y;

            int targetX, targetY;
            bool modoCaza = false;
            CalcularObjetivoEnemigo(j, i, &targetX, &targetY, &modoCaza);

            int pasoX, pasoY;
            bool hayCamino = BFSCaminoLibre(j, ex, ey, targetX, targetY, &pasoX, &pasoY);
            
            if (!hayCamino && (targetX != j->jugador.x || targetY != j->jugador.y)) {
                hayCamino = BFSCaminoLibre(j, ex, ey, j->jugador.x, j->jugador.y, &pasoX, &pasoY);
            }

            float velocidad = TIEMPO_PASO_ENEMIGO_BASE - (j->nivel * DECREMENTO_POR_NIVEL);
            if (velocidad < TIEMPO_PASO_ENEMIGO_MIN) velocidad = TIEMPO_PASO_ENEMIGO_MIN;

            if (hayCamino) {
                if (j->enemigos[i].timerPaso >= velocidad) {
                    j->enemigos[i].timerPaso = 0.0f;
                    j->visitadoPorEnemigos[ey][ex]++;
                    if (!HayEnemigoEn(j, pasoX, pasoY, i)) {
                        j->enemigos[i].x = pasoX;
                        j->enemigos[i].y = pasoY;
                    }
                }
            } else if (puedeExcavar && (j->enemigos[i].excavador || !modoCaza)) {
                if (j->enemigos[i].timerPaso >= TIEMPO_PASO_ENEMIGO_TIERRA) {
                    j->enemigos[i].timerPaso = 0.0f;
                    j->visitadoPorEnemigos[ey][ex]++;

                    int dx = targetX - ex;
                    int dy = targetY - ey;
                    int stepX = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
                    int stepY = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);
                    bool prefiereX = abs(dx) >= abs(dy);

                    if (prefiereX && dx != 0 && ex + stepX >= 0 && ex + stepX < COLUMNAS && 
                        !HayManzanaEn(j, ex + stepX, ey) && !HayEnemigoEn(j, ex + stepX, ey, i) && !EsZonaNido(ex + stepX, ey)) {
                        if (j->grid[ey][ex + stepX] == TIERRA) GenerarParticulasExcavacion(j, ex + stepX, ey);
                        j->enemigos[i].x = ex + stepX;
                        j->grid[ey][ex + stepX] = VACIO;
                    } else if (dy != 0 && ey + stepY >= 0 && ey + stepY < FILAS && 
                                !HayManzanaEn(j, ex, ey + stepY) && !HayEnemigoEn(j, ex, ey + stepY, i) && !EsZonaNido(ex, ey + stepY)) {
                        if (j->grid[ey + stepY][ex] == TIERRA) GenerarParticulasExcavacion(j, ex, ey + stepY);
                        j->enemigos[i].y = ey + stepY;
                        j->grid[ey + stepY][ex] = VACIO;
                    } else if (dx != 0 && ex + stepX >= 0 && ex + stepX < COLUMNAS && 
                                !HayManzanaEn(j, ex + stepX, ey) && !HayEnemigoEn(j, ex + stepX, ey, i) && !EsZonaNido(ex + stepX, ey)) {
                        if (j->grid[ey][ex + stepX] == TIERRA) GenerarParticulasExcavacion(j, ex + stepX, ey);
                        j->enemigos[i].x = ex + stepX;
                        j->grid[ey][ex + stepX] = VACIO;
                    }
                }
            } else {
                if (j->enemigos[i].timerPaso >= velocidad) {
                    j->enemigos[i].timerPaso = 0.0f;
                    j->visitadoPorEnemigos[ey][ex]++;

                    int dirsX[4] = { j->enemigos[i].dirX, 0, 0, -j->enemigos[i].dirX };
                    int dirsY[4] = { j->enemigos[i].dirY, 1, -1, -j->enemigos[i].dirY };
                    
                    for (int d = 0; d < 4; d++) {
                        int tx = ex + dirsX[d];
                        int ty = ey + dirsY[d];
                        if (tx >= 0 && tx < COLUMNAS && ty >= 0 && ty < FILAS &&
                            j->grid[ty][tx] == VACIO && !HayManzanaEn(j, tx, ty) && !HayEnemigoEn(j, tx, ty, i) && !EsZonaNido(tx, ty)) {
                            j->enemigos[i].x = tx;
                            j->enemigos[i].y = ty;
                            j->enemigos[i].dirX = dirsX[d];
                            j->enemigos[i].dirY = dirsY[d];
                            break;
                        }
                    }
                }
            }

            if (j->enemigos[i].x == j->jugador.x && j->enemigos[i].y == j->jugador.y) {
                j->jugador.vidas--;
                if (IsSoundReady(sonidos->muerte)) PlaySound(sonidos->muerte);
                eventoMuerte = true;
                break;
            }
        }
    }

    if (eventoMuerte) {
        if (j->jugador.vidas <= 0) {
            *pantalla = pantalla_gameover;
        } else {
            IniciarNivel(j, j->nivel, cfg->modo);
        }
        return;
    }

    // 6. Condición de Victoria
    bool todasCerezas = true;
    for (int i = 0; i < j->totalCerezas; i++) {
        if (!j->cerezas[i].recogida) { todasCerezas = false; break; }
    }
    bool todosMuertos = true;
    for (int i = 0; i < j->numEnemigos; i++) {
        if (j->enemigos[i].vivo) { todosMuertos = false; break; }
    }

    if (todasCerezas || todosMuertos) {
        j->jugador.score += 2000;
        if (cfg->modo == MODO_HISTORIA) {
            if (j->nivel >= 10) {
                *pantalla = pantalla_victoria_final;
            } else {
                IniciarNivel(j, j->nivel + 1, cfg->modo);
            }
        } else {
            if (j->nivel >= 10) *pantalla = pantalla_victoria_final;
            else IniciarNivel(j, j->nivel + 1, cfg->modo);
        }
    }
}

// --- DIBUJADO DE TABLERO Y ELEMENTOS ---
void DibujarJuego(EstadoJuego *j, texturas_juego *tex, Configuracion *cfg) {
    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            Rectangle rect = { (float)(OFFSET_X + c * TAM_CELDA), (float)(OFFSET_Y + f * TAM_CELDA), TAM_CELDA, TAM_CELDA };
            if (j->grid[f][c] == TIERRA) {
                Color colTierra = (Color){ 20, 40, 140, 255 };
                if (j->nivel == 4) colTierra = (Color){ 130, 20, 100, 255 };
                if (j->nivel == 5) colTierra = (Color){ 20, 120, 30, 255 };
                if (j->nivel == 6) colTierra = (Color){ 30, 90, 130, 255 };
                if (j->nivel == 7) colTierra = (Color){ 150, 40, 140, 255 };
                if (j->nivel == 8) colTierra = (Color){ 25, 60, 170, 255 };
                if (j->nivel == 9) colTierra = (Color){ 140, 120, 20, 255 };
                if (j->nivel == 10) colTierra = (Color){ 20, 110, 130, 255 };

                DrawRectangleRec(rect, colTierra);
                DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, Fade(WHITE, 0.15f));
            } else {
                DrawRectangleRec(rect, BLACK);
            }
        }
    }

    int nidoX = OFFSET_X + 7 * TAM_CELDA;
    int nidoY = OFFSET_Y + 4 * TAM_CELDA;
    DrawRectangleGradientV(nidoX, nidoY, TAM_CELDA, TAM_CELDA, Fade(PURPLE, 0.70f), Fade(BLACK, 0.95f));
    DrawRectangleLines(nidoX, nidoY, TAM_CELDA, TAM_CELDA, VIOLET);

    for (int i = 0; i < j->totalCerezas; i++) {
        if (!j->cerezas[i].recogida) {
            int cx = OFFSET_X + j->cerezas[i].x * TAM_CELDA + TAM_CELDA / 4;
            int cy = OFFSET_Y + j->cerezas[i].y * TAM_CELDA + TAM_CELDA / 4;
            if (tex->cargadas) {
                DrawTexturePro(tex->cereza, (Rectangle){0, 0, (float)tex->cereza.width, (float)tex->cereza.height},
                               (Rectangle){(float)cx, (float)cy, 20, 20}, (Vector2){0,0}, 0.0f, WHITE);
            } else {
                DrawCircle(cx + 8, cy + 8, 8, RED);
            }
        }
    }

    for (int i = 0; i < j->numManzanas; i++) {
        if (!j->manzanas[i].destruida) {
            int vibracionX = 0;
            if (j->manzanas[i].tambaleando) {
                vibracionX = ((int)(GetTime() * 30.0f) % 2 == 0) ? -2 : 2;
            }

            int mx = OFFSET_X + j->manzanas[i].x * TAM_CELDA + 4 + vibracionX;
            int my = OFFSET_Y + j->manzanas[i].y * TAM_CELDA + 4;
            if (tex->cargadas) {
                DrawTexturePro(tex->manzana, (Rectangle){0, 0, (float)tex->manzana.width, (float)tex->manzana.height},
                               (Rectangle){(float)mx, (float)my, 32, 32}, (Vector2){0,0}, 0.0f, WHITE);
            } else {
                DrawRectangle(mx, my, 32, 32, ORANGE);
            }
        }
    }

    for (int i = 0; i < j->numEnemigos; i++) {
        if (!j->enemigos[i].vivo || !j->enemigos[i].enJuego) continue;

        int totalEnCelda = 0;
        int ordenEnCelda = 0;

        for (int k = 0; k < j->numEnemigos; k++) {
            if (j->enemigos[k].vivo && j->enemigos[k].enJuego && j->enemigos[k].x == j->enemigos[i].x && j->enemigos[k].y == j->enemigos[i].y) {
                if (k == i) ordenEnCelda = totalEnCelda;
                totalEnCelda++;
            }
        }

        int tamBase = (totalEnCelda > 1) ? 24 : 32;
        int offsetApilado = (totalEnCelda > 1) ? (ordenEnCelda * 5) : 4;

        int ex = OFFSET_X + j->enemigos[i].x * TAM_CELDA + offsetApilado;
        int ey = OFFSET_Y + j->enemigos[i].y * TAM_CELDA + offsetApilado;

        if (tex->cargadas) {
            DrawTexturePro(tex->enemigo, (Rectangle){0, 0, (float)tex->enemigo.width, (float)tex->enemigo.height},
                           (Rectangle){(float)ex, (float)ey, (float)tamBase, (float)tamBase}, (Vector2){0,0}, 0.0f, WHITE);
        } else {
            Color colorEnemigo = j->enemigos[i].excavador ? MAROON : PURPLE;
            DrawRectangle(ex, ey, tamBase, tamBase, colorEnemigo);
            if (totalEnCelda > 1) {
                DrawRectangleLines(ex, ey, tamBase, tamBase, RAYWHITE);
            }
        }
    }

    for (int i = 0; i < MAX_PARTICULAS; i++) {
        if (j->particulas[i].activa) {
            float ratio = j->particulas[i].vida / j->particulas[i].vidaMax;
            DrawCircleV(j->particulas[i].pos, 3.5f * ratio + 1.0f, Fade(j->particulas[i].col, ratio));
        }
    }

    for (int i = 0; i < MAX_TEXTOS_FLOTANTES; i++) {
        if (j->textos[i].activo) {
            DrawText(TextFormat("+%d", j->textos[i].puntos), (int)j->textos[i].pos.x, (int)j->textos[i].pos.y, 14, YELLOW);
        }
    }

    if (j->pelota.activa) {
        DrawCircleV(j->pelota.pos, RADIO_PELOTA + 1.0f, GOLD);
        DrawCircleV(j->pelota.pos, RADIO_PELOTA - 2.0f, YELLOW);
    }

    int px = OFFSET_X + j->jugador.x * TAM_CELDA + 4;
    int py = OFFSET_Y + j->jugador.y * TAM_CELDA + 4;
    if (tex->cargadas) {
        DrawTexturePro(tex->mrdo, (Rectangle){0, 0, (float)tex->mrdo.width, (float)tex->mrdo.height},
                       (Rectangle){(float)px, (float)py, 32, 32}, (Vector2){0,0}, 0.0f, WHITE);
    } else {
        DrawRectangle(px, py, 32, 32, LIME);
    }

    DrawText((cfg->idioma == IDIOMA_ES) ? "JUGADOR" : "PLAYER", 10, 20, 12, GRAY);
    DrawText(j->jugador.nombre, 10, 36, 14, WHITE);
    DrawText((cfg->idioma == IDIOMA_ES) ? "SCORE" : "SCORE", 10, 70, 12, GRAY);
    DrawText(TextFormat("%05d", j->jugador.score), 10, 86, 16, YELLOW);
    DrawText((cfg->idioma == IDIOMA_ES) ? "VIDAS" : "LIVES", 10, 120, 12, GRAY);
    for (int v = 0; v < j->jugador.vidas; v++) {
        DrawRectangle(10 + v * 16, 138, 10, 10, RED);
    }
    DrawText((cfg->idioma == IDIOMA_ES) ? "NIVEL" : "LEVEL", 10, 170, 12, GRAY);
    
    int displayNivel = (j->nivel == 10) ? 0 : j->nivel;
    DrawText(TextFormat("SCENE %02d", displayNivel), 10, 186, 14, GREEN);

    DrawText((cfg->idioma == IDIOMA_ES) ? "ARMA" : "WEAPON", 10, 230, 12, GRAY);
    if (j->pelota.disponible) {
        DrawCircle(18, 255, 6, GOLD);
        DrawText((cfg->idioma == IDIOMA_ES) ? "LISTA" : "READY", 30, 249, 13, GREEN);
    } else if (j->pelota.activa) {
        DrawCircleLines(18, 255, 6, YELLOW);
        DrawText((cfg->idioma == IDIOMA_ES) ? "VUELO" : "FLYING", 30, 249, 13, YELLOW);
    } else {
        DrawCircleLines(18, 255, 6, RED);
        DrawText(TextFormat("%.1fs", j->pelota.timerRecarga), 30, 249, 13, RED);
    }
}

void GuardarRecord(int score, const char *nombre) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s,%d", nombre, score);
    SaveFileText("highscore.dat", buffer);
}

int CargarRecord(char *nombreOut) {
    if (FileExists("highscore.dat")) {
        char *texto = LoadFileText("highscore.dat");
        if (texto != NULL) {
            char nombreBuffer[32] = { 0 };
            int score = 0;
            if (sscanf(texto, "%31[^,],%d", nombreBuffer, &score) == 2) {
                if (nombreOut != NULL) {
                    strncpy(nombreOut, nombreBuffer, 15);
                    nombreOut[15] = '\0';
                }
                UnloadFileText(texto);
                return score;
            }
            score = atoi(texto);
            UnloadFileText(texto);
            return score;
        }
    }
    return 0;
}
