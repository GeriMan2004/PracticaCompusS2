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

// Versión cooperativa de SendChar
char motor_SendChar(char c) {
    // Si no hay espacio en el buffer de transmisión, retornar 0 (necesita más llamadas)
    if (Terminal_TXAvailable() == 0) {
        return 0;
    }
    
    // Si hay espacio, enviar el caracter y retornar 1 (completado)
    TXREG = c;
    return 1;
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

// Versión cooperativa de SendString
char motor_SendString(void) {
    switch(state_str) {
        case 0: // Estado inicial: espera un string para enviar
            return 1; // Listo para recibir un nuevo string (estado de inactividad)
            
        case 1: // Estado de envío: procesando caracteres
            if (*str_ptr == 0) { // Final del string
                state_str = 0;       // Volver al estado inicial
                return 1;        // Completado
            }
            
            // Intentar enviar el caracter actual
            if (motor_SendChar(*str_ptr)) {
                str_ptr++; // Avanzar al siguiente caracter
            }
            return 0; // Necesita más llamadas
    }
    
    return 0; // Por defecto: necesita más llamadas
}

// Función para iniciar el envío de un string con el motor
void motor_StartSendString(const char* str) {
    if (motor_SendString() == 1) { // Verificar que el motor esté inactivo
        str_ptr = str;             // Guardar el puntero al string
        state_str = 1;                 // Cambiar al estado de envío
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
	static char sending_string = 0; // Flag para indicar si estamos enviando texto
	
	// Si estamos enviando un string, seguir procesándolo
	if (sending_string) {
		if (motor_SendString() == 1) {
			sending_string = 0; // Terminamos de enviar
		} else {
			return; // Seguimos enviando, no hacer más en este ciclo
		}
	}

	switch(state) {
		case 0:
			if (Terminal_ReceiveChar() == 0x1B) {
				motor_StartSendString("---------------\r\n");
				sending_string = 1;
				state = 10; // Estado intermedio para mostrar el menú con cooperación
			}
			
			if (hashtag_pressed == 1){
				motor_StartSendString("---------------\r\n");
				sending_string = 1;
				state = 10; // Estado intermedio para mostrar el menú con cooperación
				hashtag_pressed = 0;
			}
		break;
		
		case 10: // Menú - parte 1
			if (!sending_string) {
				motor_StartSendString("Menú principal\r\n");
				sending_string = 1;
				state = 11;
			}
		break;
		
		case 11: // Menú - parte 2
			if (!sending_string) {
				motor_StartSendString("---------------\r\n");
				sending_string = 1;
				state = 12;
			}
		break;
		
		case 12: // Menú - parte 3
			if (!sending_string) {
				motor_StartSendString("Tria una opció:\r\n");
				sending_string = 1;
				state = 13;
			}
		break;
		
		case 13: // Menú - parte 4
			if (!sending_string) {
				motor_StartSendString("\t1. Qui hi ha a la sala?\r\n");
				sending_string = 1;
				state = 14;
			}
		break;
		
		case 14: // Menú - parte 5
			if (!sending_string) {
				motor_StartSendString("\t2. Mostrar configuracions\r\n");
				sending_string = 1;
				state = 15;
			}
		break;
		
		case 15: // Menú - parte 6
			if (!sending_string) {
				motor_StartSendString("\t3. Modificar hora del sistema\r\n");
				sending_string = 1;
				state = 16;
			}
		break;
		
		case 16: // Menú - parte 7 (final)
			if (!sending_string) {
				motor_StartSendString("Opció: ");
				sending_string = 1;
				state = 1;
			}
		break;
		
		case 1:
			if(Terminal_RXAvailable() == 1){
				if (Terminal_ReceiveChar() == '1') {
					motor_StartSendString("\r\n");
					sending_string = 1;
					state = 20; // Estado para mostrar UID
				}
				else if (Terminal_ReceiveChar() == '2') {
					motor_StartSendString("\r\n");
					sending_string = 1;
					state = 30; // Estado para mostrar configuraciones
				}
				else if (Terminal_ReceiveChar() == '3') {
					motor_StartSendString("\r\n");
					sending_string = 1;
					state = 40; // Estado para cambiar la hora
				}
				else {
					motor_StartSendString("ERROR. Valor introduit erroni.\r\n");
					sending_string = 1;
					state = 0;
				}
			}
		break;
		
		case 20: // Mostrar UID - parte 1
			if (!sending_string) {
				unsigned char currentUser[5];
				getActualUID(currentUser);
				
				if (currentUser[0] != 0) {
					motor_StartSendString("UID: ");
					sending_string = 1;
					state = 21; // Continuar mostrando UID
				} else {
					motor_StartSendString("No hi ha cap usuari a la sala.\r\n");
					sending_string = 1;
					state = 25; // Saltar a final
				}
			}
		break;
		
		case 21: // Mostrar UID - parte 2 (imprimir UID - delegamos a otra función)
			if (!sending_string) {
				unsigned char currentUser[5];
				getActualUID(currentUser);
				printfUID(currentUser);
				state = 25;
			}
		break;
		
		case 25: // Volver a menú principal
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
		
		case 40: // Cambiar hora - parte 1
			if (!sending_string) {
				motor_StartSendString("Introduce la hora actual(HHMM): ");
				sending_string = 1;
				state = 2; // Ir al estado original de entrada de hora
			}
		break;
		
		case 2: // Estado original para entrada de hora
			if(Terminal_RXAvailable() == 1){
				static unsigned char hour[4] = "0000";
				static char index = 0;
				hour[index] = Terminal_ReceiveChar();
				Terminal_SendChar(hour[index]); // Esto podría hacerse cooperativo también
				index++;
				if(index == 4){
					saveHourToData(hour);
					motor_StartSendString("\r\nHora introduida correctament\r\n");
					sending_string = 1;
					index = 0;
					state = 0;
				}
			}
		break;
	}
}
