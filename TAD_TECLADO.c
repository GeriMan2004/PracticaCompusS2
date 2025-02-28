#include <xc.h>
#include "TAD_TECLADO.h"

static unsigned char Filas, Columnas, timer, tecla = 0;


static unsigned char ReadFilas(void) {
    return (PORTA & 0x0F);  // Read only the lower 4 bits
}

void initTeclado(void) {
    Filas = 0x00;    // Initialize rows to all high (pulled up)
    Columnas = 0x00;  // Initialize columns to all high
    tecla = 0;
    TI_NewTimer(&timer);
}
/**
 * En esta funcin, seguiremos la logica del motor implementado desde el diseo de un examen (LSElevator)
 * En este motor, utilizan el barrido en las columnas en vez de en las filas, esto es debido a que de esta
 * manera se ahorran un estado
 */
void motorTeclado(void) {
	static char state = 0;
	Filas = ReadFilas();
	switch(state) {
		case 0:
			if (Filas == 0x0) {
				Columnas = (0x01);
				LATB = Columnas;
				state = 1;
			}
			else if (Filas != 0x0) {
				TI_ResetTics(timer);
				state = 3;
			}
		break;
		case 1:
			if (Filas == 0x0) {
				Columnas = (0x02);
				LATB = Columnas;
				state = 2;
			}
			else if (Filas != 0x0) {
				TI_ResetTics(timer);
				state = 3;
			}
		break;
		case 2:
			if (Filas != 0x0) {
				TI_ResetTics(timer);
				state = 3;
			}
			else if (Filas == 0x0) {
				Columnas = (0x04);
				LATB = Columnas;
				state = 0;
			}
		break;
		case 3:
			tecla = GetTecla ();
			if (Filas == 0x0) {
				Columnas = (0x04);
				LATB = Columnas;
				state = 0;
			}
			else if (Filas != 0x0 && TI_GetTics(timer) > REBOTE && tecla != 11) {
				LATD = tecla;
				showTecla();
				state = 5;
			}
			else if (Filas != 0x0 && TI_GetTics(timer) > REBOTE && tecla == 11) {
				TI_ResetTics(timer);
				state = 4;
			}
		break;
		case 4:
			if (Filas == 0x0) {
				state = 0;
			}
			else if (Filas != 0x0 && TI_GetTics(timer) > HASHTAG_TIME) {
				// ResetData();
				state = 5;
			}
		break;
		case 5:
			if (Filas == 0x0) {
				state = 0;
				Columnas = (0x04);
				LATB = Columnas;
			}
		break;
	}
	LATD = (LATD & 0x0F) | ((unsigned char)(state << 4));
}

unsigned char GetTecla(void) {
    unsigned char fila = 0;
    unsigned char columna = 0;
    
    switch(Filas) {
        case 0x1: fila = 0; break;
        case 0x2: fila = 1; break;
        case 0x4: fila = 2; break;
        case 0x8: fila = 3; break;
        default: return 0xFF;  // Invalid/multiple/no keys
    }
    
    // Find which column is active
    switch(Columnas & 0x07) {
        case 0x01: columna = 0; break;  // 110
        case 0x02: columna = 1; break;  // 101
        case 0x04: columna = 2; break;  // 011
        default: return 0xFF;  // Invalid column
    }
    
    const unsigned char keymap[4][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
        {10, 0, 11}  // 10 = *, 11 = #
    };
    
    return keymap[fila][columna];
}

void showTecla(void) {    
    // In this test, i just want to print the key pressed
    LATD = tecla;
}