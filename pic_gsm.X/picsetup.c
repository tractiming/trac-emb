#include "picsetup.h"

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

    // RB5 (Pin 14) is INT3 for shutdown interrupt.
    INT3Rbits.INT3R = 0b0001;
    TRISBbits.TRISB5 = 1;
    
    // Setting up Timer2 and OC1 for PWM 
    T2CONbits.TCKPS = 0b010;     // Timer2 prescaler N=4 (1:4)
	PR2 = 4999;              // period register
	TMR2 = 0;                // initial TMR2 count is 0
	OC1CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
	OC1RS = PR2/2;             // duty cycle = 50%
	OC1R = PR2/2;              // initialize before turning OC1 on; afterward it is read-only
	
    // Not implemented.
    // RB14 (Pin 25) is the kill signal to the shutdown timer.
    //TRISBbits.TRISB14 = 0;
        
};

/* Restarts pic (for example, if an initialization task fails).*/
void pic_reset(void) {
    SoftReset();
}

/* Enable INT2, which accepts the kill signal. */
void setup_shutdown_int(void) {
    mINT3SetEdgeMode(0); // Rising edge = 1. Falling edge = 0.
    mINT3SetIntPriority(5);
    mINT3SetIntSubPriority(0);
    mINT3ClearIntFlag();
    mINT3IntEnable(1);
}

void set_buzzer_freq(int freq){
    //freq is buzzer frequency in Hz, 2000 is good
    
    //Turn off Timer2, OC1
    OC1RS = 0;
    OC1CONbits.ON = 0;       // turn off OC1
    T2CONbits.ON = 0;        // turn off Timer2
    
    //Adjust PR2
    PR2 = (SYS_FREQ / 4) / freq - 1;
            
    //Turn on Timer2, OC1
    T2CONbits.ON = 1;        // turn on Timer2
	OC1CONbits.ON = 1;       // turn on OC1
    OC1RS = 0;
}

int set_buzzer_duty(int percent){
    //Percent should be a int between 0 and 100
    OC1RS = percent*PR2/100; 
}
