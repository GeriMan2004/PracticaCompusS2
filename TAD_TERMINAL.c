#include <xc.h>
#include "TAD_TERMINAL.h"
#include "TAD_DATOS.h"

// Variables globales para el motor SendString
static const char* str_ptr = NULL;
static char state_str = 0;
static char hashtag_pressed = 0;

// Inicializar para Serial 9615 baud rate
void Terminal_Init(void){
	TXSTA = 0x24;
	RCSTA = 0x90;
	SPBRG = 255;
	BAUDCON = 0x00;
	hashtag_pressed = 0;
}

// Verificar si hay datos disponibles para enviar/recibir
int Terminal_TXAvailable(void) { return (PIR1bits.TXIF == 1); }
char Terminal_RXAvailable(void) { return (PIR1bits.RCIF == 1); }
char Terminal_ReceiveChar(void) { return RCREG; }

// Enviar un caracter
void Terminal_SendChar(char c) {
	while (!Terminal_TXAvailable());
	TXREG = c;
}

void Terminal_SendString(const char *str) {
	while (*str) {
		Terminal_SendChar(*str++);
	}
}

// Versión cooperativa de SendChar
char motor_SendChar(char c) {
	if (!Terminal_TXAvailable()) return 0;
	TXREG = c;
	return 1;
}

// Versión cooperativa de SendString
char motor_SendString(void) {
	switch(state_str) {
		case 0: return 1;
		case 1:
			if (!*str_ptr) {
				state_str = 0;
				return 1;
			}
			if (motor_SendChar(*str_ptr)) str_ptr++;
			return 0;
	}
	return 0;
}

void motor_StartSendString(const char* str) {
	if (motor_SendString() == 1) {
		str_ptr = str;
		state_str = 1;
	}
}

void hashtag_pressed3s(void) { hashtag_pressed = 1; }

// Función optimizada para imprimir UID
void printfUID(unsigned char *currentUser) {
	static const char hex[] = "0123456789ABCDEF";
	Terminal_SendString("UID: ");
	for (int i = 0; i < 5; i++) {
		Terminal_SendChar(hex[currentUser[i] >> 4]);
		Terminal_SendChar(hex[currentUser[i] & 0x0F]);
		if (i < 4) Terminal_SendString("-");
	}
	Terminal_SendString("\r\n");
}

// Función optimizada para imprimir configuración de LEDs
void printLedConfig(unsigned char *leds) {
	static const char hex[] = "0123456789ABCDEF";
	for (int i = 0; i < 6; i++) {
		Terminal_SendChar('L');
		Terminal_SendChar('0' + i);
		Terminal_SendString(": ");
		Terminal_SendChar(hex[leds[i]]);
		if (i < 5) Terminal_SendString(" - ");
	}
	Terminal_SendString("\r\n");
}

// Motor de terminal optimizado
void motorTerminal(void) {
	static char state = 0;
	static char sending_string = 0;
	static unsigned char hour[4] = "0000";
	static char index = 0;

	if (sending_string) {
		if (motor_SendString() == 1) {
			sending_string = 0;
		} else {
			return;
		}
	}

	switch(state) {
		case 0:
			if (Terminal_ReceiveChar() == 0x1B || hashtag_pressed) {
				motor_StartSendString("---------------\r\n");
				sending_string = 1;
				state = 10;
				hashtag_pressed = 0;
			}
			break;

		case 10: // Menú principal
			if (!sending_string) {
				motor_StartSendString("Menú principal\r\n---------------\r\nTria una opció:\r\n");
				sending_string = 1;
				state = 13;
			}
			break;

		case 13: // Opciones del menú
			if (!sending_string) {
				motor_StartSendString("\t1. Qui hi ha a la sala?\r\n\t2. Mostrar configuracions\r\n\t3. Modificar hora del sistema\r\nOpció: ");
				sending_string = 1;
				state = 1;
			}
			break;

		case 1: // Procesar opción seleccionada
			if(Terminal_RXAvailable()) {
				char opcion = Terminal_ReceiveChar();
				if (opcion >= '1' && opcion <= '3') {
					motor_StartSendString("\r\n");
					sending_string = 1;
					state = (opcion - '1') * 10 + 20;
				} else {
					motor_StartSendString("ERROR. Valor introduit erroni.\r\n");
					sending_string = 1;
					state = 0;
				}
			}
			break;

		case 20: // Mostrar UID
			if (!sending_string) {
				unsigned char currentUser[5];
				getActualUID(currentUser);
				if (currentUser[0]) {
					printfUID(currentUser);
				} else {
					motor_StartSendString("No hi ha cap usuari a la sala.\r\n");
					sending_string = 1;
				}
				state = 25;
			}
			break;

		case 25: // Volver a menú
			if (!sending_string) {
				motor_StartSendString("\r\n");
				sending_string = 1;
				state = 0;
			}
			break;

		case 30: // Mostrar configuraciones
			if (!sending_string) {
				showAllConfigurations();
				state = 0;
			}
			break;

		case 40: // Cambiar hora
			if (!sending_string) {
				motor_StartSendString("Introduce la hora actual(HHMM): ");
				sending_string = 1;
				state = 2;
				index = 0;
			}
			break;

		case 2: // Procesar entrada de hora
			if(Terminal_RXAvailable()) {
				hour[index] = Terminal_ReceiveChar();
				Terminal_SendChar(hour[index]);
				if(++index == 4) {
					saveHourToData(hour);
					motor_StartSendString("\r\nHora introduida correctament\r\n");
					sending_string = 1;
					state = 0;
				}
			}
			break;
	}
}