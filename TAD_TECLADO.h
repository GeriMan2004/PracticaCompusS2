#ifndef TAD_TECLADO_H
#define TAD_TECLADO_H

#include "TAD_TIMER.h"  // Add this to get TI_FALS and timer functions

#define REBOTE 500  // 8 ticks * 2ms = 16ms debounce time

// Function prototypes
void initTeclado(void);
void motorTeclado(void);
static unsigned char GetTecla(unsigned char filas, unsigned char columnas);
static void StopRequest(unsigned char key);

// Function to process the key (you'll need to implement this in your main)
void ProcessKey(unsigned char key);

#endif