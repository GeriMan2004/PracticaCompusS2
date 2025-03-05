/*
 * File:   TRFID.c
 * Author: jnavarro & ester.vidana
 *
 * 
 * Inspired by: https://simplesoftmx.blogspot.com/2014/11/libreria-para-usar-lector-rfid-rc522.html
 */


#include <xc.h>
#include "TAD_RFID.h"
#include "TAD_TERMINAL.h"
#include <stdio.h>
#include <string.h>

//-------------- Private functions: --------------
void InitPortDirections () {
    DIR_MFRC522_SO  = 1; 
    DIR_MFRC522_SI  = 0; 
    DIR_MFRC522_SCK = 0; 
    DIR_MFRC522_CS  = 0; 
    DIR_MFRC522_RST = 0;
}

void delay_us (char howMany) {
    #define NUM_US 3  // A 10MHz, cada instrucción = 0.4µs, así que necesitamos ~3 instrucciones para 1µs
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

char MFRC522_ToCard (char command, char *sendData, char sendLen, char *backData, unsigned *backLen) {
    char _status = MI_ERR;
    char irqEn = 0x00;
    char waitIRq = 0x00;               
    char lastBits;
    char n;
    unsigned char i;
    
    switch (command) {
        case PCD_AUTHENT:       //Certification cards close
            irqEn = 0x12;
            waitIRq = 0x10;
            break;

        case PCD_TRANSCEIVE:    //Transmit FIFO data
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
    }
    MFRC522_Wr(COMMIENREG, irqEn | 0x80);  //Interrupt request
    MFRC522_Clear_Bit(COMMIRQREG, 0x80);   //Clear all interrupt request bit
    MFRC522_Set_Bit(FIFOLEVELREG, 0x80);   //FlushBuffer=1, FIFO Initialization
    MFRC522_Wr(COMMANDREG, PCD_IDLE);      //NO action; Cancel the current command???
    
    for (i = 0; i < sendLen; i++) MFRC522_Wr(FIFODATAREG, sendData[i]);
    
    MFRC522_Wr(COMMANDREG, command);
    if (command == PCD_TRANSCEIVE) MFRC522_Set_Bit(BITFRAMINGREG, 0x80); //StartSend=1,transmission of data starts 
    i = 0xFF;
    do {
        n = MFRC522_Rd(COMMIRQREG);
        i--;
    } while (i && !(n & 0x01) && !(n & waitIRq));
    MFRC522_Clear_Bit(BITFRAMINGREG, 0x80);   
    if (i != 0) {
        if(!(MFRC522_Rd(ERRORREG) & 0x1B)){
            _status = MI_OK;
            if (n & irqEn & 0x01) _status = MI_NOTAGERR;      
            if (command == PCD_TRANSCEIVE) {
                n = MFRC522_Rd(FIFOLEVELREG);
                lastBits = MFRC522_Rd(CONTROLREG) & 0x07;
                if (lastBits) {
                    *backLen = (n - 1) * 8 + lastBits;
                } else {
                    *backLen = n * 8;
                }
                if (n == 0) {
                    n = 1;
                } else if (n > 16) {
                    n = 16;
                }
                for (i = 0; i < n; i++) {
                    backData[i] = MFRC522_Rd(FIFODATAREG);
                }
                backData[i] = 0;
            }
        }
        else _status = MI_ERR;
    }
    return _status;
}


char MFRC522_Request(char reqMode, char *TagType) {
    char _status;
    unsigned backBits;
    MFRC522_Wr(BITFRAMINGREG, 0x07);
    TagType[0] = reqMode;
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
    if ((_status != MI_OK) || (backBits != 0x10))
        _status = MI_ERR;
    return _status;
}

void MFRC522_CRC(char *dataIn, char length, char *dataOut) {
    unsigned char i, n;
    MFRC522_Clear_Bit(DIVIRQREG, 0x04);
    MFRC522_Set_Bit(FIFOLEVELREG, 0x80);   
    
    for (i = 0; i < length; i++) {
        MFRC522_Wr(FIFODATAREG, *dataIn++);
    }
    
    MFRC522_Wr(COMMANDREG, PCD_CALCCRC);
    
    // Optimizado el bucle de espera
    i = 255;  // Suficiente para el timeout a 10MHz
    do {
        n = MFRC522_Rd(DIVIRQREG);
        i--;
    } while (i && !(n & 0x04));        //CRCIrq = 1
       
    dataOut[0] = MFRC522_Rd(CRCRESULTREGL);
    dataOut[1] = MFRC522_Rd(CRCRESULTREGM);       
}

unsigned MFRC522_SelectTag(char *serNum) {
    char i;
    char _status;
    unsigned size;
    char buffer[9];
    
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    
    for (i = 0; i < 5; i++) {
        buffer[i + 2] = *serNum++;
    }
    
    MFRC522_CRC(buffer, 7, &buffer[7]);            
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &size);
    
    if ((_status == MI_OK) && (size == 0x18)) {
        size = buffer[0];
    } else {
        size = 0;
    }
    return size;
}

//hibernation
void MFRC522_Halt() {
    unsigned unLen;
    char buff[4];
    
    buff[0] = PICC_HALT;
    buff[1] = 0;
    MFRC522_CRC(buff, 2, &buff[2]);
    MFRC522_Clear_Bit(STATUS2REG, 0x80);
    MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
    MFRC522_Clear_Bit(STATUS2REG, 0x08);
}

char MFRC522_AntiColl(unsigned char *serNum) {
    char _status;
    char i;
    char serNumCheck = 0;
    unsigned unLen;
    MFRC522_Wr(BITFRAMINGREG, 0x00);                //TxLastBists = BitFramingReg[2..0]
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    MFRC522_Clear_Bit(STATUS2REG, 0x08);
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, (char *)serNum, 2, (char *)serNum, &unLen);
    if (_status == MI_OK) {
        for (i = 0; i < 4; i++)
            serNumCheck ^= serNum[(int)i];  
        if (serNumCheck != serNum[4]) 
            _status = MI_ERR;
    }
    return _status;
}

char MFRC522_isCard (char *TagType) {
    return (MFRC522_Request(PICC_REQIDL, TagType) == MI_OK);
}

char MFRC522_ReadCardSerial (unsigned char *str) {
    char _status = MFRC522_AntiColl(str);
    str[5] = 0;  // Aseguramos que el string termine en null
    return (_status == MI_OK);
}

//-------------- Public functions: --------------
void initRFID() {
    InitPortDirections();
    MFRC522_Init(); 
}

void ReadRFID_NoCooperatiu() {
    unsigned char UID[6];    // 5 bytes para UID + 1 para null terminator
    char TagType;   // Variable para el tipo de tarjeta
    if (MFRC522_isCard (&TagType) == 1) {
        //At this point, TagType contains an integer value corresponding to the type of card.
        //Read ID
        if (MFRC522_ReadCardSerial (UID)) {
            //At this point, UID contains the value of the card.
            displayUID(UID);
        }                                      
        MFRC522_Halt ();
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
    static char buffer[100];
    char *p = buffer;
    p += sprintf(p, "state: %d, substate: %d\r\n", state, substate);

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
                    // Limpia el registro de interrupciones
                    MFRC522_Clear_Bit(COMMIRQREG, 0x80);
                    substate = 3;
                    break;
                case 3:
                    // Reinicia el FIFO
                    MFRC522_Set_Bit(FIFOLEVELREG, 0x80);
                    substate = 4;
                    break;
                case 4:
                    // Pone el lector en modo IDLE
                    MFRC522_Wr(COMMANDREG, PCD_IDLE);
                    substate = 5;
                    break;
                case 5:
                    // Carga el comando (TagType) en el FIFO
                    MFRC522_Wr(FIFODATAREG, TagType);
                    substate = 6;
                    break;
                case 6:
                    // Inicia el comando TRANSCEIVE
                    MFRC522_Wr(COMMANDREG, PCD_TRANSCEIVE);
                    substate = 7;
                    break;
                case 7:
                    // Activa StartSend para iniciar la transmisión y reinicia el contador
                    MFRC522_Set_Bit(BITFRAMINGREG, 0x80);
                    i = 0xFF;
                    substate = 8;
                    break;
                case 8:
                    // Espera la respuesta; se realiza una comprobación en cada llamada
                    n = MFRC522_Rd(COMMIRQREG);
                    if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                        substate = 9;
                    }
                    break;
                case 9:
                    // Desactiva el StartSend
                    MFRC522_Clear_Bit(BITFRAMINGREG, 0x80);
                    substate = 10;
                    break;
                case 10:
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
                            substate = 11;
                        } else {
                            substate = 12;
                        }
                    } else {
                        substate = 12;
                    }
                    break;
                case 11:
                    // La lectura es válida: pasa al estado de lectura de UID
                    state = 1;
                    substate = 0;
                    break;
                case 12:
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
                    // Limpia el STATUS2 para iniciar el proceso de anti-colisión
                    MFRC522_Clear_Bit(STATUS2REG, 0x08);
                    substate = 2;
                    break;
                case 2:
                    // Configura las interrupciones para TRANSCEIVE en modo anti-colisión
                    irqEn = 0x77;
                    waitIRq = 0x30;
                    MFRC522_Wr(COMMIENREG, irqEn | 0x80);
                    substate = 3;
                    break;
                case 3:
                    // Limpia el registro de interrupciones
                    MFRC522_Clear_Bit(COMMIRQREG, 0x80);
                    substate = 4;
                    break;
                case 4:
                    // Reinicia el FIFO
                    MFRC522_Set_Bit(FIFOLEVELREG, 0x80);
                    substate = 5;
                    break;
                case 5:
                    // Pone el lector en modo IDLE
                    MFRC522_Wr(COMMANDREG, PCD_IDLE);
                    substate = 6;
                    break;
                case 6:
                    // Carga en el FIFO el comando de anti-colisión
                    MFRC522_Wr(FIFODATAREG, UID[0]);
                    MFRC522_Wr(FIFODATAREG, UID[1]);
                    substate = 7;
                    break;
                case 7:
                    // Inicia el comando TRANSCEIVE para anti-colisión
                    MFRC522_Wr(COMMANDREG, PCD_TRANSCEIVE);
                    substate = 8;
                    break;
                case 8:
                    // Activa StartSend y reinicia el contador de espera
                    MFRC522_Set_Bit(BITFRAMINGREG, 0x80);
                    i = 0xFF;
                    substate = 9;
                    break;
                case 9:
                    // Espera la respuesta de anti-colisión
                    n = MFRC522_Rd(COMMIRQREG);
                    if ((n & 0x01) || (n & waitIRq) || (--i == 0)) {
                        substate = 10;
                    }
                    break;
                case 10:
                    // Desactiva el StartSend
                    MFRC522_Clear_Bit(BITFRAMINGREG, 0x80);
                    substate = 11;
                    break;
                case 11:
                    // Valida y procesa el UID recibido (se leen 4 bytes)
                    if (i != 0 && !(MFRC522_Rd(ERRORREG) & 0x1B)) {
                        unsigned char fifoLevel = MFRC522_Rd(FIFOLEVELREG);
                        for (i = 0; i < 4; i++) {
                            UID[i] = MFRC522_Rd(FIFODATAREG);
                        }
                        UID[4] = 0; // Terminador nulo
                    }
                    substate = 12;
                    break;
                case 12:
                    // Muestra el UID y pasa al estado de Halt
                    displayUID(UID);
                    state = 2;
                    substate = 0;
                    break;
            }
            break;
        // Estado 2: Halt
        case 2:
            switch(substate) {
                case 0:
                    MFRC522_Wr(COMMANDREG, PICC_HALT);
                    substate = 1;
                    break;
                case 1:
                    // Finalizado el Halt, se regresa al estado de detección
                    state = 0;
                    substate = 0;
                    break;
            }
            break;
    }
}