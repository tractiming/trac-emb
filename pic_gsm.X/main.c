#include <p32xxxx.h>
#include <plib.h>
#include "comm.h"
#include "picsetup.h"
#include "gsm.h"
#include "rfid.h"

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
const char reader_id[] = "A1010"; // Unique reader id for this device.

int main(void) {

    // Set up PIC.
    CFGCONbits.JTAGEN = 0;
    SYSTEMConfigPerformance(SYS_FREQ);
    setup_pins();

    GSM_LED = 0;
    RFID_LED = 0;
    
    // Set up UART communication with GSM. Establish heartbeat. These are only
    // setting registers on the pic, so don't worry about failure.
    gsm_init_uart();

    // Start interrupts (allows pic to receive uart messages).
    INTEnableSystemMultiVectoredInt();
    delay_ms(5000);

    // Initialize the GSM module.
    //RFID_LED=1;
    if (!gsm_init(&gsm_state))
    {
        GSM_LED = 1;
        //pic_reset();
    }
    else
        RFID_LED = 1;

    // Inititialize the rfid reader.
    //rfid_init_uart();
    //rfid_init();
    //RFID_LED = 1;

    while (1) {
        //gsm_http_post("m=Testing");
        //delay_ms(1000);
        //if (!PORTBbits.RB14)
        //{
        //    rfid_write_bfr('H');
        //    rfid_write_bfr('\n');
        //    //if (gsm_http_post("m=Test"))
        //    //{
        //        //gsm_send_command("AT+QIDEACT\r", GSM_OK, 10*GSM_TIMEOUT);
        //    //    delay_ms(100);
        //        //gsm_send_command("AT+QIACT\r", GSM_OK, 10*GSM_TIMEOUT);
        //    //}
        //    delay_ms(1000);
        //}
        //rfid_write_bfr('H');
        //rfid_write_bfr('\n');
        //delay_ms(1000);
        //rfid_read_bfr();
        //delay_ms(8000);


        // Post any new data in the split buffer.
        //update_splits(&rfid_split_queue, &rfid_line_buffer);
        //post_splits_to_server(&rfid_split_queue, reader_id);


    };
    
    return 0;
}

/* Interrupt for handling uart communication with gsm module. */
void __ISR(GSM_UART_VEC, IPL6SOFT) IntGSMUartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(GSM_UART))) {

    char data = UARTGetDataByte(GSM_UART);
    gsm_add_to_buffer(&gsm_state, data);

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
  if (INTGetFlag(INT_SOURCE_UART_RX(RFID_UART)))
  {

      // Add the next character to the serial buffer.
      char data = UARTGetDataByte(RFID_UART);
      add_char_to_buffer(&rfid_line_buffer, data);

      // Clear the RX interrupt flag.
      INTClearFlag(INT_SOURCE_UART_RX(RFID_UART));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(RFID_UART)))
  {
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
/* Interrupt for maintaining TCP connection. */
/*void __ISR(_TIMER_2_VECTOR, IPL5SOFT) T2Interrupt(void) {

    hb_cnt++;
    if (hb_cnt == HB_COUNT) {

        if (tcp_connected)
            write_string(GSM_UART, "beat\r\n");

        hb_cnt = 0;
    }
    mT2ClearIntFlag();
}*/
/*if (!gsm_init())
        GSM_LED=0;
    if (!gsm_gprs_init())
        GSM_LED=0;
    GSM_LED = 1;

    //if (!gsm_tcp_connect("http://traclock.no-ip.biz/updates/", 8000))
    //    GSM_LED = 1;
    //else
    //    RFID_LED = 1;
    //if (!gsm_http_post_raw("Hello"))
    //    RFID_LED=1;
    if (gsm_set_http_url())
        GSM_LED=0;
    //if (gsm_http_post("Hello you\r\n"))
    //    RFID_LED=1;

    //gsm_http_post("m=Hello");
    //if (gsm_http_post("m=Hello you!\n\n"))
    //    GSM_LED = 0;


    //if (!gsm_http_post("Hello world"))
    //    RFID_LED=0;

    // Initialize the GSM module:
    //     1. Turn on and sync baudrate.
    //     2. Bring up the tcp connection.
    if (gsm_init() || gsm_tcp_connect()) {
        gsm_pwr_off();
        pic_reset();
    }
    tcp_connected = 1;
    delay_ms(5000);
    GSM_LED = 1;*/

    // Initialize the RFID reader.
    //rfid_init_uart(); // init uart here so we don't interrupt gsm init.
    //if (rfid_init())
    //    pic_reset();
    //delay_ms(3000);
    //RFID_LED = 1;

    //delay_ms(3000);*/