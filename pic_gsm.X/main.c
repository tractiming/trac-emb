#include <p32xxxx.h>
#include <plib.h>
#include "comm.h"
#include "picsetup.h"
#include "gsm.h"

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

int main(void) {

    // Setup PIC.
    CFGCONbits.JTAGEN = 0;
    SYSTEMConfigPerformance(SYS_FREQ);
    setup_pins();
    ERROR_LED = 0;

    // Setup UART and gsm module.
    uart2_init();
    //uart1_init();
    INTEnableSystemMultiVectoredInt();
    delay_ms(1000);
    gsm_init();

    int flag = gsm_tcp_connect();
    if (!flag)
        GSM_LED = 1;
    else
        ERROR_LED = 1;
    
    // Set number and send text.
    //write_string(UART2, "AT+CMGS=\"+17083411935\"\r");
    //write_string(UART2, "AT+CMGS=\"+17733229404\"\r");
    //while (gsm_get_state() != GSM_READY);
    //GSM_LED = 0;

    //write_string(UART2, "gsm module is working");
    //put_character(UART2, 0x1A);
    //while (gsm_get_state() != GSM_OK);

    //GSM_LED = 1;

    while (1) {};
    
    return 0;
}

/* Interrupt for handling uart communication with gsm module. */
void __ISR(_UART_2_VECTOR, IPL7SOFT) IntUart2Handler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(UART2))) {

    char data = UARTGetDataByte(UART2);
    gsm_update_state(data);
    //put_character(UART1, data);

    // Clear the RX interrupt flag.
    INTClearFlag(INT_SOURCE_UART_RX(UART2));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(UART2))) {
    INTClearFlag(INT_SOURCE_UART_TX(UART2));
  }
}

