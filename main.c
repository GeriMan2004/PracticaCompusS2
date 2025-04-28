#include <xc.h>
#include "TAD_LEDS.h"
#include "TAD_TIMER.h"
#include "TAD_TERMINAL.h"
#include "TAD_RFID.h"
#include "TAD_TECLADO.h"
#include "TAD_DATOS.h"
#include "TAD_DISPLAY.h"

#pragma config OSC = HSPLL
#pragma config PBADEN = DIG
#pragma config MCLRE = ON
#pragma config DEBUG = OFF
#pragma config PWRT = OFF
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF

#pragma config PWRT = OFF
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF

void main(void);
void initPorts(void);

//Important: NO es poden cridar les funcions d'interrupcions des del codi
//ja que les seves funcions de retorn en asm s?n diferents.
//Definició d'una interrupció d'alta prioritat. 
extern void __interrupt (high_priority) HighRSI (void){
    // Assumim que les funcions cridades (RSI_Timer0, en aquest cas), garantiran que el seu IF es reseteja abans de retornar aquí
    if (INTCONbits.TMR0IF==1) RSI_Timer0();
}
extern void __interrupt (low_priority) LowRSI (void){
    // Assumim que les funcions cridades, garantiran que el seu IF es reseteja abans de retornar aquí
}


void main(void){
	// Enable interrupts
	INTCONbits.GIE = 1;    // Global Interrupt Enable
	INTCONbits.PEIE = 0;   // Peripheral Interrupt Enable

	// Initialize modules
	TI_Init();        // Initialize Timer system
	initTeclado();    // Initialize Keyboard
	initRFID();       // Initialize RFID
	Terminal_Init();  // Initialize Terminal
	initPorts();      // Initialize Ports
	initLeds();       // Initialize Leds
	LcInit(2, 16);    // Initialize LCD
	initData();       // Initialize Datos
	
	while(1){
		motorTeclado();  // Run keyboard state machine
    	motorTerminal(); // Run terminal state machine
	    motor_RFID();    // Run RFID state machine
		motor_LEDs();    // Run LEDs state machine
        motor_datos();   // Run Datos state machine
		LATEbits.LATE2 ^= 1;
	}				
}

void initPorts(void){
	ADCON1 = 0x0F;  // Set all pins as digital
	TRISEbits.TRISE2 = 0;
}