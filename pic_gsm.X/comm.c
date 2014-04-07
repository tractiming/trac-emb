#include <stdio.h>
#include <plib.h>
#include <string.h>
#include "comm.h"
#include "gsm.h"

#define SYS_FREQ 40000000L
char message[100];

/* Sets up UART for communication with GSM. By default it is configured with
 * an RX interrupt.
 */
void uart2_init(void) {

    UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY );
    UARTSetFifoMode(UART2, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART2, SYS_FREQ, RS232_USB_BAUDRATE);
    UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    INTEnable(INT_U2RX, INT_ENABLED);
    INTSetVectorPriority(INT_UART_2_VECTOR, INT_PRIORITY_LEVEL_7);
    INTSetVectorSubPriority(INT_UART_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
};

void uart1_init(void) {

    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY );
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, SYS_FREQ, RS232_USB_BAUDRATE);
    UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
};

/* Write a string over the serial port. */
void write_string(UART_MODULE id, const char *string) {

  while (*string != '\0') {
    while (!UARTTransmitterIsReady(id));
    UARTSendDataByte(id, (char) *string);
    string++;
    while (!UARTTransmissionHasCompleted(id));
  }
};

/* Put a character over the serial port. */
void put_character(UART_MODULE id, const char character) {
  while (!UARTTransmitterIsReady(id));
  UARTSendDataByte(id, character);
  while (!UARTTransmissionHasCompleted(id));
};

/* Read the data from UART2. */
void read_uart2(char * message, int maxLength) {
  char data;
  int complete = 0, num_bytes = 0;
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (UARTReceivedDataIsAvailable(UART2)) {
      data = UARTGetDataByte(UART2);
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        message[num_bytes] = data;
        num_bytes++;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  message[num_bytes] = '\0';
};