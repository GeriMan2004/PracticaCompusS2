#ifndef TAD_TECLADO_H
#define TAD_TECLADO_H

#include "TAD_TIMER.h"  

#define REBOTE 8  // We want a debounce time of 16ms, and we get a tick every 2ms, so the GetTicks will be 8
#define HASHTAG_TIME 1500 

// Function prototypes
void initTeclado(void);
void motorTeclado(void);
unsigned char GetTecla(void);
void showTecla(void);

#endif