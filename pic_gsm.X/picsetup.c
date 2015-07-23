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
    
    // Setting up Timer2 and OC4 for PWM 
    T2CONbits.TCKPS = 2;      // Timer2 prescaler N=4 (1:4)
	PR2 = 4999;               // period register
	TMR2 = 0;                 // initial TMR2 count is 0
	OC4CONbits.OCM = 0b110;   // PWM mode without fault pin; other OC1CON bits are defaults
    RPA2Rbits.RPA2R = 0b0101; // Enable OC4 on Pin 9 
	OC4RS = 2500;             // duty cycle = 50%
	OC4R = 2500;              // initialize before turning OC1 on; afterward it is read-only

    // Setting up interrupts for battery light indicator
    // Configure INT1 Interrupt, falling edge, RA3 (pin 10)
    INT1Rbits.INT1R = 0b0000; //Enable INT1 on Pin 10 (RA3) 
    INTCONbits.INT1EP = 0; //INT1 triggers on falling edge
    IPC1bits.INT1IP = 5; //Set priority (5)
    IPC1bits.IC1IS = 2;  //Set sub priority (2)  
    IFS0bits.INT1IF = 0; //Clear the interrupt flag
    IEC0bits.INT1IE = 1; //Enable the interrupt 
    
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

void set_buzzer_freq(double freq){
    //freq is buzzer frequency in Hz, 2000 is good
    
    //Turn off Timer2, OC1
    OC4CONbits.ON = 0;       // turn off OC1
    T2CONbits.ON = 0;        // turn off Timer2
    
    //Adjust PR2
    PR2 = (int)(10000000.0 / freq - 1.0);
            
    //Turn on Timer2, OC1
    T2CONbits.ON = 1;        // turn on Timer2
	OC4CONbits.ON = 1;       // turn on OC1
}

int set_buzzer_duty(double percent){
    //Percent should be a int between 0 and 100
    OC4RS = (int)(percent*((double)PR2)/100.0); 
}

void buzzer_beep(void){
    set_buzzer_duty(50.0);
    set_buzzer_freq(6000.0);
    delay_ms(5);
    set_buzzer_duty(0.0);
}
