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
#include "TAD_TIMER.h"  // Añadir inclusión del TAD_TIMER

#define NUM_US 8 // Reducido de 16 a 8 para delay más corto
#define CARD_TIMEOUT 200 // 200 tics = 0.4 segundo

// Variables globales para controlar los estados de las funciones motor
static char state_read = 0;
static char state_write = 0;
static unsigned char card_timer;

//-------------- Private functions: --------------

// Optimización: función delay_us más compacta
void delay_us (char howMany) {
    char x = howMany * NUM_US;
    while(x--) Nop();
}

// Consolidate repetitive bit manipulation into a function
void processBit(unsigned char *val, char *bit_count, char *state, char next_state) {
    MFRC522_SI = ((*val & 0x80) != 0);
    MFRC522_SCK = 1;
    *val <<= 1;
    delay_us(5);
    MFRC522_SCK = 0;
    delay_us(5);
    if (++(*bit_count) == 8) {
        *bit_count = 0;
        *state = next_state;
    }
}

// Use the new function in motor_Write
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
            processBit(&ucAddr, &bit_count, &state_write, 2);
            break;

        case 2: // Send value bits
            processBit(&ucValue, &bit_count, &state_write, 0);
            if (state_write == 0) {
                MFRC522_CS = 1;
                MFRC522_SCK = 1;
                return 1;
            }
            break;
    }
    return 0;
}

// Use the new function in motor_Read
char motor_Read(char addr) {
    static char bit_count = 0;
    static unsigned char ucAddr;
    static unsigned char ucResult;
    static unsigned int timeout_counter = 0;
    static unsigned int MAX_TIMEOUT = 1000;

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
            processBit(&ucAddr, &bit_count, &state_read, 2);
            return 0xFE;
            
        case 2: // Receive data bits
            MFRC522_SCK = 1;
            delay_us(5);
            ucResult = (unsigned char)((ucResult << 1) | MFRC522_SO);
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
    // Set up port directions
    DIR_MFRC522_SO  = 1; 
    DIR_MFRC522_SI  = 0; 
    DIR_MFRC522_SCK = 0; 
    DIR_MFRC522_CS  = 0; 
    DIR_MFRC522_RST = 0;
    // Hardware reset sequence:
    state_read = state_write = 0;
    MFRC522_CS = MFRC522_SCK = 1;     // Resets state and sets MFRC522_CS & MFRC522_SCK high
    MFRC522_RST = 1;
    delay_us(1);
    MFRC522_RST = 0;
    delay_us(1);
    MFRC522_RST = 1;
    delay_us(1);
    
    // Issue reset command: Write PCD_RESETPHASE to COMMANDREG
    while (!motor_Write(COMMANDREG, PCD_RESETPHASE)) { }
    delay_us(1);
    
    // Configure the timer and modulation registers
    while (!motor_Write(TMODEREG, 0x8D)) { }
    while (!motor_Write(TPRESCALERREG, 0x3E)) { }
    while (!motor_Write(TRELOADREGL, 30)) { }
    while (!motor_Write(TRELOADREGH, 0)) { }
    while (!motor_Write(TXAUTOREG, 0x40)) { }
    while (!motor_Write(MODEREG, 0x3D)) { }
    
    // Turn on the antenna by setting bits 0x03 in TXCONTROLREG:
    unsigned char regVal;
    do {
        regVal = motor_Read(TXCONTROLREG);
    } while (regVal == 0xFE); // wait until a valid value is returned
    regVal |= 0x03;
    while (!motor_Write(TXCONTROLREG, regVal)) { }
    
    // Inicializar el timer para detección de tarjeta removida
    TI_NewTimer(&card_timer);
    TI_ResetTics(card_timer);
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
    static unsigned char lastBitsVal;
    static unsigned char fifoLevel;
    static unsigned char backBitsCalc;
    static unsigned char currentUser[5];
    static unsigned char cardRemoved = 0;
    static unsigned char last_state = 0;
    
    // Variables para control de operaciones
    static char operation_pending = 0;  // 0: ninguna, 1: lectura, 2: escritura
    static unsigned char addr;         // Dirección para leer o escribir
    static unsigned char value;        // Valor para escribir
    unsigned char flag = 0;

    // Primero manejar operaciones pendientes
    if (operation_pending == 1) {
        // Lectura pendiente
        flag = motor_Read(addr);
        if (flag != 0xFE) {
            tempRegValue = flag;
            operation_pending = 0;
        } else {
            return; // Seguimos esperando
        }
    } else if (operation_pending == 2) {
        // Escritura pendiente
        flag = motor_Write(addr, value);
        if (flag != 0) {
            operation_pending = 0;
        } else {
            return; // Seguimos esperando
        }
    } else {
        // Comprobar si ha cambiado el estado
        if (state != last_state) {
            last_state = state;
            // Si entramos al estado 1, resetear el timer 
            // (significa que se ha detectado tarjeta)
            if (state == 1) {
                TI_ResetTics(card_timer);
            }
        }
        
        // Verificar si ha pasado el timeout sin entrar al estado 1
        // (significa que la tarjeta ha sido retirada)
        if (state == 0 && TI_GetTics(card_timer) > CARD_TIMEOUT) {
            cardRemoved = 1;
            TI_ResetTics(card_timer);
        }
        
        switch(state) {
            // Estado 0: Detección de tarjeta (Request)
            case 0:
                switch(substate) {
                    case 0:
                        addr = BITFRAMINGREG;
                        value = 0x07;
                        operation_pending = 2;
                        substate = 1;
                        break;
                    case 1:
                        TagType = PICC_REQIDL;
                        irqEn = 0x77;
                        waitIRq = 0x30;
                        addr = COMMIENREG;
                        value = irqEn | 0x80;
                        operation_pending = 2;
                        substate = 2;
                        break;
                    case 2:
                        addr = COMMIRQREG;
                        operation_pending = 1;
                        substate = 3;
                        break;
                    case 3:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else {
                            addr = COMMIENREG;
                            value = tempRegValue & ~0x80;
                            operation_pending = 2;
                            substate = 4;
                        }
                        break;
                    case 4:
                        addr = FIFOLEVELREG;
                        operation_pending = 1;
                        substate = 5;
                        break;
                    case 5:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else {
                            addr = FIFOLEVELREG;
                            value = tempRegValue | 0x80;
                            operation_pending = 2;
                            substate = 6;
                        }
                        break;
                    case 6:
                        addr = COMMANDREG;
                        value = PCD_IDLE;
                        operation_pending = 2;
                        substate = 7;
                        break;
                    case 7:
                        addr = FIFODATAREG;
                        value = TagType;
                        operation_pending = 2;
                        substate = 8;
                        break;
                    case 8:
                        addr = COMMANDREG;
                        value = PCD_TRANSCEIVE;
                        operation_pending = 2;
                        substate = 9;
                        break;
                    case 9:
                        addr = BITFRAMINGREG;
                        operation_pending = 1;
                        substate = 10;
                        break;
                    case 10:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else {
                            addr = BITFRAMINGREG;
                            value = tempRegValue | 0x80;
                            operation_pending = 2;
                            substate = 11;
                        }
                        break;
                    case 11:
                        i = 0xFF;
                        addr = COMMIRQREG;
                        operation_pending = 1;
                        substate = 12;
                        break;
                    case 12:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else {
                            n = tempRegValue;
                            if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                                addr = BITFRAMINGREG;
                                operation_pending = 1;
                                substate = 13;
                            } else {
                                addr = COMMIRQREG;
                                operation_pending = 1;
                            }
                        }
                        break;
                    case 13:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else if (tempRegValue != 0x00) {
                            addr = BITFRAMINGREG;
                            value = tempRegValue & ~0x80;
                            operation_pending = 2;
                            substate = 14;
                        } else {
                            addr = BITFRAMINGREG;
                            operation_pending = 1;
                            // Mantenemos mismo substate para seguir comprobando
                        }
                        break;
                    case 14:
                        addr = ERRORREG;
                        operation_pending = 1;
                        substate = 15;
                        break;
                    case 15:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else if (i != 0 && !(tempRegValue & 0x1B)) {
                            addr = FIFOLEVELREG;
                            operation_pending = 1;
                            substate = 16;
                        } else {
                            
                            state = substate = 0;
                        }
                        break;
                    case 16:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else {
                            fifoLevel = tempRegValue;
                            addr = CONTROLREG;
                            operation_pending = 1;
                            substate = 17;
                        }
                        break;
                    case 17:
                        if (tempRegValue == 0xFF) {
                            substate = 0;
                        } else {
                            lastBitsVal = tempRegValue & 0x07;
                            backBitsCalc = lastBitsVal ? (fifoLevel - 1) * 8 + lastBitsVal : fifoLevel * 8;
                            if (backBitsCalc == 0x10) {
                                state = 1;
                                substate = 0;
                            } else {
                                state = substate = 0;
                            }
                        }
                        break;
                }
                break;
            // Estado 1: Lectura de UID (AntiColisión)
            case 1:
                switch(substate) {
                    case 0:
                        addr = BITFRAMINGREG;
                        value = 0x00;
                        operation_pending = 2;
                        substate = 1;
                        break;
                    case 1:
                        UID[0] = PICC_ANTICOLL;
                        UID[1] = 0x20;
                        addr = STATUS2REG;
                        operation_pending = 1;
                        substate = 2;
                        break;
                    case 2:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            addr = STATUS2REG;
                            value = tempRegValue & ~0x08;
                            operation_pending = 2;
                            substate = 3;
                        }
                        break;
                    case 3:
                        irqEn = 0x77;
                        waitIRq = 0x30;
                        addr = COMMIENREG;
                        value = irqEn | 0x80;
                        operation_pending = 2;
                        substate = 4;
                        break;
                    case 4:
                        addr = COMMIRQREG;
                        operation_pending = 1;
                        substate = 5;
                        break;
                    case 5:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            addr = COMMIRQREG;
                            value = tempRegValue & ~0x80;
                            operation_pending = 2;
                            substate = 6;
                        }
                        break;
                    case 6:
                        addr = FIFOLEVELREG;
                        operation_pending = 1;
                        substate = 7;
                        break;
                    case 7:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            addr = FIFOLEVELREG;
                            value = tempRegValue | 0x80;
                            operation_pending = 2;
                            substate = 8;
                        }
                        break;
                    case 8:
                        addr = COMMANDREG;
                        value = PCD_IDLE;
                        operation_pending = 2;
                        substate = 9;
                        break;
                    case 9:
                        addr = FIFODATAREG;
                        value = UID[0];
                        operation_pending = 2;
                        substate = 10;
                        break;
                    case 10:
                        addr = FIFODATAREG;
                        value = UID[1];
                        operation_pending = 2;
                        substate = 11;
                        break;
                    case 11:
                        addr = COMMANDREG;
                        value = PCD_TRANSCEIVE;
                        operation_pending = 2;
                        substate = 12;
                        break;
                    case 12:
                        addr = BITFRAMINGREG;
                        operation_pending = 1;
                        substate = 13;
                        break;
                    case 13:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            addr = BITFRAMINGREG;
                            value = tempRegValue | 0x80;
                            operation_pending = 2;
                            substate = 14;
                        }
                        break;
                    case 14:
                        i = 0xFF;
                        addr = COMMIRQREG;
                        operation_pending = 1;
                        substate = 15;
                        break;
                    case 15:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            n = tempRegValue;
                            if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                                addr = BITFRAMINGREG;
                                operation_pending = 1;
                                substate = 16;
                            } else {
                                addr = COMMIRQREG;
                                operation_pending = 1;
                                // Mantenemos mismo substate para seguir comprobando
                            }
                        }
                        break;
                    case 16:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            addr = BITFRAMINGREG;
                            value = tempRegValue & ~0x80;
                            operation_pending = 2;
                            substate = 17;
                        }
                        break;
                    case 17:
                        addr = ERRORREG;
                        operation_pending = 1;
                        substate = 18;
                        break;
                    case 18:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else if (i != 0 && !(tempRegValue & 0x1B)) {
                            addr = FIFODATAREG;
                            operation_pending = 1;
                            substate = 19;
                        } else {
                            state = substate = 0;
                        }
                        break;
                    case 19:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            UID[0] = tempRegValue;
                            addr = FIFODATAREG;
                            operation_pending = 1;
                            substate = 20;
                        }
                        break;
                    case 20:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            UID[1] = tempRegValue;
                            addr = FIFODATAREG;
                            operation_pending = 1;
                            substate = 21;
                        }
                        break;
                    case 21:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            UID[2] = tempRegValue;
                            addr = FIFODATAREG;
                            operation_pending = 1;
                            substate = 22;
                        }
                        break;
                    case 22:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            UID[3] = tempRegValue;
                            addr = FIFODATAREG;
                            operation_pending = 1;
                            substate = 23;
                        }
                        break;
                    case 23:
                        if (tempRegValue == 0xFF) {
                            state = substate = 0;
                        } else {
                            UID[4] = tempRegValue;
                            UID[5] = 0;
                            checksum = UID[0] ^ UID[1] ^ UID[2] ^ UID[3];
                            allZero = ((UID[0] | UID[1] | UID[2] | UID[3]) == 0);
                            substate = 24;
                        }
                        break;
                    case 24:
                        if (checksum != UID[4] || allZero) {
                            state = substate = 0;
                        } else {
                            substate = 25;
                        }
                        break;
                        
                    case 25:
                        // Actualizamos el usuario actual y posteriormente comprobamos que este sea o no el mismo
                        getActualUID(currentUser, 0xFF);
                        if (getCurrentUserIndex() != 4) {
                            substate = 26;
                        } else if (cardRemoved == 1) { // en caso de que la tarjeta no haya sido retirada respecto a la ultima vez que se insertó, no actuamos
                            substate = 27;
                        } else {
                            substate = 28;
                        }

                        break;
                        
                    case 26:
                        // En caso de que el usuario sea el mismo que anteriormente y se haya retirado la tarjeta del sensor, significará que el usuario que habia en la sala, se ba retirado
                        if(currentUser[0] == UID[0] && currentUser[1] == UID[1] && 
                           currentUser[2] == UID[2] && currentUser[3] == UID[3] && 
                           currentUser[4] == UID[4]) {
                            if (cardRemoved == 1) {
                                motor_StartSendString("\r\nL'usuari ha sortit de la sala\r\n");
                                setStartSendString();
                                setIndex(4);
                                newConfiguration();
                                substate = 28;
                                cardRemoved = 0;
                            } else {
                                // no se ha retirado la tarjeta, por lo que no hacemos nada
                                substate = 28;
                            }
                        } else {
                            // Es un usuario diferente, actualizar al nuevo usuario
                            substate = 27;
                        }
                        break;
                        
                    case 27:
                        // Iniciar el proceso de establecer el nuevo usuario.
                        if(motor_setCurrentUser(UID[0], UID[1], UID[2], UID[3], UID[4]) == 1) {
                            substate = 28;
                            cardRemoved = 0;
                        }
                        break;
                        
                    case 28:
                        // Preparar para la siguiente lectura yow
                        addr = BITFRAMINGREG;
                        value = 0x00;
                        operation_pending = 2;
                        state = substate = 0;
                        break;
                }
                break;
        }
    }
}