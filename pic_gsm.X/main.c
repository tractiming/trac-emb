#define SETUP
#include "picsetup.h"
#include "comm.h"
#include "gsm.h"
#include "rfid.h"

#define FULL_DEMO

// Unique reader id for this device.
const char reader_id[] = "A1010"; 

int main(void) {

    // Configure PIC and I/O.
    CFGCONbits.JTAGEN = 0;
    SYSTEMConfigPerformance(SYS_FREQ);
    setup_pins();
    GSM_LED = 0; RFID_LED = 0;

    // Configure interrupts for shutdown and uart communication.
    setup_shutdown_int();
    gsm_init_uart();
    rfid_init_uart();
    INTEnableSystemMultiVectoredInt();
    delay_ms(5000);

    // Initialize the GSM module. On failure, wait to be reset.
    if (gsm_init(&gsm_state))
        while(1);
    GSM_LED = 1;

    // Inititialize the rfid reader.
    //rfid_init();
    RFID_LED = 1;

    char post_msg[MAX_STR_LEN];
    while (1) {

#ifdef FULL_DEMO

        // Parse any new splits.
        update_splits(&rfid_split_queue, &rfid_line_buffer);

        // Post any new data to the server.
        while (get_next_split_msg(&rfid_split_queue, reader_id, post_msg))
        {
            gsm_http_post(&gsm_state, post_msg);
            delay_ms(1000);
        }

        delay_ms(150);

#endif

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
void __ISR(RFID_UART_VEC, IPL6SOFT) IntRFIDUartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(RFID_UART)))
  {

      // Add the next character to the serial buffer.
      char data = UARTGetDataByte(RFID_UART);
      rfid_add_to_buffer(&rfid_line_buffer, data);

      // Clear the RX interrupt flag.
      INTClearFlag(INT_SOURCE_UART_RX(RFID_UART));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(RFID_UART)))
  {
      INTClearFlag(INT_SOURCE_UART_TX(RFID_UART));
  }
}

/* Shutdown ISR. */
void __ISR(_EXTERNAL_3_VECTOR, IPL5SOFT) ShutdownISR(void) {
    
    // Send shutoff signal to GSM.
    GSM_LED = 0;
    if (gsm_on)
        gsm_pwr_off(&gsm_state);

    // Send KILL signal to timer. (Not implemented.)

    // Disable the other UART interrupts, stopping all communication.
    INTEnable(RFID_RX_INT, INT_DISABLED);
    INTEnable(GSM_RX_INT, INT_DISABLED);

    mINT3ClearIntFlag();
}
