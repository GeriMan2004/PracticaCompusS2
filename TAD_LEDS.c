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

// Funci�n combinada para manipular LEDs (ahorra c�digo)
void controlLED(unsigned char ledActual, char estado) {
    // Tabla de bits para cada LED
    static unsigned char ledBits[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
    
    if (ledActual < 6) {
        // Asigna estado a un LED espec�fico
        if (estado)
            LATA |= ledBits[ledActual];
        else
            LATA &= ~ledBits[ledActual];
    }
    else if (ledActual == 0xFF) {
        // Asigna estado a todos los LEDs
        LATA = estado ? 0x3F : 0x00;
    }
}

// Macros para mantener compatibilidad con c�digo existente
#define setLedActual(led) controlLED(led, 1)
#define unsetLedActual(led) controlLED(led, 0)

void motor_LEDs(void) {    
    // Get the current LED intensities
    getActualLeds(ActualLeds, 20);
    
    // Optimización: solo continuar si el timer ha cambiado
    static unsigned long lastTics = 0;
    unsigned long currentTics = TI_GetTics(timer);
    
    // Si no ha pasado tiempo, no actualizamos nada
    if (currentTics == lastTics) return;
    lastTics = currentTics;
    
    // At the start of each PWM cycle, turn all LEDs on
    if (currentTics >= PWM_TIME) {
        TI_ResetTics(timer);
        // Solo encender los LEDs que tienen intensidad mayor que 0
        char i;
        for (i = 0; i < LEDS; i++) {
            if (ActualLeds[i] > 0) {
                controlLED(i, 1);
            }
        }
        return;
    }
    
    // Check each LED's PWM duty cycle - recorremos con contador para ahorrar memoria
    char i;
    for (i = 0; i < LEDS; i++) {
        if (ActualLeds[i] < 0xA && currentTics >= ActualLeds[i])
            controlLED(i, 0);  // Turn LED off if its duty cycle is complete
    }
}