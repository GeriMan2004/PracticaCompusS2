#ifndef TAD_LEDS_H
#define TAD_LEDS_H

#include <xc.h>

#define LEDS 6      
#define PWM_TIME 10 // 20 ms, 10 tics of the 2ms timer interruption

void initLeds(void);
void setLEDIntensity(unsigned char userIndex, unsigned char ledIndex, unsigned char intensity);
void motor_LEDs(void);

#endif 