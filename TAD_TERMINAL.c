#include <xc.h>
#include "TAD_TERMINAL.h"

static unsigned char state = 0;
static unsigned char timer;


static unsigned char ReadFilas(void) {
    return (PORTA & 0x0F);  // Read only the lower 4 bits
}

void initTerminal(void) {
    state = 0;
    TI_NewTimer(&timer);
}

void motorTerminal(void) {
	switch(state) {
		case 0:
			if (TXSTAbits.TRMT == EMPTY) {
				TXREG = a;
				state = 1;
			}
		break;
		case 1:
			if (PIR1bits.RCIF == 1) {
				rebut == RCREG;
				state = 2;
			}
		break;
		case 2:
			if (a == rebut) {
				LATAbits.LATA3 = 1;
				a++;
				state = 0;
			}
			else if (a != rebut) {
				LATAbits.LATA3 = 0;
				a++;
				state = 0;
			}
		break;
	}
}