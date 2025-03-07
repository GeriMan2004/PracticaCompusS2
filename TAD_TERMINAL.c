#include <xc.h>
#include "TAD_TERMINAL.h"
#include "TAD_TECLADO.h"
#include "TAD_DATOS.h"
#include <stdio.h>
#include <string.h>

char hashtag_pressed = 0;

// Inicializar para Serial 9615 baud rate
void Terminal_Init(void){
	TXSTA = 0x24;
	RCSTA = 0x90;
	SPBRG = 255;
	BAUDCON = 0x00;
	hashtag_pressed = 0;
}

// Verificar si hay datos disponibles para enviar
int Terminal_TXAvailable(void) {
	return (PIR1bits.TXIF == 1) ? 1 : 0;
}

// Verificar si hay datos disponibles para recibir
char Terminal_RXAvailable(void) {
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

void showMenu(void) {
	Terminal_SendString("---------------\r\n");
	Terminal_SendString("Menú principal\r\n");
	Terminal_SendString("---------------\r\n");
	Terminal_SendString("Tria una opció:\r\n");
	Terminal_SendString("\t1. Qui hi ha a la sala?\r\n");
	Terminal_SendString("\t2. Mostrar configuracions\r\n");
	Terminal_SendString("\t3. Modificar hora del sistema\r\n");
	Terminal_SendString("Opció: ");
}

void hashtag_pressed3s(void){
	hashtag_pressed = 1;
}


void motorTerminal(void) {
	static char state = 0;

	switch(state) {
		case 0:
			if (Terminal_ReceiveChar() == 0x1B) {
				showMenu();
				state = 1;
			}
			if (hashtag_pressed == 1){
				showMenu();
				state = 1;
				hashtag_pressed = 0;
			}
		break;
		case 1:
			if(Terminal_RXAvailable() == 1){
				initTeclado();
				if (Terminal_ReceiveChar() == '1') {
					Terminal_SendString("Has pulsado 1\r\n");
					// getActualUID();
					state = 0;  
				}
				else if (Terminal_ReceiveChar() == '2') {
					Terminal_SendString("Has pulsado 2\r\n");
					// getUserConfiguration();
					state = 0;
				}
				else if (Terminal_ReceiveChar() == '3') {
					Terminal_SendString("Has pulsado 3\r\n");
					//modifyHour();
					state = 0;
				}
				else {
					Terminal_SendString("ERROR. Valor introduit erroni.\r\n");
					state = 0;
				}
			}
		break;
	}
}
