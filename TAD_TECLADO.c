#include <xc.h>
#include "TAD_TECLADO.h"
#include "TAD_TERMINAL.h"
#include "TAD_DATOS.h"
#include "TAD_TIMER.h"

// Bits para control de columnas (corresponden a los pines f�sicos)
#define COL1 (1<<5)
#define COL2 (1<<6)
#define COL3 (1<<4)

// Variables globales compactas
static unsigned char Filas, Columnas, timer_teclado, tecla, state;

// Mapeo de teclado compacto - usando constante para ahorrar memoria de programa
static const char keymap[] = {
    0x01, 0x02, 0x03,  // Fila 1: 1,2,3
    0x04, 0x05, 0x06,  // Fila 2: 4,5,6
    0x07, 0x08, 0x09,  // Fila 3: 7,8,9
    0x0A, 0x00, 0x0B   // Fila 4: *,0,#
};

static unsigned char colValues[] = {COL1, COL2, COL3};

#define ReadFilas() (PORTD & 0x0F)
void initTeclado(void) {
    TRISD = 0x0F;  
    LATD = 0x00;   
    
    Filas = Columnas = tecla = state = 0;
    
    TI_NewTimer(&timer_teclado);
}


void writeColumnas(void) {
    if (Columnas < 3) {
        LATD = colValues[Columnas];
    } else {
        LATD = 0;
    }
}

unsigned char GetTecla(void) {
    unsigned char fila = 0, columna = 0;
                    
    switch(Filas) {
        case 0x1: fila = 0; break;
        case 0x2: fila = 1; break;
        case 0x4: fila = 2; break;
        case 0x8: fila = 3; break;
        default: return 0xFF;
    }
    
    columna = Columnas;
    
    return keymap[fila * 3 + columna];
}

void motorTeclado(void) {
    Filas = ReadFilas();
    
    switch(state) {
        case 0:
            if (Filas) {
                TI_ResetTics(timer_teclado);
                state = 3;
            } else {
                Columnas = 0; 
                writeColumnas();
                state = 1;
            }
            break;
            
        case 1:
            if (Filas) {
                TI_ResetTics(timer_teclado);
                state = 3;
            } else {
                Columnas = 1; 
                writeColumnas();
                state = 2;
            }
            break;
            
        case 2:
            if (Filas) {
                TI_ResetTics(timer_teclado);
                state = 3;
            } else {
                Columnas = 2; 
                writeColumnas();
                state = 0;
            }
            break;
            
        case 3:
            tecla = GetTecla();
            if (!Filas) {
                Columnas = 2; 
                writeColumnas();
                state = 0;
            } else if (TI_GetTics(timer_teclado) > REBOTE) {
                if (tecla != 0x0B) {
                    setLed(tecla);
                    state = 5;
                } else {
                    TI_ResetTics(timer_teclado);
                    state = 4;
                }
            }
            break;
            
        case 4:
            if (!Filas) {
                state = 0;
                hashtag_pressed3s();
            } else if (TI_GetTics(timer_teclado) > HASHTAG_TIME) {
                state = 5;
                resetData();
                motor_StartSendString("\r\nS'han resetejat les dades\r\n");
                setStartSendString();
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