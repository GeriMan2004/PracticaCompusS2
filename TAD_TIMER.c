// josepmaria.ribes@salle.url.edu (si hi trobeu alguna errada, si us plau envieu-me un correu :-)
// Arbeca, bressol de l'oliva arbequina
// Mar�, any del Senyor de 2023

// TAD TIMER. Honor i gl�ria


#include <xc.h>
#include "TAD_TIMER.h"

// Definicions, per interrupci� cada 2ms.
#define T0CON_CONFIG 0x82
#define RECARREGA_TMR0 63036        // 2 ms, suposant FOsc a 40MHz.

#define TI_NUMTIMERS 4              // Nombre de timers virtuals gestionats per aquest TAD. Si cal, s'incrementa o es disminueix...

// VARIABLES GLOBALS DEL TAD
struct Timer {
	unsigned long TicsInicials;
	unsigned char Busy;
} static Timers[TI_NUMTIMERS];

static volatile unsigned long Tics=0;

void RSI_Timer0 () {
    // Pre: IMPORTANT! Funci� que ha der ser cridada des de la RSI, en en cas que TMR0IF==1.
    TMR0=RECARREGA_TMR0;
    TMR0IF=0;
    Tics++;    
}

void TI_Init () {
	for (unsigned char counter=0; counter<TI_NUMTIMERS; counter++) {
		Timers[counter].Busy=TI_FALS;
	}
	T0CON=T0CON_CONFIG;
    TMR0=RECARREGA_TMR0;
	INTCONbits.TMR0IF = 0;
	INTCONbits.TMR0IE = 1;
    // Caldr� que des del main o des d'on sigui s'activin les interrupcions globals!
}

unsigned char TI_NewTimer(unsigned char *TimerHandle) {
	unsigned char Comptador=0;
	while (Timers[Comptador].Busy==TI_CERT) {
		if (++Comptador == TI_NUMTIMERS) return (TI_FALS);
	}
	Timers[Comptador].Busy=TI_CERT;
	*TimerHandle=Comptador;
    return (TI_CERT);
}

void TI_ResetTics (unsigned char TimerHandle) {
	// di quiere decir disable interrupts por lo que la siguiente linea de codigo, deshabilita las interrupciones 
	// Luego hace la copia de los tics actuales y luego habilita las interrupciones con ei que significa enable interrupts
	// Las interrupciones se deshabilitan temporalmente para que no se interrumpa durante la copia de los tics actuales y por ende se modifique el valor de los tics
	di(); Timers[TimerHandle].TicsInicials=Tics; ei();
}


unsigned long TI_GetTics (unsigned char TimerHandle) {
    di(); unsigned long CopiaTicsActual=Tics; ei();
	return (CopiaTicsActual-(Timers[TimerHandle].TicsInicials));
}

void TI_CloseTimer (unsigned char TimerHandle) {
	Timers[TimerHandle].Busy=TI_FALS;
}

void TI_End () {
}