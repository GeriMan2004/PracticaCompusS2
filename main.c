#include <xc.h>
#include "TAD_TIMER.h"
#include "TAD_TERMINAL.h"
#include "TAD_RFID.h"
#include "TAD_TECLADO.h"

#pragma config OSC = HSPLL
#pragma config PBADEN = DIG
#pragma config MCLRE = OFF
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
//Definici? d'una interrupci? d'alta prioritat. 
extern void __interrupt (high_priority) HighRSI (void){
    // Assumim que les funcions cridades (RSI_Timer0, en aquest cas), garantiran que el seu IF es reseteja abans de retornar aqu?
    if (INTCONbits.TMR0IF==1) RSI_Timer0();
}
extern void __interrupt (low_priority) LowRSI (void){
    // Assumim que les funcions cridades, garantiran que el seu IF es reseteja abans de retornar aqu?
}


void main(void){
	TI_Init();      // Initialize Timer system
	initTeclado();  // Initialize Keyboard
	initRFID();     // Initialize RFID
	Terminal_Init();  // Initialize Terminal
	initPorts();    // Initialize Ports

	// Enable interrupts
	INTCONbits.GIE = 1;    // Global Interrupt Enable
	INTCONbits.PEIE = 1;   // Peripheral Interrupt Enable
	
	while(1){
		LATA = 0x00;  // Clear PORTA
		motorTeclado();  // Run keyboard state machine
    	motorTerminal();
		motor_RFID();  // Run RFID read public function
		// ReadRFID_NoCooperatiu();
		LATA = 0xFF; 
	}				
}

void initPorts(void){
	ADCON1 = 0xFF;  // Set all pins as digital
	TRISA = 0x00;   // Set all pins as output
}


// Implement the ProcessKey function that was referenced in TAD_TECLADO
void ProcessKey(unsigned char key) {
	// Add your key processing logic here
	// For example, you could:
	LATC = key;  // Display the key value on PORTC (for testing)
}
