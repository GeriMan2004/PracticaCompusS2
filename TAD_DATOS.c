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

unsigned char configurations[MAX_USERS][LEDS] = { 
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1}
};

// Variables de estado
static unsigned char currentUser[5] = {0};
static unsigned char new_configuration = 0;
static unsigned char new_user = 0;
static unsigned char index = 0;
static unsigned char currentTime[4] = "0000";

// Función optimizada para inicialización
void initData(void) {
    for(char i = 0; i < 5; i++) currentUser[i] = 0;
}

void getActualUID(unsigned char* UID) {
    if(!UID) return;
    
    // Verificación rápida del primer byte
    if(!currentUser[0]) {
        UID[0] = 0x00;
        UID[1] = 0x00;
        UID[2] = 0x00;
        UID[3] = 0x00;
        UID[4] = 0x00;
        return;
    }
    
    // Copia optimizada usando un bucle
    for(char i = 0; i < 5; i++) {
        UID[i] = currentUser[i];
    }
}

// Función optimizada para obtener LEDs actuales
void getActualLeds(unsigned char* leds) {
    if(!leds) return;
    
    // Copia directa de la configuración actual
    for(char i = 0; i < LEDS; i++) {
        leds[i] = configurations[index][i];
    }
}

// Función optimizada para mostrar configuraciones
void showAllConfigurations(void) {
    static const char userStr[] = "User ";
    static const char configStr[] = " Config: ";
    
    for(char i = 0; i < MAX_USERS; i++) {
        Terminal_SendString(userStr);
        Terminal_SendChar('1' + i);
        Terminal_SendString(configStr);
        
        for(char j = 0; j < LEDS; j++) {
            Terminal_SendChar('0' + configurations[i][j]);
            Terminal_SendString(" ");
        }
        Terminal_SendString("\r\n");
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

// Función optimizada para verificar UID de usuario
char checkUserUID(void) {
    // Verificación rápida del primer byte
    if(!currentUser[0]) return 0;
    
    // Comparación optimizada usando un bucle
    for(char i = 0; i < MAX_USERS; i++) {
        char match = 1;
        for(char j = 0; j < 5; j++) {
            if(currentUser[j] != userUIDs[i][j]) {
                match = 0;
                break;
            }
        }
        if(match) return i;
    }
    return 0;
}

// Función optimizada para establecer usuario actual
void setCurrentUser(char UID0, char UID1, char UID2, char UID3, char UID4) {
    currentUser[0] = UID0;
    currentUser[1] = UID1;
    currentUser[2] = UID2;
    currentUser[3] = UID3;
    currentUser[4] = UID4;
    new_user = 1;
    index = checkUserUID();
    printfUID(currentUser);
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
    static char userIndex = 0;

    if(!modeLED) {
        ledIndex = tecla - 1;
        modeLED = 1;
    } else {
        userIndex = checkUserUID();
        setLEDIntensity(userIndex, ledIndex, tecla);
        new_configuration = 1;
        modeLED = 0;
    }
}