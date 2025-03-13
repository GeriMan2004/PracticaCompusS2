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

char motor_Write(char addr, char value) {
    static char state_write = 0;
    static unsigned char ucAddr;
    static unsigned char ucValue;
    static char bit_count = 0;

    switch (state_write) {
        case 0: // Initialization phase
            MFRC522_SCK = 0;
            MFRC522_CS = 0;
            ucAddr = ((addr << 1) & 0x7E);
            ucValue = value;
            bit_count = 0;
            state_write = 1; // Move to sending address bits
            break;

        case 1: // Send address bits
            MFRC522_SI = ((ucAddr & 0x80) != 0);
            MFRC522_SCK = 1;
            ucAddr <<= 1;
            delay_us(5);
            MFRC522_SCK = 0;
            delay_us(5);
            bit_count++;
            if (bit_count == 8) {
                bit_count = 0;   // Reset counter for the next byte
                state_write = 2; // Transition to sending value bits
            }
            break;

        case 2: // Send value bits
            MFRC522_SI = ((ucValue & 0x80) != 0);
            MFRC522_SCK = 1;
            ucValue <<= 1;
            delay_us(5);
            MFRC522_SCK = 0;
            delay_us(5);
            bit_count++;
            if (bit_count == 8) {
                MFRC522_CS = 1;
                MFRC522_SCK = 1;
                state_write = 0; // Reset state for next transmission
                return 1;      // Transmission complete
            }
            break;
    }
    return 0; // Transmission still in progress
}


char motor_Read(char addr) {
    static char state_read = 0;
    static char bit_count = 0;
    static unsigned char ucAddr;
    static unsigned char ucResult;

    switch(state_read) {
        case 0: // Initialization
            MFRC522_SCK = 0;
            MFRC522_CS = 0;
            ucAddr = ((addr<<1) & 0x7E) | 0x80;
            ucResult = 0;
            bit_count = 0;
            state_read = 1;
            break;
            
        case 1: // Send address bits
            MFRC522_SI = ((ucAddr & 0x80) == 0x80);
            MFRC522_SCK = 1;
            delay_us(5);
            ucAddr <<= 1;
            MFRC522_SCK = 0;
            delay_us(5);
            bit_count++;
            
            if (bit_count >= 8) {
                bit_count = 0;
                state_read = 2;
            }
            break;
            
        case 2: // Receive data bits
            MFRC522_SCK = 1;
            delay_us(5);
            ucResult <<= 1;
            ucResult |= MFRC522_SO;
            MFRC522_SCK = 0;
            delay_us(5);
            bit_count++;
            
            if (bit_count >= 8) {
                MFRC522_CS = 1;
                MFRC522_SCK = 1;
                state_read = 0;
                return ucResult; // Return the read value only when complete
            }
            break;
    }
    return 0; // Still in progress or initialization
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
    unsigned char flag = 0;
    static unsigned char lastBitsVal;
    static unsigned char fifoLevel;
    unsigned char backBitsCalc;
    switch(state) {
        // Estado 0: Detección de tarjeta (Request)
        case 0:
            switch(substate) {
                case 0:
                    // Configura BITFRAMING y asigna TagType
                    flag = motor_Write(BITFRAMINGREG, 0x07);
                    if (flag != 0){
                    TagType = PICC_REQIDL;
                    substate = 1;
                    }
                    break;
                case 1:
                    // Configura las interrupciones para el comando TRANSCEIVE
                    irqEn = 0x77;
                    waitIRq = 0x30;
                    flag = motor_Write(COMMIENREG, irqEn | 0x80);
                    if (flag != 0){
                    substate = 2;
                    }
                    break;
                case 2:
                    // Read COMMIRQREG to prepare for clearing
                    flag = motor_Read(COMMIRQREG);
                    if (flag != 0){
                        tempRegValue = flag;
                        substate = 3;
                    }
                    break;
                case 3:
                    // Clear the interrupt register
                    flag = motor_Write(COMMIENREG, tempRegValue & ~0x80);
                    if (flag != 0){
                        substate = 4;
                    }
                    break;
                case 4: // NO VA
                    // Read FIFOLEVELREG to prepare for setting
                    tempRegValue = MFRC522_Rd(FIFOLEVELREG);
                    substate = 5;
                    break;
                case 5:
                    // Reinicia el FIFO
                    flag = motor_Write(FIFOLEVELREG, tempRegValue | 0x80);
                    if (flag != 0){
                        substate = 6;
                    }
                    break;
                case 6:
                    // Pone el lector en modo IDLE
                    flag = motor_Write(COMMANDREG, PCD_IDLE);
                    if (flag != 0){
                        substate = 7;
                    }
                    break;
                case 7:
                    // Carga el comando (TagType) en el FIFO
                    flag = motor_Write(FIFODATAREG, TagType);
                    if (flag != 0){
                        substate = 8;
                    }
                    break;
                case 8:
                    // Inicia el comando TRANSCEIVEç
                    flag = motor_Write(COMMANDREG, PCD_TRANSCEIVE);
                    if (flag != 0){
                        substate = 9;
                    }
                    break;
                case 9:
                    // Read BITFRAMINGREG to prepare for setting
                    flag = motor_Read(BITFRAMINGREG);
                    if (flag != 0){
                        tempRegValue = flag;
                        substate = 10;
                    }
                    break;
                case 10:
                    // Activa StartSend para iniciar la transmisión y reinicia el contador
                    flag = motor_Write(BITFRAMINGREG, tempRegValue | 0x80);
                    if (flag != 0){
                        i = 0xFF;
                        substate = 11;
                    }
                    break;
                case 11:
                    // Espera la respuesta; se realiza una comprobación en cada llamada
                    flag = motor_Read(COMMIRQREG);
                    if (flag != 0){
                        n = flag;
                        if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                            substate = 12;
                        }
                    }
                    break;
                case 12:
                    // Read BITFRAMINGREG to prepare for clearing
                    flag = motor_Read(BITFRAMINGREG);
                    if (flag != 0){
                        tempRegValue = flag;
                        substate = 13;
                    }
                    break;
                case 13:
                    // Desactiva el StartSend
                    flag = motor_Write(BITFRAMINGREG, tempRegValue & ~0x80);
                    if (flag != 0){
                        substate = 14;
                    }
                    break;
                case 14: // NO VA
                    // Primera lectura: verificar errores
                    tempRegValue = MFRC522_Rd(ERRORREG);
                    if (i != 0 && !(tempRegValue & 0x1B)) {
                        substate = 15; // Continuar con validación
                    } else { // Lectura inválida
                        substate = 0;
                        state = 0;
                    }
                    break;
                    
                case 15: // NO VA
                    // Segunda lectura: obtener nivel de FIFO
                    fifoLevel = MFRC522_Rd(FIFOLEVELREG);
                    substate = 16;
                    break;
                    
                case 16:
                    // Tercera lectura: obtener bits de control
                    flag = motor_Read(CONTROLREG);
                    if (flag != 0){
                        lastBitsVal = flag & 0x07;
                        substate = 17;
                    }
                    break;
                    
                case 17:
                    // Cálculo de bits recibidos (sin lectura MFRC522)
                    if (lastBitsVal)
                        backBitsCalc = (fifoLevel - 1) * 8 + lastBitsVal;
                    else
                        backBitsCalc = fifoLevel * 8;
                    
                    // Si se reciben exactamente 16 bits (0x10), la lectura es válida
                    if (backBitsCalc == 0x10) {
                        state = 1;
                        substate = 0;
                    } else {
                        state = 0;
                        substate = 0;
                    }
                    break;
            }
            break;
        // Estado 1: Lectura de UID (AntiColisión)
        case 1:
            switch(substate) {
                case 0:
                    // Configura BITFRAMING para la anti-colisión y asigna el comando correspondiente
                    flag = motor_Write(BITFRAMINGREG, 0x00);
                    if (flag != 0){
                    UID[0] = PICC_ANTICOLL;
                    UID[1] = 0x20;
                    substate = 1;
                    }
                    break;
                case 1:
                    // Read STATUS2REG to prepare for clearing
                    flag = motor_Read(STATUS2REG);
                    if (flag != 0){
                        tempRegValue = flag;
                        substate = 2;
                    }
                    break;
                case 2:
                    // Limpia el STATUS2 para iniciar el proceso de anti-colisión
                    flag = motor_Write(STATUS2REG, tempRegValue & ~0x08);
                    if (flag != 0){
                        substate = 3;
                    }
                    break;
                case 3:
                    // Configura las interrupciones para TRANSCEIVE en modo anti-colisión
                    irqEn = 0x77;
                    waitIRq = 0x30;
                    flag = motor_Write(COMMIENREG, irqEn | 0x80);
                    if (flag != 0){
                        substate = 4;
                    }
                    break;
                case 4:
                    // Read COMMIRQREG to prepare for clearing
                    flag = motor_Read(COMMIRQREG);
                    if (flag != 0){
                        tempRegValue = flag;
                        substate = 5;
                    }
                    break;
                case 5:
                    // Limpia el registro de interrupciones
                    flag = motor_Write(COMMIRQREG, tempRegValue & ~0x80);
                    if (flag != 0){
                        substate = 6;
                    }
                    break;
                case 6:
                    // Read FIFOLEVELREG to prepare for setting
                    flag = motor_Read(FIFOLEVELREG);
                    if (flag != 0){
                        tempRegValue = flag;
                        substate = 7;
                    }
                    break;
                case 7:
                    // Reinicia el FIFO
                    flag = motor_Write(FIFOLEVELREG, tempRegValue | 0x80);
                    if (flag != 0){
                        substate = 8;
                    }
                    break;
                case 8:
                    // Pone el lector en modo IDLE
                    flag = motor_Write(COMMANDREG, PCD_IDLE);
                    if (flag != 0){
                        substate = 9;
                    }
                    break;
                case 9:
                    // Carga en el FIFO el comando de anti-colisión
                    flag = motor_Write(FIFODATAREG, UID[0]);
                    if (flag != 0){
                        substate = 10;
                    }
                    break;
                case 10:
                    flag = motor_Write(FIFODATAREG, UID[1]);
                    if (flag != 0){
                        substate = 11;
                    }
                    break;
                case 11:
                    // Inicia el comando TRANSCEIVE para anti-colisión
                    flag = motor_Write(COMMANDREG, PCD_TRANSCEIVE);
                    if (flag != 0){
                        substate = 12;
                    }
                    break;
                case 12:
                    // Read BITFRAMINGREG to prepare for setting
                    tempRegValue = MFRC522_Rd(BITFRAMINGREG);
                    substate = 13;
                    break;
                case 13:
                    // Activa StartSend y reinicia el contador de espera (timeout reducido)
                    flag = motor_Write(BITFRAMINGREG, tempRegValue | 0x80);
                    if (flag != 0){
                    i = 0xFF;
                        substate = 14;
                    }
                    break;
                case 14: // NO VA
                    // Espera la respuesta de anti-colisión
                    n = MFRC522_Rd(COMMIRQREG);
                    if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                        substate = 15;
                    }
                    break;
                case 15: // NO VA
                    // Read BITFRAMINGREG to prepare for clearing
                    tempRegValue = MFRC522_Rd(BITFRAMINGREG);
                    substate = 16;
                    break;
                case 16:
                    // Desactiva el StartSend
                    flag = motor_Write(BITFRAMINGREG, tempRegValue & ~0x80);
                    if (flag != 0){
                        substate = 17;
                    }
                    break;
                case 17: // NO VA
                    // Iniciar validación del UID - Lee el registro de errores
                    tempRegValue = MFRC522_Rd(ERRORREG);
                    if (i != 0 && !(tempRegValue & 0x1B)) {
                        // No hay error, continuar con la lectura de UID
                        substate = 18;
                    } else {
                        // Error detectado, reinicia el proceso
                        state = 0;
                        substate = 0;
                    }
                    break;
                
                case 18:
                    // Lee el primer byte del UID
                    flag = motor_Read(FIFODATAREG);
                    if (flag != 0){
                        UID[0] = flag;
                        substate = 19;
                    }
                    break;
                case 19: // NO VA
                    // Lee el segundo byte del UID
                    UID[1] = MFRC522_Rd(FIFODATAREG);
                    substate = 20;
                    break;               
                case 20: // NO VA
                    // Lee tercer byte del UID
                    UID[2] = MFRC522_Rd(FIFODATAREG);
                    substate = 21;
                    break;
                case 21:
                    // Lee cuarto byte del UID
                    flag = motor_Read(FIFODATAREG);
                    if (flag != 0){
                        UID[3] = flag;
                        substate = 22;
                    }
                    break;
                
                case 22: // NO VA
                    // Lee el byte de checksum y añade terminador
                    UID[4] = MFRC522_Rd(FIFODATAREG);
                    UID[5] = 0; // Terminador nulo
                    substate = 23;
                    break;
                
                case 23:                    
                    // Calcula el checksum como XOR de los 4 primeros bytes
                    checksum = UID[0] ^ UID[1] ^ UID[2] ^ UID[3];
                    // Inicializa flag para verificar UID no cero
                    allZero = 1;
                    substate = 24;
                    break;
                
                case 24:
                    // Verifica los primeros 2 bytes del UID por si son ceros
                    if (UID[0] != 0 || UID[1] != 0) {
                        allZero = 0;
                    }
                    substate = 25;
                    break;
                
                case 25:
                    // Verifica los siguientes 2 bytes del UID por si son ceros
                    if (UID[2] != 0 || UID[3] != 0) {
                        allZero = 0;
                    }
                    substate = 26;
                    break;
                
                case 26:
                    // Verifica el checksum y si el UID no es todo ceros
                    if (checksum != UID[4] || allZero) {
                        // Si hay error, reinicia
                        state = 0;
                        substate = 0;
                    } else {
                        // Todo correcto, continúa
                        substate = 27;
                    }
                    break;
                
                case 27:
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
                        substate = 28;
                    }
                    break;
                
                case 28:
                    // Configuración final y reinicio
                    flag = motor_Write(BITFRAMINGREG, 0x00);
                    if (flag != 0){
                        state = 0;
                        substate = 0;
                    }
                    break;
            }
            break;
    }
}