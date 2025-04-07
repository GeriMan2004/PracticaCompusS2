#ifndef TAD_TECLADO_H
#define TAD_TECLADO_H

#define REBOTE 4          // Tiempo de rebote (4 tics * 2ms = 8ms)
#define HASHTAG_TIME 1500 // Tiempo para detectar pulsación larga de hashtag (1500 tics * 2ms = 3s)

void initTeclado(void);
void motorTeclado(void);
void writeColumnas(void);
unsigned char GetTecla(void);

#endif