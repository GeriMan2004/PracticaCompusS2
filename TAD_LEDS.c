#include <xc.h>
#include "TAD_LEDS.h"
#include "TAD_DATOS.h"
#include "TAD_TIMER.h"

static unsigned char ActualLeds[6];
static unsigned char timer;

void initLeds(void) {
    TRISA = 0x00; 
    TRISE = 0x00;
    TI_NewTimer(&timer);
}

void setLedActual(unsigned char ledActual) {
    switch(ledActual) {
        case 0x00:
            LATAbits.LATA0 = 1;
            break;
        case 0x01:
            LATAbits.LATA1 = 1;
            break;
        case 0x02:
            LATAbits.LATA2 = 1;
            break;
        case 0x03:
            LATAbits.LATA3 = 1;
            break;
        case 0x04:
            LATAbits.LATA4 = 1;
            break;
        case 0x05:
            LATAbits.LATA5 = 1;
            break;
        default: // set all leds to 1
            LATAbits.LATA0 = 1;
            LATAbits.LATA1 = 1;
            LATAbits.LATA2 = 1;
            LATAbits.LATA3 = 1;
            LATAbits.LATA4 = 1;
            LATAbits.LATA5 = 1;
            break;
    }
}

void unsetLedActual(unsigned char ledActual) {
    switch(ledActual) {
        case 0x00:
            LATAbits.LATA0 = 0;
            break;
        case 0x01:
            LATAbits.LATA1 = 0;
            break;
        case 0x02:
            LATAbits.LATA2 = 0;
            break;
        case 0x03:
            LATAbits.LATA3 = 0;
            break;
        case 0x04:
            LATAbits.LATA4 = 0;
            break;
        case 0x05:
            LATAbits.LATA5 = 0;
            break;
        default: // set all leds to 0
            LATAbits.LATA0 = 0;
            LATAbits.LATA1 = 0;
            LATAbits.LATA2 = 0;
            LATAbits.LATA3 = 0;
            LATAbits.LATA4 = 0;
            LATAbits.LATA5 = 0;
    }
}

void motor_LEDs(void) {    
    // Get the current LED intensities
    getActualLeds(ActualLeds);
    
    // At the start of each PWM cycle, turn all LEDs on
    if (TI_GetTics(timer) >= PWM_TIME) {
        TI_ResetTics(timer);
        setLedActual(0xFF);  // Turn all LEDs on
        //negate the value of the led 7 to turn it on and off and calculate the time of the pwm
        //LATEbits.LATE2 ^= 1;
    }
    
    // Check each LED's PWM duty cycle
    for (int i = 0; i < LEDS; i++) {
        if (ActualLeds[i] < 0xA && TI_GetTics(timer) >= ActualLeds[i]) {
            unsetLedActual(i);  // Turn LED off if its duty cycle is complete
        }
    }
}
