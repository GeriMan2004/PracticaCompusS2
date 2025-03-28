#ifndef TAD_TECLADO_H
#define TAD_TECLADO_H

#define REBOTE 4          // Tiempo de rebote (4 tics � 5ms = 20ms)
#define HASHTAG_TIME 1500 // Tiempo para detectar pulsaci�n larga de hashtag

// Prototipos de funci�n
void initTeclado(void);
void motorTeclado(void);
void writeColumnas(void);
unsigned char GetTecla(void);

#endif