#ifndef TAD_TERMINAL_H
#define TAD_TERMINAL_H

#define MAX_LEDS 6
// Function prototypes
void Terminal_Init(void);
int Terminal_TXAvailable(void);
char Terminal_RXAvailable(void);
void Terminal_SendChar(char c);
char Terminal_ReceiveChar(void);
void Terminal_SendString(const char *str);
void printfUID(unsigned char *currentUser, char userIndex, const char* extraString);
void printLedConfig(unsigned char *leds);
void showMenu(void);
void hashtag_pressed3s(void);
void motorTerminal(void);

// Funciones cooperativas
char motor_SendChar(char c);
char motor_SendString(void);
void motor_StartSendString(const char* str);

#endif