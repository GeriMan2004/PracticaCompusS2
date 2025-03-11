#include <xc.h>
#include "TAD_TECLADO.h"
#include "TAD_TERMINAL.h"
#include "TAD_DATOS.h"

static unsigned char Filas, Columnas, timer, tecla = 0, state = 0;


static unsigned char ReadFilas(void) {
    return (PORTD & 0x0F);  // Read only the lower 4 bits
}

void initTeclado(void) {
	initPortsTeclado();
    Filas = 0x00;    // Initialize rows to all high (pulled up)
    Columnas = 0x00;  // Initialize columns to all high
    tecla = 0;
	state = 0;
    TI_NewTimer(&timer);
}

void initPortsTeclado(void) {
	// set portd as digital
	
	TRISD = 0x0F; // Set the lower 4 bits as inputs, Filas, and the upper 3 bits (4, 5 y 6) as outpus, Columnas
	LATD = 0x00;  // Initialize all columns to low
}
/**
 * En esta funcin, seguiremos la logica del motor implementado desde el diseo de un examen (LSElevator)
 * En este motor, utilizan el barrido en las columnas en vez de en las filas, esto es debido a que de esta
 * manera se ahorran un estado
 */
void motorTeclado(void) {
	switch(state) {
		case 0:
			Filas = ReadFilas();
			if (Filas == 0x0) {
				Columnas = (0x01);
				writeColumnas();
				state = 1;
			}
			else if (Filas != 0x0) {
				TI_ResetTics(timer);
				state = 3;
			}
		break;
		case 1:
			Filas = ReadFilas();
			if (Filas == 0x0) {
				Columnas = (0x02);
				writeColumnas();
				state = 2;
			}
			else if (Filas != 0x0) {
				TI_ResetTics(timer);
				state = 3;
			}
		break;
		case 2:
			Filas = ReadFilas();
			if (Filas != 0x0) {
				TI_ResetTics(timer);
				state = 3;
			}
			else if (Filas == 0x0) {
				Columnas = (0x04);
				writeColumnas();
				state = 0;
			}
		break;
		case 3:
			tecla = GetTecla ();
			Filas = ReadFilas();
			if (Filas == 0x0) {
				Columnas = (0x04);
				writeColumnas();
				state = 0;
			}
			else if (Filas != 0x0 && TI_GetTics(timer) > REBOTE && tecla != 0x0B) {
				setLed(tecla);
				state = 5;
			}
			else if (Filas != 0x0 && TI_GetTics(timer) > REBOTE && tecla == 0x0B) {
				TI_ResetTics(timer);
				state = 4;
			}
		break;
		case 4:
			Filas = ReadFilas();
			if (Filas == 0x0) {
				state = 0;
			}
			else if (Filas != 0x0 && TI_GetTics(timer) > HASHTAG_TIME) {
				hashtag_pressed3s();
				// ResetData();
				state = 5;
			}
		break;
		case 5:
			Filas = ReadFilas();
			if (Filas == 0x0) {
				state = 0;
				Columnas = (0x04);
				writeColumnas();
			}
		break;
	}
}
/**
 * En esta función, printaremos la columna en el puerto D, pero teniendo en cuenta que utilizo los bits 4-6
 * y que en vez de ser columna 1 es bit 4, columna 2 es bit 5 y columna 3 es bit 6, he hecho que columna 1 
 * sea el bit 5, columna 2 sea el bit 6 y columna 3 sea el bit 4, esto debido a que era mas facil de soldar
 */
void writeColumnas(void) {
    LATD = (0x00);  
    if (Columnas == 0x01) {
        // Columna 1 -> Bit 5
        LATD |= (1 << 5);  // Set bit 5
    } else if (Columnas == 0x02) {
        // Columna 2 -> Bit 6
        LATD |= (1 << 6);  // Set bit 6
    } else if (Columnas == 0x04) {
        // Columna 3 -> Bit 4
        LATD |= (1 << 4);  // Set bit 4
    }
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
        {0x01, 0x02, 0x03},
        {0x04, 0x05, 0x06},
        {0x07, 0x08, 0x09},
        {0x0A, 0x00, 0x0B}  // 10 = *, 11 = #
    };
    
    return keymap[fila][columna];
}