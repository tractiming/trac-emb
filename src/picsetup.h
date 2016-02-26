#ifndef PICSETUP_H
#define	PICSETUP_H

#define _SUPPRESS_PLIB_WARNING
#include <p32xxxx.h>
#include <plib.h>

#ifdef SETUP
#pragma config IOL1WAY  = OFF       // Peripheral Pin Select Configuration, allow mult reconfig
#pragma config PMDL1WAY = OFF	    // Peripheral Module Disable Config, allow mult reconfig
#pragma config FPLLODIV = DIV_2     // PLL Output Divider
#pragma config FPLLMUL  = MUL_20    // PLL Multiplier
#pragma config FPLLIDIV = DIV_2     // PLL Input Divider
#pragma config FWDTEN   = OFF       // Watchdog Timer
#pragma config WDTPS    = PS8192    // Watchdog Timer Postscale
#pragma config FCKSM    = CSDCMD    // Clock Switching & Fail Safe Clock Monitor
#pragma config FPBDIV   = DIV_1     // Peripheral Clock divisor
#pragma config OSCIOFNC = OFF       // CLKO Enable
#pragma config POSCMOD  = OFF       // Primary Oscillator
#pragma config IESO     = OFF       // Internal/External Switch-over
#pragma config FSOSCEN  = OFF       // Secondary Oscillator Enable (KLO was off)
#pragma config FNOSC    = FRCPLL    // Oscillator Selection
#pragma config CP       = OFF       // Code Protect
#pragma config BWP      = ON        // Boot Flash Write Protect
#pragma config PWP      = OFF       // Program Flash Write Protect
#pragma config ICESEL   = ICS_PGx1  // ICE/ICD Comm Channel Select
#pragma config JTAGEN   = OFF       // JTAG Enable
#pragma config DEBUG    = OFF       // Background Debugger Enable
#endif

#define USE_LCD                 /* Uncomment to enable screen functionality. */

#define SYS_FREQ 40000000L
#define PBCLK  (SYS_FREQ)

#define Fsck	50000
#define BRG_VAL ((PBCLK/2/Fsck)-2)

#define POWERKEY        LATAbits.LATA2
#define GSM_LED         LATBbits.LATB4
#define RFID_LED        LATBbits.LATB15
#define KILL            LATBbits.LATB14
#define BATTERY_STATUS  PORTAbits.RA3

#define MAX_STR_LEN 250

void setup_pins(void);
void pic_reset(void);
void setup_shutdown_int(void);
void setup_battery_int(void);

#endif	/* PICSETUP_H */
