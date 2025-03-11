/*
 * File:   TRFID.c
 * Author: jnavarro & ester.vidana
 *
 * 
 * Inspired by: https://simplesoftmx.blogspot.com/2014/11/libreria-para-usar-lector-rfid-rc522.html
 */


#include <xc.h>
#include "TAD_RFID.h"
#include "TAD_DATOS.h"

#define NUM_US 16

//-------------- Private functions: --------------
void InitPortDirections () {
    DIR_MFRC522_SO  = 1; 
    DIR_MFRC522_SI  = 0; 
    DIR_MFRC522_SCK = 0; 
    DIR_MFRC522_CS  = 0; 
    DIR_MFRC522_RST = 0;
}

void delay_us (char howMany) {
    char x;
    for (x = 0; x < howMany * NUM_US; x++) Nop();
}

unsigned char MFRC522_Rd (unsigned char Address) {
    unsigned char i, ucAddr = ((Address<<1) & 0x7E) | 0x80; // Se pueden permitir ser chars, debido a que no se requeire mas de 1 byte
    unsigned char ucResult = 0;

    MFRC522_SCK = 0;
    MFRC522_CS = 0; 

    for (i = 8; i > 0; i--) {
        MFRC522_SI = ((ucAddr & 0x80) == 0x80);
        MFRC522_SCK = 1;
        delay_us(5);
        ucAddr <<= 1;
        MFRC522_SCK = 0;
        delay_us(5);
    }

    for (i = 8; i > 0; i--) {
        MFRC522_SCK = 1;
        delay_us(5);
        ucResult <<= 1;   
        ucResult|= MFRC522_SO;
        MFRC522_SCK = 0;
        delay_us(5);
    }

    MFRC522_CS = 1;
    MFRC522_SCK = 1;
    return ucResult;
}


void MFRC522_Wr (unsigned char Address, unsigned char value) {
    unsigned char i, ucAddr = ((Address << 1) & 0x7E);
    MFRC522_SCK = 0;
    MFRC522_CS = 0;
    for (i = 8; i > 0; i--) {
        MFRC522_SI = ((ucAddr & 0x80) == 0x80);
        MFRC522_SCK = 1;
        ucAddr <<= 1;
        delay_us(5);
        MFRC522_SCK = 0;
        delay_us(5);
    }

    for (i = 8; i > 0; i--) {
        MFRC522_SI = ((value & 0x80) == 0x80);
        MFRC522_SCK = 1;
        value <<= 1;
        delay_us(5);
        MFRC522_SCK = 0;
        delay_us(5);
    }

    MFRC522_CS = 1;
    MFRC522_SCK = 1;
}



void MFRC522_Clear_Bit(char addr, char mask) {     
    MFRC522_Wr(addr, MFRC522_Rd(addr) & ~mask);   
}

void MFRC522_Set_Bit(char addr, char mask) {    
    MFRC522_Wr(addr, MFRC522_Rd(addr) | mask);
}

void MFRC522_Reset () { 
    MFRC522_RST = 1;
    delay_us (1);
    MFRC522_RST = 0;
    delay_us (1);
    MFRC522_RST = 1;
    delay_us (1);
    MFRC522_Wr(COMMANDREG, PCD_RESETPHASE);
    delay_us (1);
}

void MFRC522_AntennaOn () {                                              
    MFRC522_Set_Bit(TXCONTROLREG, 0x03);
}

void MFRC522_AntennaOff () {
    MFRC522_Clear_Bit(TXCONTROLREG, 0x03);                                          
}

void MFRC522_Init() {                 
    MFRC522_CS = 1; 
    MFRC522_RST = 1;

    MFRC522_Reset();       
    MFRC522_Wr(TMODEREG, 0x8D);      //Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    MFRC522_Wr(TPRESCALERREG, 0x3E); //TModeReg[3..0] + TPrescalerReg
    MFRC522_Wr(TRELOADREGL, 30);
    MFRC522_Wr(TRELOADREGH, 0); 
    MFRC522_Wr(TXAUTOREG, 0x40);    //100%ASK
    MFRC522_Wr(MODEREG, 0x3D);      // CRC valor inicial de 0x6363
    
    MFRC522_AntennaOff();            
    MFRC522_AntennaOn();
}



//-------------- Public functions: --------------
void initRFID() {
    InitPortDirections();
    MFRC522_Init(); 
}

void motor_RFID(void) {
    static char state = 0;
    static char substate = 0;
    static char irqEn, waitIRq, n;
    static unsigned char i;
    static char _status;
    static unsigned unLen;
    static char TagType;
    static unsigned char UID[6];
    static unsigned char checksum;
    static unsigned char allZero;
    static unsigned char tempRegValue; // Temporary storage for register values

    switch(state) {
        // Estado 0: Detección de tarjeta (Request)
        case 0:
            switch(substate) {
                case 0:
                    // Configura BITFRAMING y asigna TagType
                    MFRC522_Wr(BITFRAMINGREG, 0x07);
                    TagType = PICC_REQIDL;
                    substate = 1;
                    break;
                case 1:
                    // Configura las interrupciones para el comando TRANSCEIVE
                    irqEn = 0x77;
                    waitIRq = 0x30;
                    MFRC522_Wr(COMMIENREG, irqEn | 0x80);
                    substate = 2;
                    break;
                case 2:
                    // Read COMMIRQREG to prepare for clearing
                    tempRegValue = MFRC522_Rd(COMMIRQREG);
                    substate = 3;
                    break;
                case 3:
                    // Clear the interrupt register
                    MFRC522_Wr(COMMIRQREG, tempRegValue & ~0x80);
                    substate = 4;
                    break;
                case 4:
                    // Read FIFOLEVELREG to prepare for setting
                    tempRegValue = MFRC522_Rd(FIFOLEVELREG);
                    substate = 5;
                    break;
                case 5:
                    // Reinicia el FIFO
                    MFRC522_Wr(FIFOLEVELREG, tempRegValue | 0x80);
                    substate = 6;
                    break;
                case 6:
                    // Pone el lector en modo IDLE
                    MFRC522_Wr(COMMANDREG, PCD_IDLE);
                    substate = 7;
                    break;
                case 7:
                    // Carga el comando (TagType) en el FIFO
                    MFRC522_Wr(FIFODATAREG, TagType);
                    substate = 8;
                    break;
                case 8:
                    // Inicia el comando TRANSCEIVE
                    MFRC522_Wr(COMMANDREG, PCD_TRANSCEIVE);
                    substate = 9;
                    break;
                case 9:
                    // Read BITFRAMINGREG to prepare for setting
                    tempRegValue = MFRC522_Rd(BITFRAMINGREG);
                    substate = 10;
                    break;
                case 10:
                    // Activa StartSend para iniciar la transmisión y reinicia el contador
                    MFRC522_Wr(BITFRAMINGREG, tempRegValue | 0x80);
                    i = 0xFF;
                    substate = 11;
                    break;
                case 11:
                    // Espera la respuesta; se realiza una comprobación en cada llamada
                    n = MFRC522_Rd(COMMIRQREG);
                    if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                        substate = 12;
                    }
                    break;
                case 12:
                    // Read BITFRAMINGREG to prepare for clearing
                    tempRegValue = MFRC522_Rd(BITFRAMINGREG);
                    substate = 13;
                    break;
                case 13:
                    // Desactiva el StartSend
                    MFRC522_Wr(BITFRAMINGREG, tempRegValue & ~0x80);
                    substate = 14;
                    break;
                case 14:
                    // Valida la respuesta leyendo el FIFO y calculando la cantidad de bits recibidos
                    if (i != 0 && !(MFRC522_Rd(ERRORREG) & 0x1B)) {
                        unsigned char fifoLevel = MFRC522_Rd(FIFOLEVELREG);
                        unsigned char lastBitsVal = MFRC522_Rd(CONTROLREG) & 0x07;
                        unsigned backBitsCalc;
                        if (lastBitsVal)
                            backBitsCalc = (fifoLevel - 1) * 8 + lastBitsVal;
                        else
                            backBitsCalc = fifoLevel * 8;
                        // Si se reciben exactamente 16 bits (0x10), la lectura es válida
                        if (backBitsCalc == 0x10) {
                            substate = 15;
                        } else {
                            substate = 16;
                        }
                    } else {
                        substate = 16;
                    }
                    break;
                case 15:
                    // La lectura es válida: pasa al estado de lectura de UID
                    state = 1;
                    substate = 0;
                    break;
                case 16:
                    // Lectura inválida: se reinicia la detección
                    state = 0;
                    substate = 0;
                    break;
            }
            break;
        // Estado 1: Lectura de UID (AntiColisión)
        case 1:
            switch(substate) {
                case 0:
                    // Configura BITFRAMING para la anti-colisión y asigna el comando correspondiente
                    MFRC522_Wr(BITFRAMINGREG, 0x00);
                    UID[0] = PICC_ANTICOLL;
                    UID[1] = 0x20;
                    substate = 1;
                    break;
                case 1:
                    // Read STATUS2REG to prepare for clearing
                    tempRegValue = MFRC522_Rd(STATUS2REG);
                    substate = 2;
                    break;
                case 2:
                    // Limpia el STATUS2 para iniciar el proceso de anti-colisión
                    MFRC522_Wr(STATUS2REG, tempRegValue & ~0x08);
                    substate = 3;
                    break;
                case 3:
                    // Configura las interrupciones para TRANSCEIVE en modo anti-colisión
                    irqEn = 0x77;
                    waitIRq = 0x30;
                    MFRC522_Wr(COMMIENREG, irqEn | 0x80);
                    substate = 4;
                    break;
                case 4:
                    // Read COMMIRQREG to prepare for clearing
                    tempRegValue = MFRC522_Rd(COMMIRQREG);
                    substate = 5;
                    break;
                case 5:
                    // Limpia el registro de interrupciones
                    MFRC522_Wr(COMMIRQREG, tempRegValue & ~0x80);
                    substate = 6;
                    break;
                case 6:
                    // Read FIFOLEVELREG to prepare for setting
                    tempRegValue = MFRC522_Rd(FIFOLEVELREG);
                    substate = 7;
                    break;
                case 7:
                    // Reinicia el FIFO
                    MFRC522_Wr(FIFOLEVELREG, tempRegValue | 0x80);
                    substate = 8;
                    break;
                case 8:
                    // Pone el lector en modo IDLE
                    MFRC522_Wr(COMMANDREG, PCD_IDLE);
                    substate = 9;
                    break;
                case 9:
                    // Carga en el FIFO el comando de anti-colisión
                    MFRC522_Wr(FIFODATAREG, UID[0]);
                    MFRC522_Wr(FIFODATAREG, UID[1]);
                    substate = 10;
                    break;
                case 10:
                    // Inicia el comando TRANSCEIVE para anti-colisión
                    MFRC522_Wr(COMMANDREG, PCD_TRANSCEIVE);
                    substate = 11;
                    break;
                case 11:
                    // Read BITFRAMINGREG to prepare for setting
                    tempRegValue = MFRC522_Rd(BITFRAMINGREG);
                    substate = 12;
                    break;
                case 12:
                    // Activa StartSend y reinicia el contador de espera (timeout reducido)
                    MFRC522_Wr(BITFRAMINGREG, tempRegValue | 0x80);
                    i = 0xFF;
                    substate = 13;
                    break;
                case 13:
                    // Espera la respuesta de anti-colisión
                    n = MFRC522_Rd(COMMIRQREG);
                    if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                        substate = 14;
                    }
                    break;
                case 14:
                    // Read BITFRAMINGREG to prepare for clearing
                    tempRegValue = MFRC522_Rd(BITFRAMINGREG);
                    substate = 15;
                    break;
                case 15:
                    // Desactiva el StartSend
                    MFRC522_Wr(BITFRAMINGREG, tempRegValue & ~0x80);
                    substate = 16;
                    break;
                case 16:
                    // Iniciar validación del UID - Lee el registro de errores
                    if (i != 0 && !(MFRC522_Rd(ERRORREG) & 0x1B)) {
                        // Lee los primeros 2 bytes del UID
                        UID[0] = MFRC522_Rd(FIFODATAREG);
                        UID[1] = MFRC522_Rd(FIFODATAREG);
                        substate = 17;
                    } else {
                        // Error detectado, reinicia el proceso
                        state = 0;
                        substate = 0;
                    }
                    break;
                case 17:
                    // Lee segundo par de bytes del UID
                    UID[2] = MFRC522_Rd(FIFODATAREG);
                    UID[3] = MFRC522_Rd(FIFODATAREG);
                    substate = 18;
                    break;
                case 18:
                    // Lee el byte de checksum y añade terminador
                    UID[4] = MFRC522_Rd(FIFODATAREG);
                    UID[5] = 0; // Terminador nulo
                    substate = 19;
                    break;
                case 19:                    
                    // Calcula el checksum como XOR de los 4 primeros bytes
                    checksum = UID[0] ^ UID[1] ^ UID[2] ^ UID[3];
                    // Inicializa flag para verificar UID no cero
                    allZero = 1;
                    substate = 20;
                    break;
                case 20:
                    // Verifica los primeros 2 bytes del UID por si son ceros
                    if (UID[0] != 0 || UID[1] != 0) {
                        allZero = 0;
                    }
                    substate = 21;
                    break;
                case 21:
                    // Verifica los siguientes 2 bytes del UID por si son ceros
                    if (UID[2] != 0 || UID[3] != 0) {
                        allZero = 0;
                    }
                    substate = 22;
                    break;
                case 22:
                    // Verifica el checksum y si el UID no es todo ceros
                    if (checksum != UID[4] || allZero) {
                        // Si hay error, reinicia
                        state = 0;
                        substate = 0;
                    } else {
                        // Todo correcto, continúa
                        substate = 23;
                    }
                    break;
                case 23:
                    {
                        char differentUID = 1;
                        unsigned char* currentUser = getActualUID();
                        for (int i = 0; i < 5; i++) {
                            if(currentUser[i] != UID[i]) {
                                differentUID = 0;
                                break;
                            }
                        }
                        if(differentUID == 0) {
                            setCurrentUser(UID[0], UID[1], UID[2], UID[3], UID[4]);
                        }
                        substate = 24;
                    }
                    break;
                case 24:
                    // Configuración final y reinicio
                    MFRC522_Wr(BITFRAMINGREG, 0x00);
                    state = 0;
                    substate = 0;
                    break;
                }
        break;
    }
}