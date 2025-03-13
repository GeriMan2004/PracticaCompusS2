#include <xc.h>
#include "TAD_TERMINAL.h"
#include "TAD_DATOS.h"
#include "TAD_DISPLAY.h"

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
	while (Terminal_TXAvailable() == 0);
	TXREG = c;
}

// Recibir un caracter
char Terminal_ReceiveChar(void) {
	return RCREG;
}

// Enviar una cadena de caracteres
void Terminal_SendString(const char *str) {
	while (*str) {
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

void printfUID(unsigned char *currentUser) {
	Terminal_SendString("UID: ");
	for (int i = 0; i < 5; i++) {
		// Convert high nibble to hex
		unsigned char high = (currentUser[i] >> 4) & 0x0F;
		Terminal_SendChar(high < 10 ? '0' + high : 'A' + high - 10);
		
		// Convert low nibble to hex
		unsigned char low = currentUser[i] & 0x0F;
		Terminal_SendChar(low < 10 ? '0' + low : 'A' + low - 10);
		
		// Add separator between bytes
		if (i < 4) Terminal_SendString("-");
	}
	Terminal_SendString("\r\n");
}

void printLedConfig(unsigned char *leds) {
	for (int i = 0; i < MAX_LEDS; i++) {
		// LED label with padding
		Terminal_SendChar('L');
		Terminal_SendChar('0' + i);
		Terminal_SendString(": ");
		
		// Convert intensity to hex
		unsigned char val = leds[i];
		Terminal_SendChar(val < 10 ? '0' + val : 'A' + val - 10);
		
		// Add separator with proper spacing
		if (i < MAX_LEDS - 1) Terminal_SendString(" - ");
	}
	Terminal_SendString("\r\n");
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
				if (Terminal_ReceiveChar() == '1') {
					Terminal_SendString("\r\n");
					unsigned char *currentUser = getActualUID();
					

					// For each byte of the UID
					printfUID(currentUser);

					Terminal_SendString("\r\n");
					state = 0;  
				}
				else if (Terminal_ReceiveChar() == '2') {
					Terminal_SendString("\r\n");
					showAllConfigurations();
					state = 0;
				}
				else if (Terminal_ReceiveChar() == '3') {
					Terminal_SendString("\r\n");
					Terminal_SendString("Introduce la hora actual(HHMM): ");
					state = 2;
				}
				else {
					Terminal_SendString("ERROR. Valor introduit erroni.\r\n");
					state = 0;
				}
			}
		break;
		case 2:
			if(Terminal_RXAvailable() == 1){
				static unsigned char hour[4] = "0000";
				static char index = 0;
				hour[index] = Terminal_ReceiveChar();
				Terminal_SendChar(hour[index]);
				index++;
				if(index == 4){
					saveHourToData(hour);
					Terminal_SendString("\r\nHora introduida correctament\r\n");
					index = 0;
					state = 0;
				}
			}
		break;
	}
}
