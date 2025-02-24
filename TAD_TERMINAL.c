#include <xc.h>
#include "TAD_TERMINAL.h"

static unsigned char state = 0;

// Inicializar para Serial 9615 baud rate
void Terminal_Init(void){
	state = 0;
	TXSTA = 0x24;
	RCSTA = 0x90;
	SPBRG = 64;
	BAUDCON = 0x00;
}

// Verificar si hay datos disponibles para enviar
int Terminal_TXAvailable(void) {
	return (PIR1bits.TXIF == 1) ? 1 : 0;
}

// Verificar si hay datos disponibles para recibir
void Terminal_RXAvailable(void) {
	return (PIR1bits.RCIF == 1) ? 1 : 0;
}

// Enviar un caracter
void Terminal_SendChar(char c) {
	TXREG = c;
}

// Recibir un caracter
char Terminal_ReceiveChar(void) {
	return RCREG;
}

// Enviar una cadena de caracteres
void Terminal_SendString(const char *str) {
	while (*str) {
		while (Terminal_TXAvailable() == 0);
		Terminal_SendChar(*str++);
	}
}

