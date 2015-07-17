#define NU32_STANDALONE
#include "NU32.h"
#include "comm.h"
#include "rfid.h"
#include "gsm.h"
//#include <plib.h>

#define MAX_MESSAGE_LENGTH 300
const char reader_id[] = "A1010"; // Unique reader id for this device.
// 3.3 = uc15
// GND = alien


/* Interrupt for handling uart communication with gsm module. */
void __ISR(UC15_UART_VEC, IPL6SOFT) IntUC15UartHandler(void) {

  // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(UC15_UART))) {

    char data = UARTGetDataByte(UC15_UART);
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
      //NU32_WriteUART1(data);
      //NU32_WriteUART1("Inside the interrupt");
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

       
    init_uc15_uart();
    init_serial_uart();
    //WriteString(SERIAL_UART, "Hello World\r\n");

    while(1)
    {
        NU32LED1 = 1;
        NU32LED2 = 0;
        delay_ms(500);
        NU32LED1 = 0;
        NU32LED2 = 1;
        delay_ms(500);

        //WriteString(SERIAL_UART, "Serial Hello World\r\n");
        //NU32_WriteUART1("UART1 Hello World\r\n");
    }

    

    return 0;
}


