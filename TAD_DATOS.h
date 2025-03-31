#ifndef TAD_DATOS_H
#define TAD_DATOS_H

#define UID_SIZE 5
#define MAX_USERS 4
#define LEDS 6

void initData(void);
void resetData(void);
void setLed(unsigned char tecla);
void setIndex(unsigned char index);
void getActualUID(unsigned char* UID, unsigned char userIndex);
void getActualLeds(unsigned char* leds, unsigned char userIndex);
unsigned char getUserUID(unsigned char userIndex, unsigned char bytePos);
void showAllConfigurations(void);
char motor_setCurrentUser(char UID0, char UID1, char UID2, char UID3, char UID4);
void newConfiguration(void);
void saveHourToData(unsigned char hour[4]);
void motor_datos(void);
unsigned char getCurrentUserIndex(void);

#endif 

