#include <xc.h>
#include "TAD_DATOS.h"
#include "TAD_DISPLAY.h"
#include "TAD_TERMINAL.h"

// Datos de usuarios y configuraciones
unsigned char userUIDs[MAX_USERS][UID_SIZE] = {
    {0x65, 0xDC, 0xF9, 0x03, 0x43},
    {0xDC, 0x0D, 0xF9, 0x03, 0x2B},
    {0xDF, 0x8B, 0xDF, 0xC4, 0x4F},
    {0x21, 0x32, 0xA9, 0x89, 0x33}
};

unsigned char configurations[MAX_USERS + 1][LEDS];

// Variables de estado
static unsigned char currentUser[5] = {0};
static unsigned char new_configuration = 0;
static unsigned char new_user = 0;
static unsigned char index = 4;  // Inicializar a 20 para indicar que no hay usuario
static unsigned char currentTime[4] = "0000";

// Función optimizada para inicialización
void initData(void) {
    for(char i = 0; i < 5; i++) currentUser[i] = 0;
    for(char i = 0; i < MAX_USERS; i++) {
        for(char j = 0; j < LEDS; j++) {
            configurations[i][j] = i;
        }
    }
    for(char i = 0; i < LEDS; i++) {
        configurations[MAX_USERS][i] = 0;
    }
    index = 4;  // Inicializar a 20 para indicar que no hay usuario
}

void resetData(void) {
    for (unsigned char i = 0; i < MAX_USERS; i++) {
        for (unsigned char j = 0; j < LEDS; j++) {
            configurations[i][j] = 0;
        }
    }
}

void getActualUID(unsigned char* UID, unsigned char userIndex) {
    if(!UID) return;
    if (userIndex == 0xFF) {
        if (currentUser[0]) {
            for(unsigned char i = 0; i < 5; i++) {
                UID[i] = currentUser[i];
            }
        } else {
            for(unsigned char i = 0; i < 5; i++) {
                UID[i] = 0x00;
            }
        }
    } else {
        for(unsigned char i = 0; i < 5; i++) {
            UID[i] = userUIDs[userIndex][i];
        }
    }
}

// Función optimizada para obtener LEDs actuales
void getActualLeds(unsigned char* leds, unsigned char userIndex) {
    if(!leds) return;
    if(userIndex == 20) {
        // Obtener la configuración del usuario actual
        for(char i = 0; i < LEDS; i++) {
            leds[i] = configurations[index][i];
        }
    } else {
        // Usuario Solicitado
        for(char i = 0; i < LEDS; i++) {
            leds[i] = configurations[userIndex][i];
        }
    }
}

// Funciones simples optimizadas
void newConfiguration(void) {
    new_configuration = 1;
}

void saveHourToData(unsigned char hour[4]) {
    if(!hour) return;
    for(char i = 0; i < 4; i++) currentTime[i] = hour[i];
}

void setIndex(unsigned char indexActual) {
    index = indexActual;
}

// Este motor se encarga de establecer el usuario actual y de comprobar cual indice corresponde al usuario actual y establecerlo
char motor_setCurrentUser(char UID0, char UID1, char UID2, char UID3, char UID4) {
    static char state_setCurrentUser = 0;

    switch(state_setCurrentUser) {
        case 0:
            currentUser[0] = UID0;
            currentUser[1] = UID1;
            currentUser[2] = UID2;
            currentUser[3] = UID3;
            currentUser[4] = UID4;
            new_user = 1;
            state_setCurrentUser = 1;
            break;
        case 1:
            // Actualizar el índice directamente sin usar checkUserUID
            if (UID0 == 0x65 && UID1 == 0xDC && UID2 == 0xF9 && UID3 == 0x03 && UID4 == 0x43) {
                index = 0;
                state_setCurrentUser = 6;
                break;
            } 
            state_setCurrentUser = 2;
            break;
        case 2:
            if (UID0 == 0xDC && UID1 == 0x0D && UID2 == 0xF9 && UID3 == 0x03 && UID4 == 0x2B) {
                index = 1;
                state_setCurrentUser = 6;
                break;
            } 
            state_setCurrentUser = 3;
        case 3:
            if (UID0 == 0xDF && UID1 == 0x8B && UID2 == 0xDF && UID3 == 0xC4 && UID4 == 0x4F) {
                index = 2;
                state_setCurrentUser = 6;
                break;
            } 
            state_setCurrentUser = 4;
            break;
        case 4:
            if (UID0 == 0x21 && UID1 == 0x32 && UID2 == 0xA9 && UID3 == 0x89 && UID4 == 0x33) {
                index = 3;
                state_setCurrentUser = 6;
                break;
            } 
            state_setCurrentUser = 5;
            break;
        case 5:
            index = 4;  // Si no coincide con ningún usuario conocido
            state_setCurrentUser = 6;
            break;
        case 6:
            printfUID(currentUser, index, "\r\nTargeta detectada!");
            state_setCurrentUser = 0;
            return 1;
            break;
    }
    return 0;
}

// Motor de datos optimizado
void motor_datos(void) {
    static char state = 0;
    static char pointer = 0;
    static unsigned char lastChar;

    switch(state) {
        case 0:
            if(new_configuration || new_user) {
                new_configuration = new_user = 0;
                state = 1;
            }
            break;

        case 1:
            lastChar = currentUser[4];
            LcPutChar((lastChar < 10) ? ('0' + lastChar) : ('A' + (lastChar - 10)));
            LcPutChar(' ');
            state = 2;
            break;

        case 2:
            for(char i = 0; i < 4; i++) {
                LcPutChar(currentTime[i]);
                if(i == 1) LcPutChar(':');
            }
            LcPutChar(' ');
            pointer = 0;
            state = 3;
            break;

        case 3:
            if(pointer < LEDS) {
                LcPutChar('1' + pointer);
                LcPutChar('-');
                LcPutChar('0' + configurations[index][pointer]);
                LcPutChar(' ');
                pointer++;
            } else {
                pointer = 0;
                state = 0;
            }
            break;
    }
}

// Función optimizada para establecer intensidad de LED
void setLEDIntensity(unsigned char userIndex, unsigned char ledIndex, unsigned char intensity) {
    if(userIndex < MAX_USERS && ledIndex < LEDS && intensity <= 0xA) {
        configurations[userIndex][ledIndex] = intensity;
    }
}

// Función optimizada para establecer LED
void setLed(unsigned char tecla) {
    static char modeLED = 0;
    static char ledIndex = 0;

    // Si index es 4, no hay nadie en la sala, así que no hacemos nada
    if (index == 4) return;

    if(!modeLED) {
        ledIndex = tecla - 1;
        modeLED = 1;
    } else {
        if (index < MAX_USERS) {  // Solo si el índice es válido
            setLEDIntensity(index, ledIndex, tecla);
            new_configuration = 1;
        }
        modeLED = 0;
    }
}

// Función para obtener el índice actual del usuario
unsigned char getCurrentUserIndex(void) {
    return index;
}