#include <xc.h>
#include "TAD_DATOS.h"
#include "TAD_DISPLAY.h"
#include "TAD_TERMINAL.h"
#include "TAD_TIMER.h"

// Datos de usuarios y configuraciones
unsigned char userUIDs[MAX_USERS + 1][UID_SIZE] = {
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
static unsigned char index = MAX_USERS;  // Inicializar a 20 para indicar que no hay usuario
static unsigned char currentTime[4] = "0000";
static unsigned char timer;
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
    index = MAX_USERS;  // Inicializar a 20 para indicar que no hay usuario
    TI_NewTimer(&timer);
    new_configuration = 1;
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
            index = MAX_USERS;  // Si no coincide con ningún usuario conocido
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
    static char i = 0; // Para las iteraciones
    char c; // Moved variable declaration here for case 13
    // Si el tiempo transcurrido es mayor que el tiempo de espera (1 minuto), actualiza la hora sumandole 1 minuto a esta y resetea el timer para volver a contar 1 minuto
    if (TI_GetTics(timer) > MINUTE_DELAY) {
        new_configuration = 1;
        if (currentTime[3] == '9') {
            currentTime[3] = '0';
            if (currentTime[2] == '5') {
                currentTime[2] = '0';
                if (currentTime[1] == '9') {
                    currentTime[1] = '0';
                    if (currentTime[0] == '2') {
                        currentTime[0] = '0';
                    } else {
                        currentTime[0]++;
                    }
                } else {
                    currentTime[1]++;
                }
            } else {
                currentTime[2]++;
            }
        } else {
            currentTime[3]++;
        }
        TI_ResetTics(timer);
    }
    switch(state) {
        // Estado inicial - verifica si hay que actualizar la pantalla
        case 0:
            if(new_configuration || new_user) {
                new_configuration = new_user = 0;
                state = 2;
            }
            break;

        // Posiciona el cursor en la primera líneaF
        case 2:
            LcGotoXY(0, 0);
            state = 3;
            break;

        // Muestra el último caracter del UID
        case 3:
            lastChar = currentUser[4];
            // Para números (0x03, 0x43)
            if(index != MAX_USERS) {
                if((lastChar & 0x0F) < 10) {
                    LcPutChar('0' + (lastChar & 0x0F));
                } 
                // Para letras (0x2B, 0x4F)
                else {
                    LcPutChar('A' + (lastChar & 0x0F) - 10);
                }
            } else {
                LcPutChar(' ');
            }
            state = 4;
            break;

        // Pone un espacio después del UID
        case 4:
            LcPutChar(' ');
            state = 5;
            i = 0; // Inicializar contador para la hora
            break;

        // Muestra cada dígito de la hora
        case 5:
            if(i < 4) {
                LcPutChar(currentTime[i]);
                i++;
                state = 6;
            } else {
                // Ya terminamos la hora, ir al espacio
                state = 9;
            }
            break;

        // Verifica si hay que poner dos puntos
        case 6:
            if(i == 2) {
                LcPutChar(':');
            }
            state = 5; // Volver a mostrar el siguiente dígito
            break;

        // Pone un espacio después de la hora
        case 9:
            LcPutChar(' ');
            pointer = 0; // Reiniciar para los LEDs
            state = 10;
            break;

        // Verifica si hay que cambiar de línea
        case 10:
            if(pointer < LEDS) {
                if(pointer == 2) {
                    LcGotoXY(0, 1); // Cambiar a la segunda línea
                }
                state = 11;
            } else {
                // Ya terminamos todos los LEDs
                pointer = 0;
                state = 0;
            }
            break;

        case 11:
            LcPutChar('1' + pointer);
            state = 12;
            break;

        case 12:
            LcPutChar('-');
            state = 13;
            break;

        case 13:
            c = '0' + configurations[index][pointer];
            if(c > '9') {
                c = 'A';
            }
            LcPutChar(c);
            state = 14;
            break;

        case 14:
            LcPutChar(' ');
            pointer++;
            state = 10;
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

    // Si index es MAX_USERS, no hay nadie en la sala, así que no hacemos nada
    if (index == MAX_USERS) return;

    if(!modeLED) {
        ledIndex = tecla - 1;
        modeLED = 1;
    } else {
        setLEDIntensity(index, ledIndex, tecla);
        new_configuration = 1;
        modeLED = 0;
    }
}

// Función para obtener el índice actual del usuario
unsigned char getCurrentUserIndex(void) {
    return index;
}