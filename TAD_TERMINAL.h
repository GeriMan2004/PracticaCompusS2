#ifndef TAD_TERMINAL_H
#define TAD_TERMINAL_H
#include "TAD_TIMER.h"  

// Function prototypes
void Terminal_Init(void);
int Terminal_TXAvailable(void);
char Terminal_RXAvailable(void);
void Terminal_SendChar(char c);
char Terminal_ReceiveChar(void);
void Terminal_SendString(const char *str);
void showMenu(void);
void hashtag_pressed3s(void);
void motorTerminal(void);


#endif