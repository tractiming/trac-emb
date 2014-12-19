#define NU32_STANDALONE
#include "NU32.h"
#include "comm.h"
#include "rfid.h"
#include "gsm.h"
#include <plib.h>

#define MAX_MESSAGE_LENGTH 300
const char reader_id[] = "A1010"; // Unique reader id for this device.
// 3.3 = uc15
// GND = alien


/* Interrupt for handling uart communication with gsm module. */
void __ISR(UC15_UART_VEC, IPL6SOFT) IntUC15UartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(UC15_UART))) {

    char data = UARTGetDataByte(UC15_UART);
    gsm_add_to_buffer(&gsm_state, data);
    PutCharacter(SERIAL_UART, data);

    // Clear the RX interrupt flag.
    INTClearFlag(INT_SOURCE_UART_RX(UC15_UART));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(UC15_UART))) {
    INTClearFlag(INT_SOURCE_UART_TX(UC15_UART));
  }
}

/* Interrupt for handling uart communication with rfid reader. */
void __ISR(ALIEN_UART_VEC, IPL6SOFT) IntAlienUartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(ALIEN_UART)))
  {
      // Add the next character to the serial buffer.
      char data = UARTGetDataByte(ALIEN_UART);
      add_char_to_buffer(&rfid_line_buffer, data);
      //PutCharacter(SERIAL_UART, data);

      // Clear the RX interrupt flag.
      INTClearFlag(INT_SOURCE_UART_RX(ALIEN_UART));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(ALIEN_UART)))
  {
      INTClearFlag(INT_SOURCE_UART_TX(ALIEN_UART));
  }
}

/* Interrupt for handling uart communication with rfid reader. */
void __ISR(SERIAL_UART_VEC, IPL7SOFT) IntSerialUartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(SERIAL_UART)))
  {

      // Add the next character to the serial buffer.
      char data = UARTGetDataByte(SERIAL_UART);
      //if (PORTAbits.RA2)
      PutCharacter(UC15_UART, data);
      //else
      //PutCharacter(ALIEN_UART, data);

      // Clear the RX interrupt flag.
      INTClearFlag(INT_SOURCE_UART_RX(SERIAL_UART));
  }

  // We don't care about TX interrupt.
  if (INTGetFlag(INT_SOURCE_UART_TX(SERIAL_UART)))
  {
      INTClearFlag(INT_SOURCE_UART_TX(SERIAL_UART));
  }
}

int main(void) {

    char message[MAX_MESSAGE_LENGTH];

    NU32_Startup();

    TRISAbits.TRISA2 = 1;
    
    delay_ms(1000);
    TRISDbits.TRISD0 = 0;
    LATDbits.LATD0 = 1;
    delay_ms(200);
    LATDbits.LATD0 = 0;
    delay_ms(10000);

    // Set up serial communication.
    NU32_EnableUART1Interrupt();
    U2STAbits.URXEN = 1;
    U2STAbits.UTXEN = 1;
    init_uc15_uart();

    U5STAbits.URXEN = 1;
    U5STAbits.UTXEN = 1;
    init_alien_uart();

    clear_queue(&rfid_split_queue);
    clear_buffer(&rfid_line_buffer);

    // Reset buffer and state.
    gsm_clear_buffer(&gsm_state);
    gsm_state.resp = GSM_NONE;

    delay_ms(2000);
    gsm_send_command(&gsm_state, GSM_OK, "AT\r", GSM_TIMEOUT);
    delay_ms(200);
    gsm_send_command(&gsm_state, GSM_OK, "AT+QHTTPCFG=\"contextid\",1\r", GSM_TIMEOUT);
    delay_ms(200);
    gsm_send_command(&gsm_state, GSM_OK, "AT+QICSGP=1,1,\"ATT.MVNO\",\"\",\"\",1\r", GSM_TIMEOUT);
    delay_ms(200);
    gsm_send_command(&gsm_state, GSM_OK, "AT+QIACT=1\r", GSM_TIMEOUT);
    delay_ms(5000);
    gsm_set_http_url(&gsm_state, post_domain_name);
    delay_ms(200);
    gsm_http_post(&gsm_state, "testing");


    while(1) {

        //if (!NU32USER) {
        //    delay_ms(250);
        //    //gsm_send_command(&gsm_state, GSM_OK, "AT\r", GSM_TIMEOUT);
        //    gsm_http_post(&gsm_state, "test");

        //}

        
        //gsm_update_state(&gsm_state);
        //if (gsm_state.resp == GSM_OK) {
        //    WriteString(SERIAL_UART, "hooray!\r\n");
        //    gsm_state.resp = GSM_NONE;
        //    gsm_clear_buffer(&gsm_state);
        //}
        update_splits(&rfid_split_queue, &rfid_line_buffer);
        
        while (!queue_is_empty(&rfid_split_queue))
        {
            pop_split_from_queue(&rfid_split_queue, message);
            strcat(message, "&r=");
            strcat(message, reader_id);
            gsm_http_post(&gsm_state, message);
            delay_ms(1000);
            //gsm_http_post(s, msg);
            //WriteString(SERIAL_UART, message);
            //WriteString(SERIAL_UART, "\r\n");
        }

        delay_ms(150);
         

    }
    return 0;
}


