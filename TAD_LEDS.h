#ifndef TAD_LEDS_H
#define TAD_LEDS_H

#include <xc.h>

#define MAX_USERS 4
#define LEDS 6      
#define PWM_TIME 10 // 50 ms, 10 tics of the 5ms timer interruption

void initLeds(void);
void setLEDIntensity(unsigned char userIndex, unsigned char ledIndex, unsigned char intensity);
void updateLEDs(void);
void motor_LEDs(void);

#endif 