#ifndef TAD_DATOS_H
#define TAD_DATOS_H

// Definición de constantes
#define UID_SIZE 5
#define MAX_USERS 3
#define LEDS 6

// Declaración de variables externas
extern unsigned char userUIDs[MAX_USERS][UID_SIZE];
extern unsigned char configurations[MAX_USERS][LEDS];

// Prototipos de funciones
unsigned char getActualUID(void);
unsigned char getUserConfiguration(void);
void setUserConfiguration(unsigned char led[6], unsigned char UID);

#endif /* TAD_DATOS_H */
