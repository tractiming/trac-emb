#include <plib.h>

void setup_pins(void) {
	// Set all analog input pins to digital mode.
	ANSELA = 0;
	ANSELB = 0;

        // Set the pins for rs232/usb communication.
	U2RXRbits.U2RXR = 0b0000; // U2RX
	RPA3Rbits.RPA3R = 0b0010; // U2TX

        // Set RA2 to be a digital out for powerkey.
        TRISAbits.TRISA2 = 0;

        // Set RB4 to be the GSM indicator LED.
        TRISBbits.TRISB4 = 0;

        // RB15 is error LED.
        TRISBbits.TRISB15 = 0;
        
};