#include <xc.h>
#include "TAD_DISPLAY.h"
#include "TAD_TIMER.h"
#include "TAD_DATOS.h"

#define UID_SIZE 5
#define MAX_USERS 3
#define LEDS 6

unsigned char userUIDs[MAX_USERS][UID_SIZE] = {
    {"42-34-23-23-32"},
    {"12-12-D2-12-12"},
    {"23-23-23-23-23"}
};

unsigned char configurations[MAX_USERS][LEDS] = { 
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0}
};


unsigned char getActualUID(){
    UID[UID_SIZE] = getActualUser();
    return UID;
}

unsigned char getUserConfiguration(){
    if(UID == userUIDs[0]){
        return configurations[0];
    } else if(UID == userUIDs[1]){
        return configurations[1];
    } else if(UID == userUIDs[2]){
        return configurations[2];
    } else {
        return NULL;
    }
}

void setUserConfiguration(unsigned char led[6], unsigned char UID){
    if(UID == userUIDs[0]){
        configurations[0][0] = led[0];
        configurations[0][1] = led[1];
        configurations[0][2] = led[2];
        configurations[0][3] = led[3];
        configurations[0][4] = led[4];
        configurations[0][5] = led[5];
    } else if(UID == userUIDs[1]){
        configurations[1][0] = led[0];
        configurations[1][1] = led[1];
        configurations[1][2] = led[2];
        configurations[1][3] = led[3];
        configurations[1][4] = led[4];
        configurations[1][5] = led[5];
    } else if(UID == userUIDs[2]){
        configurations[2][0] = led[0];
        configurations[2][1] = led[1];
        configurations[2][2] = led[2];
        configurations[2][3] = led[3];
        configurations[2][4] = led[4];
        configurations[2][5] = led[5];
    }
}

