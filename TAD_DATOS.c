#include <xc.h>
#include "TAD_DISPLAY.h"
#include "TAD_TIMER.h"
#include "TAD_DATOS.h"
#include <string.h>

unsigned char userUIDs[MAX_USERS][UID_SIZE] = {
    "11-04-19-94-E0",
    "22-05-18-74-F1",
    "33-06-17-84-G2"
};

unsigned char configurations[MAX_USERS][LEDS] = { 
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0}
};

unsigned char currentUser[16] = "00-00-00-00-00";
unsigned char new_user = 0;
unsigned char new_configuration = 0;
int index = 0, pointer = 0;
unsigned char currentTime[5] = "00:00";

unsigned char* getActualUID(void) {
    return currentUser;
}

// Function to get all users configurations - returns pointer to first element
unsigned char* getUsersConfigurations(void) {
    return &configurations[0][0]; // Returns pointer to first element, can be accessed as array of MAX_USERS * LEDS
}

void newUser(void) {
    new_user = 1;
}

void newConfiguration(void) {
    new_configuration = 1;
}

void motor_datos(void) {
	static char state = 0;
    unsigned char* temp_user;
    unsigned char* temp_config;

	switch(state) {
		case 0:
			if (new_configuration == 1) {
				new_configuration = 0;
				index = 0;
				state = 1;
			}
            if(new_user == 1) {
                new_user = 0;
                temp_user = getCurrentUser();
                if(temp_user != NULL) {
                    strncpy((char*)currentUser, (const char*)temp_user, UID_SIZE);
                }
				state = 2;
            }
		break;
		case 1:
            if (index < MAX_USERS) {
                if (strncmp((char*)currentUser, (char*)userUIDs[index], UID_SIZE) == 0) {
                    //temp_config = getNewConfiguration();
                    if(temp_config != NULL) {
                        for(int i = 0; i < LEDS; i++) {
                            configurations[index][i] = temp_config[i];
                        }
                    }
                    state = 2;
                } else {
                    index++;
                }
            } else {
                state = 2; 
            }
		break;
		case 2:
			LcPutChar(currentUser[15]);
			LcPutChar(' ');
			pointer = 0;
			state = 3;
		break;
		case 3:
			if (pointer < 5) {
				LcPutChar(currentTime[pointer]);
				pointer++;
			}
			else if (pointer == 5) {
				pointer = 0;
				LcPutChar(' ');
				state = 4;
			}
		break;
		case 4:
			if (pointer < LEDS) {
				LcPutChar((char)(pointer + 1));
				LcPutChar('-');
				LcPutChar(configurations[index][pointer]);
				LcPutChar(' ');
				pointer++;
			} else {
				pointer = 0;
				state = 0;
			}
		break;

	}
}

