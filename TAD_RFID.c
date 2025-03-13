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
#include "TAD_TERMINAL.h"

#define NUM_US 8 // Reducido de 16 a 8 para delay más corto

// Variables globales para controlar los estados de las funciones motor
char state_read = 0;
char state_write = 0;

//-------------- Private functions: --------------
void InitPortDirections () {
    DIR_MFRC522_SO  = 1; 
    DIR_MFRC522_SI  = 0; 
    DIR_MFRC522_SCK = 0; 
    DIR_MFRC522_CS  = 0; 
    DIR_MFRC522_RST = 0;
}

// Optimización: función delay_us más compacta
void delay_us (char howMany) {
    char x = howMany * NUM_US;
    while(x--) Nop();
}

// Macro para reducir código repetitivo en los bucles de bits
#define PROC_BIT(val, bit_op) do { \
    MFRC522_SI = ((val & 0x80) ? 1 : 0); \
    MFRC522_SCK = 1; \
    bit_op; \
    delay_us(5); \
    MFRC522_SCK = 0; \
    delay_us(5); \
} while(0)

unsigned char MFRC522_Rd (unsigned char Address) {
    unsigned char i, ucAddr = ((Address<<1) & 0x7E) | 0x80;
    unsigned char ucResult = 0;

    MFRC522_SCK = 0;
    MFRC522_CS = 0; 

    // Optimización: uso de macro para reducir código repetido
    for (i = 8; i > 0; i--) {
        PROC_BIT(ucAddr, ucAddr <<= 1);
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
    
    // Optimización: uso de macro para reducir código repetido
    for (i = 8; i > 0; i--) {
        PROC_BIT(ucAddr, ucAddr <<= 1);
    }

    for (i = 8; i > 0; i--) {
        PROC_BIT(value, value <<= 1);
    }

    MFRC522_CS = 1;
    MFRC522_SCK = 1;
}

// Funciones combinadas para ahorrar espacio
void MFRC522_Bit_Mask(char addr, char mask, char op) {
    char temp = MFRC522_Rd(addr);
    MFRC522_Wr(addr, op ? (temp | mask) : (temp & ~mask));
}

// Reemplaza MFRC522_Clear_Bit y MFRC522_Set_Bit con llamadas a MFRC522_Bit_Mask
#define MFRC522_Clear_Bit(addr, mask) MFRC522_Bit_Mask(addr, mask, 0)
#define MFRC522_Set_Bit(addr, mask) MFRC522_Bit_Mask(addr, mask, 1)

void resetMotorStates() {
    state_read = state_write = 0;
    MFRC522_CS = MFRC522_SCK = 1;
}

void MFRC522_Reset () { 
    resetMotorStates();
    
    MFRC522_RST = 1;
    delay_us(1);
    MFRC522_RST = 0;
    delay_us(1);
    MFRC522_RST = 1;
    delay_us(1);
    MFRC522_Wr(COMMANDREG, PCD_RESETPHASE);
    delay_us(1);
}

// Optimización: combinamos funciones similares
void MFRC522_AntennaControl(char on) {
    if(on) 
        MFRC522_Set_Bit(TXCONTROLREG, 0x03);
    else
        MFRC522_Clear_Bit(TXCONTROLREG, 0x03);
}

// Redefinición como macros
#define MFRC522_AntennaOn() MFRC522_AntennaControl(1)
#define MFRC522_AntennaOff() MFRC522_AntennaControl(0)

void MFRC522_Init() {                 
    MFRC522_CS = 1; 
    MFRC522_RST = 1;

    MFRC522_Reset();       
    MFRC522_Wr(TMODEREG, 0x8D);
    MFRC522_Wr(TPRESCALERREG, 0x3E);
    MFRC522_Wr(TRELOADREGL, 30);
    MFRC522_Wr(TRELOADREGH, 0); 
    MFRC522_Wr(TXAUTOREG, 0x40);
    MFRC522_Wr(MODEREG, 0x3D);
    
    MFRC522_AntennaOn();
}

char motor_Write(char addr, char value) {
    static char bit_count = 0;
    static unsigned char ucAddr;
    static unsigned char ucValue;

    switch (state_write) {
        case 0: // Initialization phase
            MFRC522_SCK = 0;
            MFRC522_CS = 0;
            ucAddr = ((addr << 1) & 0x7E);
            ucValue = value;
            bit_count = 0;
            state_write = 1;
            break;

        case 1: // Send address bits
            MFRC522_SI = ((ucAddr & 0x80) != 0);
            MFRC522_SCK = 1;
            ucAddr <<= 1;
            delay_us(5);
            MFRC522_SCK = 0;
            delay_us(5);
            if (++bit_count == 8) {
                bit_count = 0;
                state_write = 2;
            }
            break;

        case 2: // Send value bits
            MFRC522_SI = ((ucValue & 0x80) != 0);
            MFRC522_SCK = 1;
            ucValue <<= 1;
            delay_us(5);
            MFRC522_SCK = 0;
            delay_us(5);
            if (++bit_count == 8) {
                MFRC522_CS = 1;
                MFRC522_SCK = 1;
                state_write = 0;
                return 1;
            }
            break;
    }
    return 0;
}

char motor_Read(char addr) {
    static char bit_count = 0;
    static unsigned char ucAddr;
    static unsigned char ucResult;
    static unsigned int timeout_counter = 0;
    const unsigned int MAX_TIMEOUT = 1000;

    if (++timeout_counter > MAX_TIMEOUT) {
        MFRC522_CS = MFRC522_SCK = 1;
        state_read = 0;
        timeout_counter = 0;
        return 0xFF;
    }

    switch(state_read) {
        case 0: // Initialization
            timeout_counter = 0;
            MFRC522_SCK = 0;
            MFRC522_CS = 0;
            ucAddr = ((addr<<1) & 0x7E) | 0x80;
            ucResult = 0;
            bit_count = 0;
            state_read = 1;
            return 0xFE;
            
        case 1: // Send address bits
            MFRC522_SI = ((ucAddr & 0x80) == 0x80);
            MFRC522_SCK = 1;
            delay_us(5);
            ucAddr <<= 1;
            MFRC522_SCK = 0;
            delay_us(5);
            
            if (++bit_count >= 8) {
                bit_count = 0;
                state_read = 2;
            }
            return 0xFE;
            
        case 2: // Receive data bits
            MFRC522_SCK = 1;
            delay_us(5);
            ucResult = (ucResult << 1) | MFRC522_SO;
            MFRC522_SCK = 0;
            delay_us(5);
            
            if (++bit_count >= 8) {
                MFRC522_CS = MFRC522_SCK = 1;
                state_read = 0;
                timeout_counter = 0;
                return (ucResult == 0xFE || ucResult == 0xFF) ? 0xFD : ucResult;
            }
            return 0xFE;
    }
    return 0xFE;
}

//-------------- Public functions: --------------
void initRFID() {
    InitPortDirections();
    MFRC522_Init(); 
}

// Función optimizada para procesar múltiples casos
void process_substates(char *substate, char flag, char next_state) {
    if (flag != 0 && flag != 0xFE) {
        *substate = next_state;
    }
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
    static unsigned char tempRegValue;
    unsigned char flag = 0;
    static unsigned char lastBitsVal;
    static unsigned char fifoLevel;
    unsigned char backBitsCalc;
    
    switch(state) {
        // Estado 0: Detección de tarjeta (Request)
        case 0:
            switch(substate) {
                case 0:
                    flag = motor_Write(BITFRAMINGREG, 0x07);
                    if (flag != 0){
                        TagType = PICC_REQIDL;
                        substate = 1;
                    }
                    break;
                case 1:
                    irqEn = 0x77;
                    waitIRq = 0x30;
                    flag = motor_Write(COMMIENREG, irqEn | 0x80);
                    if (flag != 0) substate = 2;
                    break;
                case 2:
                    flag = motor_Read(COMMIRQREG);
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 3;
                    }
                    break;
                case 3:
                    flag = motor_Write(COMMIENREG, tempRegValue & ~0x80);
                    if (flag != 0) substate = 4;
                    break;
                case 4: 
                    flag = motor_Read(FIFOLEVELREG);
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 5;
                    }
                    break;
                case 5:
                    flag = motor_Write(FIFOLEVELREG, tempRegValue | 0x80);
                    if (flag != 0) substate = 6;
                    break;
                case 6:
                    flag = motor_Write(COMMANDREG, PCD_IDLE);
                    if (flag != 0) substate = 7;
                    break;
                case 7:
                    flag = motor_Write(FIFODATAREG, TagType);
                    if (flag != 0) substate = 8;
                    break;
                case 8:
                    flag = motor_Write(COMMANDREG, PCD_TRANSCEIVE);
                    if (flag != 0) substate = 9;
                    break;
                case 9:
                    flag = motor_Read(BITFRAMINGREG);
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 10;
                    }
                    break;
                case 10:
                    flag = motor_Write(BITFRAMINGREG, tempRegValue | 0x80);
                    if (flag != 0){
                        i = 0xFF;
                        substate = 11;
                    }
                    break;
                case 11:
                    flag = motor_Read(COMMIRQREG);
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE) {
                        n = flag;
                        if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                            substate = 12;
                        }
                    }
                    break;
                case 12:
                    flag = motor_Read(BITFRAMINGREG);   
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE && flag != 0x00) {
                        substate = 13;
                    }
                    break;
                case 13:
                    flag = motor_Write(BITFRAMINGREG, tempRegValue & ~0x80);
                    if (flag != 0) substate = 14;
                    break;
                case 14:
                    flag = motor_Read(ERRORREG);
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        if (i != 0 && !(tempRegValue & 0x1B)) {
                            substate = 15;
                        } else {
                            state = substate = 0;
                        }
                    }
                    break;
                case 15:
                    flag = motor_Read(FIFOLEVELREG);
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE) {
                        fifoLevel = flag;
                        substate = 16;
                    }
                    break;
                case 16:
                    flag = motor_Read(CONTROLREG);
                    if (flag == 0xFF) {
                        substate = 0;
                    } else if (flag != 0xFE) {
                        lastBitsVal = flag & 0x07;
                        substate = 17;
                    }
                    break;
                case 17:
                    backBitsCalc = lastBitsVal ? (fifoLevel - 1) * 8 + lastBitsVal : fifoLevel * 8;
                    if (backBitsCalc == 0x10) {
                        state = 1;
                        substate = 0;
                    } else {
                        state = substate = 0;
                    }
                    break;
            }
            break;
        // Estado 1: Lectura de UID (AntiColisión)
        case 1:
            switch(substate) {
                case 0:
                    flag = motor_Write(BITFRAMINGREG, 0x00);
                    if (flag != 0){
                        UID[0] = PICC_ANTICOLL;
                        UID[1] = 0x20;
                        substate = 1;
                    }
                    break;
                case 1:
                    flag = motor_Read(STATUS2REG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 2;
                    }
                    break;
                case 2:
                    flag = motor_Write(STATUS2REG, tempRegValue & ~0x08);
                    if (flag != 0) substate = 3;
                    break;
                case 3:
                    irqEn = 0x77;
                    waitIRq = 0x30;
                    flag = motor_Write(COMMIENREG, irqEn | 0x80);
                    if (flag != 0) substate = 4;
                    break;
                case 4:
                    flag = motor_Read(COMMIRQREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 5;
                    }
                    break;
                case 5:
                    flag = motor_Write(COMMIRQREG, tempRegValue & ~0x80);
                    if (flag != 0) substate = 6;
                    break;
                case 6:
                    flag = motor_Read(FIFOLEVELREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 7;
                    }
                    break;
                case 7:
                    flag = motor_Write(FIFOLEVELREG, tempRegValue | 0x80);
                    if (flag != 0) substate = 8;
                    break;
                case 8:
                    flag = motor_Write(COMMANDREG, PCD_IDLE);
                    if (flag != 0) substate = 9;
                    break;
                case 9:
                    flag = motor_Write(FIFODATAREG, UID[0]);
                    if (flag != 0) substate = 10;
                    break;
                case 10:
                    flag = motor_Write(FIFODATAREG, UID[1]);
                    if (flag != 0) substate = 11;
                    break;
                case 11:
                    flag = motor_Write(COMMANDREG, PCD_TRANSCEIVE);
                    if (flag != 0) substate = 12;
                    break;
                case 12:
                    flag = motor_Read(BITFRAMINGREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 13;
                    }
                    break;
                case 13:
                    flag = motor_Write(BITFRAMINGREG, tempRegValue | 0x80);
                    if (flag != 0){
                        i = 0xFF;
                        substate = 14;
                    }
                    break;
                case 14:
                    flag = motor_Read(COMMIRQREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        n = flag;
                        if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                            substate = 15;
                        }
                    }
                    break;
                case 15:
                    flag = motor_Read(BITFRAMINGREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        substate = 16;
                    }
                    break;
                case 16:
                    flag = motor_Write(BITFRAMINGREG, tempRegValue & ~0x80);
                    if (flag != 0) substate = 17;
                    break;
                case 17:
                    flag = motor_Read(ERRORREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        tempRegValue = flag;
                        if (i != 0 && !(tempRegValue & 0x1B)) {
                            substate = 18;
                        } else {
                            state = substate = 0;
                        }
                    }
                    break;
                
                case 18:
                    flag = motor_Read(FIFODATAREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        UID[0] = flag;
                        substate = 19;
                    }
                    break;
                case 19:
                    flag = motor_Read(FIFODATAREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        UID[1] = flag;
                        substate = 20;
                    }
                    break;               
                case 20:
                    flag = motor_Read(FIFODATAREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        UID[2] = flag;
                        substate = 21;
                    }
                    break;
                case 21:
                    flag = motor_Read(FIFODATAREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        UID[3] = flag;
                        substate = 22;
                    }
                    break;
                
                case 22:
                    flag = motor_Read(FIFODATAREG);
                    if (flag == 0xFF) {
                        state = substate = 0;
                    } else if (flag != 0xFE) {
                        UID[4] = flag;
                        UID[5] = 0;
                        substate = 23;
                    }
                    break;
                
                case 23:                    
                    checksum = UID[0] ^ UID[1] ^ UID[2] ^ UID[3];
                    allZero = 1;
                    substate = 24;
                    break;
                
                case 24:
                    // Verificación optimizada: combinamos verificaciones
                    allZero = ((UID[0] | UID[1] | UID[2] | UID[3]) == 0);
                    substate = 26;
                    break;
                
                case 26:
                    if (checksum != UID[4] || allZero) {
                        state = substate = 0;
                    } else {
                        substate = 27;
                    }
                    break;
                
                case 27:
                    {
                        char differentUID = 1;
                        unsigned char currentUser[5];
                        getActualUID(currentUser);
                        // Optimizado: solo verificamos el primer byte para detección rápida
                        if(currentUser[0] != UID[0]) {
                            setCurrentUser(UID[0], UID[1], UID[2], UID[3], UID[4]);
                        }
                        substate = 28;
                    }
                    break;
                
                case 28:
                    flag = motor_Write(BITFRAMINGREG, 0x00);
                    if (flag != 0) {
                        state = substate = 0;
                    }
                    break;
            }
            break;
    }
}