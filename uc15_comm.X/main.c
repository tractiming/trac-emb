#define NU32_STANDALONE
#include "NU32.h"

#define MAX_MESSAGE_LENGTH 200

void delay(void);

int main(void) {

    char message[MAX_MESSAGE_LENGTH];

    NU32_Startup();

    while(1) {
        sprintf(message, "Hello? What do you want to say? ");
        NU32_WriteUART1(message);
        NU32_ReadUART1(message, MAX_MESSAGE_LENGTH);
        NU32_WriteUART1(message);
        NU32_WriteUART1("\r\n");
        NU32LED1 = !NU32LED1;
        NU32LED2 = !NU32LED2;

        //delay();
        //LATAINV = 0x0030;    // toggle the two lights
    }
    return 0;
}

void delay(void) {
    int j;
    for (j=0; j<1000000; j++) { // number is 1 million
        while(!NU32USER);   // Pin C13 is the USER switch, low if pressed.
    }
}

