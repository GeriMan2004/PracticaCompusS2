#ifndef TAD_DATOS_H
#define TAD_DATOS_H

// Definición de constantes
#define UID_SIZE 16
#define MAX_USERS 3
#define LEDS 6


unsigned char* getActualUID(void);
unsigned char* getUsersConfigurations(void);
void newUser(void);
void newConfiguration(void);
void motor_datos(void);

#endif /* TAD_DATOS_H */
