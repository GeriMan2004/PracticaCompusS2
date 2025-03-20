#ifndef TAD_TECLADO_H
#define TAD_TECLADO_H
#include "TAD_TIMER.h"

#define REBOTE 4  // We want a debounce time of 20ms, and we get a tick every 5ms, so the GetTicks will be 8
#define HASHTAG_TIME 1500 

// Function prototypes
static unsigned char ReadFilas(void);
void initTeclado(void);
void initPortsTeclado(void);
void motorTeclado(void);
void writeColumnas(void);
unsigned char GetTecla(void);
void showTecla(void);

#endif