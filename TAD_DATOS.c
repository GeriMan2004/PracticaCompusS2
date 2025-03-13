#include <xc.h>
#include "TAD_DATOS.h"
#include "TAD_DISPLAY.h"
#include "TAD_TERMINAL.h"

unsigned char userUIDs[MAX_USERS][UID_SIZE] = {
    {0x65, 0xDC, 0xF9, 0x03, 0x43},
    {0xDC, 0x0D, 0xF9, 0x03, 0x2B},
    {0xDF, 0x8B, 0xDF, 0xC4, 0x4F},
	{0x21, 0x32, 0xA9, 0x89, 0x33}
};

unsigned char configurations[MAX_USERS][LEDS] = { 
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1}
};

unsigned char currentUser[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char new_configuration = 0;
unsigned char new_user = 0;
int index = 0, pointer = 0;
unsigned char currentTime[4] = "0000";



unsigned char* getActualUID(void) {
	if(currentUser[0] == 0x00 && currentUser[1] == 0x00 && currentUser[2] == 0x00 && currentUser[3] == 0x00 && currentUser[4] == 0x00) {
		return NULL;
	}
    return currentUser;
}

void getActualLeds(unsigned char* leds) {
    for(int i = 0; i < LEDS; i++) {
        leds[i] = configurations[index][i];
    }
}

void showAllConfigurations(void) {
    for (int i = 0; i < MAX_USERS; i++) {
        Terminal_SendString("User ");
        Terminal_SendChar('1' + i);  // Convert user number to character
        Terminal_SendString(" Config: ");
        
        for (int j = 0; j < LEDS; j++) {
            // Convert 0/1 directly to character
            Terminal_SendChar('0' + configurations[i][j]);
            Terminal_SendString(" ");
        }
        Terminal_SendString("\r\n");
    }
}


void newConfiguration(void) {
    new_configuration = 1;
}

void saveHourToData(unsigned char hour[4]) {
    currentTime[0] = hour[0];
    currentTime[1] = hour[1];
    currentTime[2] = hour[2];
    currentTime[3] = hour[3];
}

void setCurrentUser(char UID0, char UID1, char UID2, char UID3, char UID4) {
	currentUser[0] = UID0;
	currentUser[1] = UID1;
	currentUser[2] = UID2;
	currentUser[3] = UID3;
	currentUser[4] = UID4;
	new_user = 1;
	Terminal_SendString("Targeta detectada!\r\n\t");
	printfUID(currentUser);
	Terminal_SendString("\t");
	printLedConfig(configurations[index]);
}

char checkUserUID(void) {
    if (currentUser[0] == userUIDs[0][0] && currentUser[1] == userUIDs[0][1] &&
        currentUser[2] == userUIDs[0][2] && currentUser[3] == userUIDs[0][3] &&
        currentUser[4] == userUIDs[0][4]) {
        return 0;
    } 
    else if (currentUser[0] == userUIDs[1][0] && currentUser[1] == userUIDs[1][1] &&
             currentUser[2] == userUIDs[1][2] && currentUser[3] == userUIDs[1][3] &&
             currentUser[4] == userUIDs[1][4]) {
        return 1;
    } 
    else if (currentUser[0] == userUIDs[2][0] && currentUser[1] == userUIDs[2][1] &&
             currentUser[2] == userUIDs[2][2] && currentUser[3] == userUIDs[2][3] &&
             currentUser[4] == userUIDs[2][4]) {
        return 2;
    } 
    else if (currentUser[0] == userUIDs[3][0] && currentUser[1] == userUIDs[3][1] &&
             currentUser[2] == userUIDs[3][2] && currentUser[3] == userUIDs[3][3] &&
             currentUser[4] == userUIDs[3][4]) {
    	return 3;
    } 
	return 0;
}

void motor_datos(void) {
    static char state = 0;
    static char pointer = 0; // Contador para imprimir LEDs
    unsigned char lastChar;

    switch(state) {
        case 0:
            // Esperamos a que haya un nuevo usuario o una nueva configuración
            if (new_configuration == 1 || new_user == 1) {
                new_configuration = 0;
                new_user = 0;

                // Averiguamos qué usuario está activo según la UID
                index = checkUserUID();  

                state = 1;
            }
            break;

        case 1:
            // 1) Imprimimos el último byte de la UID en hexadecimal (nibble alto/bajo).
            //    Si el valor es <10, lo mostramos como dígito, si no, como A-F.
            lastChar = currentUser[4];
            LcPutChar((lastChar < 10) ? ('0' + lastChar) : ('A' + (lastChar - 10)));
            
            // Espacio separador
            LcPutChar(' ');

            // Pasamos al siguiente estado para imprimir la hora
            state = 2;
            break;

        case 2:
            // 2) Imprimimos la hora en formato HH:MM
            //    currentTime[0] y [1] → Horas
            //    currentTime[2] y [3] → Minutos
            LcPutChar(currentTime[0]); 
            LcPutChar(currentTime[1]);
            LcPutChar(':');
            LcPutChar(currentTime[2]);
            LcPutChar(currentTime[3]);
            
            // Espacio separador
            LcPutChar(' ');

            // Preparamos el contador de LEDs
            pointer = 0;
            state = 3;
            break;

        case 3:
            // 3) Imprimimos el estado de los 6 LEDs, uno por llamada:
            //    Formato "1-x 2-y 3-z 4-w 5-a 6-b"
            if (pointer < LEDS) {
                // (pointer+1) es el número de LED en ASCII
                LcPutChar((char)('1' + pointer));
                LcPutChar('-');
				Terminal_SendChar((char)('1' + pointer));
				Terminal_SendChar('-');
                // Convertimos el valor (0..9) a su correspondiente dígito ASCII
                LcPutChar((char)('0' + configurations[index][pointer]));
                LcPutChar(' ');
				Terminal_SendChar((char)('0' + configurations[index][pointer]));
				Terminal_SendChar(' ');
                pointer++;
            } else {
                // Hemos terminado de imprimir todos los LEDs
                pointer = 0;
                state = 0; // Volvemos al estado de espera
            }
            break;
    }
}


void setLEDIntensity(unsigned char userIndex, unsigned char ledIndex, unsigned char intensity) {
    if (userIndex < MAX_USERS && ledIndex < LEDS) {
        if (intensity <= 0xA) { 
            configurations[userIndex][ledIndex] = intensity;
        }
    }
}

void setLed(unsigned char tecla){
	static char modeLED = 0;
	static char ledIndex = 0;
	static char	userIndex = 0;

	if(modeLED == 0){
		ledIndex = tecla - 1;
		modeLED	= 1;
	} else{
		userIndex = checkUserUID();
		setLEDIntensity(userIndex, ledIndex, tecla);
		new_configuration = 1;
		modeLED = 0;
	}

}



