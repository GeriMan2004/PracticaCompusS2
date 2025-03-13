#ifndef TAD_DATOS_H
#define TAD_DATOS_H

#define UID_SIZE 16
#define MAX_USERS 4
#define LEDS 6

void initData(void);
void setLed(unsigned char tecla);
void getActualUID(unsigned char* UID);
void getActualLeds(unsigned char* leds);
void showAllConfigurations(void);
void setCurrentUser(char UID0, char UID1, char UID2, char UID3, char UID4);
void newConfiguration(void);
void saveHourToData(unsigned char hour[4]);
void motor_datos(void);

#endif 
