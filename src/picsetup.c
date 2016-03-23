#include "picsetup.h"

void setup_pins(void)
{
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

        // Not implemented.
        // RB14 (Pin 25) is the kill signal to the shutdown timer.
        //TRISBbits.TRISB14 = 0;

#ifdef USE_LCD
        // SPI communication with LCD screen.
        TRISBbits.TRISB13 = 0;     // RB13 (pin 24, output for A0 select)
        TRISAbits.TRISA1 = 0;      // RA1 (pin 3, lcd reset line)
        RPB2Rbits.RPB2R = 0b0011;  // SDO1 (pin 6, SDO1 out)
#endif

#ifdef USE_BATTERY_MONITOR
        // Set RA3 (Pin 10) to be battery level input.
        TRISAbits.TRISA3 = 1;
#endif
}

/* Restarts pic (for example, if an initialization task fails).*/
void pic_reset(void)
{
        SoftReset();
}

/* Enable INT2, which accepts the kill signal. */
void setup_shutdown_int(void)
{
        mINT3SetEdgeMode(0); // Rising edge = 1. Falling edge = 0.
        mINT3SetIntPriority(5);
        mINT3SetIntSubPriority(0);
        mINT3ClearIntFlag();
        mINT3IntEnable(1);
}

/* Set up change interrupt for voltage indicator. */
void setup_battery_int(void)
{
#ifdef USE_BATTERY_MONITOR
        ConfigINT1(EXT_INT_ENABLE | RISING_EDGE_INT | EXT_INT_PRI_5);
#endif
}