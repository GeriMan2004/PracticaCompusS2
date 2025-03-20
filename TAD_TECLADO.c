#include <xc.h>
#include "TAD_TECLADO.h"
#include "TAD_TERMINAL.h"
#include "TAD_DATOS.h"
#include "TAD_TIMER.h"

// Bits para control de columnas (corresponden a los pines físicos)
#define COL1 (1<<5)
#define COL2 (1<<6)
#define COL3 (1<<4)

// Variables globales compactas
static unsigned char Filas, Columnas, timer, tecla, state;

// Mapeo de teclado compacto - usando constante para ahorrar memoria de programa
static const char keymap[] = {
    0x01, 0x02, 0x03,  // Fila 1: 1,2,3
    0x04, 0x05, 0x06,  // Fila 2: 4,5,6
    0x07, 0x08, 0x09,  // Fila 3: 7,8,9
    0x0A, 0x00, 0x0B   // Fila 4: *,0,#
};

// Tabla de valores de columna para secuencia de escaneo
static unsigned char colValues[] = {COL1, COL2, COL3};

// Función optimizada para leer filas
#define ReadFilas() (PORTD & 0x0F)

// Inicialización del teclado
void initTeclado(void) {
    // Configuración de puertos directamente
    TRISD = 0x0F;  // Bits 0-3 como entradas (filas), 4-7 como salidas (columnas)
    LATD = 0x00;   // Inicializar salidas a 0
    
    // Inicializa variables de estado
    Filas = Columnas = tecla = state = 0;
    
    // Crear timer para rebotes
    TI_NewTimer(&timer);
}

// La función writeColumnas simplificada usando índice
void writeColumnas(void) {
    if (Columnas < 3) {
        LATD = colValues[Columnas];
    } else {
        LATD = 0;
    }
}

// Función optimizada GetTecla que evita switch múltiples
unsigned char GetTecla(void) {
    unsigned char fila = 0, columna = 0;
    
    // Detectar fila activa con operaciones de bit
    switch(Filas) {
        case 0x1: fila = 0; break;
        case 0x2: fila = 1; break;
        case 0x4: fila = 2; break;
        case 0x8: fila = 3; break;
        default: return 0xFF;
    }
    
    // Mapear columna según valor actual de Columnas
    columna = Columnas;
    
    // Devolver valor del keymap usando acceso directo al array unidimensional
    return keymap[fila * 3 + columna];
}

// Motor de teclado optimizado con menos código repetido
void motorTeclado(void) {
    // Lectura común de filas para todos los estados
    Filas = ReadFilas();
    
    switch(state) {
        case 0:
            if (Filas) {
                TI_ResetTics(timer);
                state = 3;
            } else {
                Columnas = 0; // Primera columna
                writeColumnas();
                state = 1;
            }
            break;
            
        case 1:
            if (Filas) {
                TI_ResetTics(timer);
                state = 3;
            } else {
                Columnas = 1; // Segunda columna
                writeColumnas();
                state = 2;
            }
            break;
            
        case 2:
            if (Filas) {
                TI_ResetTics(timer);
                state = 3;
            } else {
                Columnas = 2; // Tercera columna
                writeColumnas();
                state = 0;
            }
            break;
            
        case 3:
            tecla = GetTecla();
            if (!Filas) {
                Columnas = 2; // Volver a columna 3
                writeColumnas();
                state = 0;
            } else if (TI_GetTics(timer) > REBOTE) {
                if (tecla != 0x0B) {
                    setLed(tecla);
                    state = 5;
                } else {
                    TI_ResetTics(timer);
                    state = 4;
                }
            }
            break;
            
        case 4:
            if (!Filas) {
                state = 0;
            } else if (TI_GetTics(timer) > HASHTAG_TIME) {
                hashtag_pressed3s();
                state = 5;
            }
            break;
            
        case 5:
            if (!Filas) {
                state = 0;
                Columnas = 2;
                writeColumnas();
            }
            break;
    }
}