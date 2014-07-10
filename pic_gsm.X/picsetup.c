#include <plib.h>

void setup_pins(void) {
	// Set all analog input pins to digital mode.
	ANSELA = 0;
	ANSELB = 0;

        // Set all pins to digital inputs.
        //TRISA = 0xFFFF;
        //TRISB = 0xFFFF;

        // Set the pins for uart communication.
	U2RXRbits.U2RXR = 0b0100; // U2RX (pin 17, 5V tolerant)
	RPB9Rbits.RPB9R = 0b0010; // U2TX (pin 18, 5V tolerant)
        U1RXRbits.U1RXR = 0b0010; // U1RX (pin 12)
        RPA0Rbits.RPA0R = 0b0001; // U1TX (pin 2)

        // Set RA2 to be a digital out for powerkey.
        TRISAbits.TRISA2 = 0;

        // Set RB4 to be the GSM indicator LED.
        TRISBbits.TRISB4 = 0;

        // RB15 is the RFID indicator LED.
        TRISBbits.TRISB15 = 0;

        // RB14 is push button.
        TRISBbits.TRISB14 = 1;
        
};

/* Restarts pic (for example, if an initialization task fails).*/
void pic_reset(void) {
    SoftReset();
}
