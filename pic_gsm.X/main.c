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

#define MAX_LEN 100

int tcp_connected;
int hb_cnt;

int main(void) {

    // Set up PIC.
    CFGCONbits.JTAGEN = 0;
    SYSTEMConfigPerformance(SYS_FREQ);
    setup_pins();

    GSM_LED = 0; RFID_LED = 0;
    tcp_connected = 0;
    hb_cnt = 0;
    
    // Set up UART communication with GSM. Establish heartbeat. These are only
    // setting registers on the pic, so don't worry about failure.
    gsm_init_uart();
    hb_tmr_init();

    // Start interrupts (allows pic to receive uart messages).
    INTEnableSystemMultiVectoredInt();
    delay_ms(5000);

    // Initialize the GSM module:
    //     1. Turn on and sync baudrate.
    //     2. Bring up the tcp connection.
    if (gsm_init() || gsm_tcp_connect()) {
        gsm_pwr_off();
        pic_reset();
    }
    tcp_connected = 1;
    delay_ms(5000);
    GSM_LED = 1;

    // Initialize the RFID reader.
    rfid_init_uart(); // init uart here so we don't interrupt gsm init.
    if (rfid_init())
        pic_reset();
    delay_ms(2000);
    RFID_LED = 1;
    
    //delay_ms(3000);
    while (1) {

        // Check to make sure the connection is being maintained.
        if (gsm_chk_tcp_conn()) {
            tcp_connected = 0;
            GSM_LED = 0;

            // Try to reset the connection. If this fails, shutdown
            // the pic and try to boot again.
            if (gsm_tcp_reset()) {
                gsm_pwr_off();
                pic_reset();
            }
            tcp_connected = 1;
            GSM_LED = 1;
        }

        // If shutdown button pressed, disconnect tcp.
        //if (!PORTBbits.RB14)
        //{
        //    // If button pressed, deactivate the GSM.
        //    if (tcp_connected) {
        //        gsm_tcp_close();
        //        tcp_connected = 0;
        //        GSM_LED = 0;
        //    }

        //}

        // If rfid buffer not empty, send message over gsm.
        if (!rfid_msg_empty()) {
            hb_cnt = 0;         // Reset heartbeat, data being sent.
            rfid_read_bfr();
        }

    };
    
    return 0;
}

/* Interrupt for maintaining TCP connection. */
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) T2Interrupt(void) {

    hb_cnt++;
    if (hb_cnt == HB_COUNT) {

        if (tcp_connected)
            write_string(GSM_UART, "beat\r\n");
    
        hb_cnt = 0;
    }
    mT2ClearIntFlag();
}

/* Interrupt for handling uart communication with gsm module. */
void __ISR(GSM_UART_VEC, IPL6SOFT) IntGSMUartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(GSM_UART))) {

    char data = UARTGetDataByte(GSM_UART);
    gsm_update_state(data);

    // Clear the RX interrupt flag.
    INTClearFlag(INT_SOURCE_UART_RX(GSM_UART));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(GSM_UART))) {
    INTClearFlag(INT_SOURCE_UART_TX(GSM_UART));
  }
}

/* Interrupt for handling uart communication with rfid reader. */
void __ISR(RFID_UART_VEC, IPL7SOFT) IntRFIDUartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(RFID_UART))) {

    char data = UARTGetDataByte(RFID_UART);
    rfid_write_bfr(data);

    // Clear the RX interrupt flag.
    INTClearFlag(INT_SOURCE_UART_RX(RFID_UART));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(RFID_UART))) {
    INTClearFlag(INT_SOURCE_UART_TX(RFID_UART));
  }
}

/* This is junk, but useful to reference. */
/*void send_text(void) {
    // Set number and send text.
    write_string(UART2, "AT+CMGS=\"+17083411935\"\r");
    write_string(UART2, "AT+CMGS=\"+17733229404\"\r");
    while (gsm_get_state() != GSM_READY);
    GSM_LED = 0;

    write_string(UART2, "gsm module is working");
    put_character(UART2, 0x1A);
    while (gsm_get_state() != GSM_OK);

}*/
//char test_tag[] = "Tag=E200 A123 B456  Last=Fri Jun 19 12:51:21 PDT 2004  Ant=0  Count=3\r\n";
