#include <stdio.h>
#include <string.h>
#include "picsetup.h"
#include "comm.h"

/* Delay for a given number of msecs. */
void delay_ms(unsigned int msec)
{
    unsigned int delay = (SYS_FREQ/2000)*msec;
    WriteCoreTimer(0);
    while (ReadCoreTimer()<delay);
}

/* Set up UART for communication with GSM. Configure with RX interrupt. */
void gsm_init_uart(void) {

    UARTConfigure(GSM_UART, UART_ENABLE_PINS_TX_RX_ONLY );
    UARTSetFifoMode(GSM_UART, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(GSM_UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(GSM_UART, SYS_FREQ, GSM_BAUDRATE);
    UARTEnable(GSM_UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    INTEnable(GSM_RX_INT, INT_ENABLED);
    INTSetVectorPriority(GSM_INT_VEC, INT_PRIORITY_LEVEL_6);
    INTSetVectorSubPriority(GSM_INT_VEC, INT_SUB_PRIORITY_LEVEL_0);
};

/* Set up UART for communication with Alien. Configure with RX interrupt. */
void rfid_init_uart(void) {

    UARTConfigure(RFID_UART, UART_ENABLE_PINS_TX_RX_ONLY );
    UARTSetFifoMode(RFID_UART, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(RFID_UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(RFID_UART, SYS_FREQ, ALIEN_BAUDRATE);
    UARTEnable(RFID_UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    INTEnable(RFID_RX_INT, INT_ENABLED);
    INTSetVectorPriority(RFID_INT_VEC, INT_PRIORITY_LEVEL_6);
    INTSetVectorSubPriority(RFID_INT_VEC, INT_SUB_PRIORITY_LEVEL_1);
};

/* Write a string over the serial port. */
void write_string(UART_MODULE id, char *string) {
  while (*string != '\0') {
    while (!UARTTransmitterIsReady(id));
    UARTSendDataByte(id, (char) *string);
    string++;
    while (!UARTTransmissionHasCompleted(id));
  }
};

/* Put a character over the serial port. */
void put_character(UART_MODULE id, char character) {
  while (!UARTTransmitterIsReady(id));
  UARTSendDataByte(id, character);
  while (!UARTTransmissionHasCompleted(id));
};

/* Read the data from UART. */
void read_uart(UART_MODULE id, char * message, int max_len) {
  char data;
  int complete = 0, num_bytes = 0;

  while (!complete) {
    if (UARTReceivedDataIsAvailable(id)) {
      data = UARTGetDataByte(id);
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      }
      else {
        message[num_bytes] = data;
        num_bytes++;
        
        if (num_bytes >= max_len) {
          num_bytes = 0;
        }
      }
    }
  }
  
  message[num_bytes] = '\0';
};
