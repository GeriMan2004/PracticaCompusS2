#include <xc.h>
#include "TAD_TERMINAL.h"
#include "TAD_DATOS.h"

// Variables globales y inicialización de otras variables
static const char* str_ptr = NULL;
static char state_str = 0;
static char hashtag_pressed = 0;
static char sending_string = 0;
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

char motor_SendChar(char c) {
	if (!Terminal_TXAvailable()) return 0; // en caso de que el puerto TX aun no esté disponible, no enviamos el char y devolveremos 0 que significa que aún no ha acabado el proceso del char
	TXREG = c;
	return 1; // en caso de que hayamos podido mandar el char, devolvemos 1 y significará que hemos mandado satisfactoriamente el char
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

// Función optimizada para imprimir UID y configuración de LEDs
void printfUID(unsigned char *currentUser, char userIndex, const char* extraString) {
    // Tabla de conversión hexadecimal
    static const char hex[] = "0123456789ABCDEF";
    // Usamos buffer en RAM
    static char buffer[80];  // Buffer más grande para incluir el texto extra
    char *ptr = buffer;
    
    // Si no hay usuario, mostrar mensaje de error
    if (currentUser == 0 || *currentUser == 0) {
        motor_StartSendString("\tNo hay usuario configurado\r\n");
        return;
    }
    
    // Añadir el texto extra si existe
    if (extraString) {
        while (*extraString) {
            *ptr++ = *extraString++;
        }
        // Si el texto extra es "Usuari ", añadir el número
        if (buffer[0] == 'U' && buffer[1] == 's' && buffer[2] == 'u' && 
            buffer[3] == 'a' && buffer[4] == 'r' && buffer[5] == 'i' && 
            buffer[6] == ' ') {
            *ptr++ = '1' + userIndex;
            *ptr++ = ':';
            *ptr++ = ' ';
        }
        *ptr++ = '\r';
        *ptr++ = '\n';
        *ptr++ = '\t';
    }
    
    *ptr++ = 'U';
    *ptr++ = 'I';
    *ptr++ = 'D';
    *ptr++ = ':';
    *ptr++ = ' ';
    
    // Optimización del bucle usando char en lugar de int
    char i;
    for(i = 0; i < 5; i++) {
        unsigned char val = currentUser[i];
        *ptr++ = hex[val >> 4];
        *ptr++ = hex[val & 0x0F];
        if(i < 4) *ptr++ = '-';
    }
    *ptr++ = '\r';
    *ptr++ = '\n';
    *ptr++ = '\t';
    
    unsigned char leds[6];
    getActualLeds(leds, userIndex);
    
    // Imprimimos cada LED y su configuración
    for(i = 0; i < 6; i++) {
        // Formato "L0: X"
        *ptr++ = 'L';
        *ptr++ = '0' + i;
        *ptr++ = ':';
        *ptr++ = ' ';
        *ptr++ = hex[leds[i] & 0x0F];  // Valor hexadecimal
        
        // Añadir separador si no es el último LED
        if(i < 5) {
            *ptr++ = ' ';
            *ptr++ = '-';
            *ptr++ = ' ';
        }
    }
    
    // Finalización de cadena
    *ptr++ = '\r';
    *ptr++ = '\n';
    *ptr = '\0';
    
    // Envío de cadena optimizado
    motor_StartSendString(buffer);
    sending_string = 1;
}


// Motor de terminal optimizado
void motorTerminal(void) {
	static char state = 0;
	static unsigned char hour[4] = "0000";
	static char index = 0;
	static unsigned char leds[6];
	static unsigned char currentUser[5];
	static char userNumber = 0;

	if (sending_string) {
		if (motor_SendString() == 1) {
			sending_string = 0;
		} else {
			return;
		}
	}

	switch(state) {
		case 0:
			if (Terminal_RXAvailable() && Terminal_ReceiveChar() == 0x1B || hashtag_pressed) {
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
				unsigned char currentUserIndex = getCurrentUserIndex();
				if (currentUserIndex != 4) {
					getActualUID(currentUser, currentUserIndex);
					printfUID(currentUser, currentUserIndex, "Usuari ");
					state = 25;
				} else {
					motor_StartSendString("No hi ha cap usuari a la sala.\r\n");
					sending_string = 1;
					state = 25;
				}
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
				motor_StartSendString("Configuracions:\r\n");
				sending_string = 1;
				userNumber = 0;
				state = 32; // Ir directamente al estado 32
			}
			break;
			
		case 32: // Mostrar detalles del usuario actual
			if (!sending_string) {
				if (userNumber < 4) {
					// Obtener UID del usuario actual
					getActualUID(currentUser, userNumber);
					// Mostrar configuración con el título del usuario
					printfUID(currentUser, userNumber, "Usuari ");
					// Avanzar al siguiente usuario
					userNumber++;
				} else {
					motor_StartSendString("\r\n"); // Añadir línea en blanco al final
					sending_string = 1;
					state = 0; // Volver al menú principal cuando acabemos
				}
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