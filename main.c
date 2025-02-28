#include <xc.h>
#include "TAD_TIMER.h"
#include "TAD_TERMINAL.h"
#include "TAD_RFID.h"
#include "TAD_TECLADO.h"

#pragma config OSC = HS
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
void InitPorts(void);

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
	InitPorts();
	TI_Init();      // Initialize Timer system
	initTeclado();  // Initialize Keyboard
	initRFID();     // Initialize RFID
	Terminal_Init();  // Initialize Terminal

	// Enable interrupts
	INTCONbits.GIE = 1;    // Global Interrupt Enable
	INTCONbits.PEIE = 1;   // Peripheral Interrupt Enable
	
	while(1){
		motorTeclado();  // Run keyboard state machine
		ReadRFID_NoCooperatiu();  // Run RFID read public function
	}				
}


void InitPorts(void) {
	// Configure all analog capable pins as digital
	ADCON1 = 0x0F;
	
	// Configure rows (RA0-RA3) as inputs with pull-ups
	TRISA |= 0x0F;      // RA0-RA3 como entradas
	
	INTCON2bits.RBPU = 0;
	
	// Configure columns (RB0-RB2) as outputs
	TRISB &= 0xF8;
	LATB |= 0x00;    // Set columns high initially
	
	// Configure PORTD for output (display)
	TRISD = 0x00;
	LATD = 0x00;
}

// Implement the ProcessKey function that was referenced in TAD_TECLADO
void ProcessKey(unsigned char key) {
	// Add your key processing logic here
	// For example, you could:
	LATC = key;  // Display the key value on PORTC (for testing)
}
