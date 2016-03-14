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

/* Set up UART port with default configuration (RX interrupt). */
void uart_init(UART_MODULE id, long int baudrate, INT_SOURCE src,
               INT_VECTOR vec, INT_PRIORITY pr, INT_SUB_PRIORITY sub_pr)
{
        UARTConfigure(id, UART_ENABLE_PINS_TX_RX_ONLY);
        UARTSetFifoMode(id,
                        UART_INTERRUPT_ON_TX_DONE |
                        UART_INTERRUPT_ON_RX_NOT_EMPTY);
        UARTSetLineControl(id,
                           UART_DATA_SIZE_8_BITS |
                           UART_PARITY_NONE | UART_STOP_BITS_1);
        UARTSetDataRate(id, SYS_FREQ, baudrate);
        UARTEnable(id,
                   UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

        INTEnable(src, INT_ENABLED);
        INTSetVectorPriority(vec, pr);
        INTSetVectorSubPriority(vec, sub_pr);
}

/* Write a string over a serial port. */
void write_string(UART_MODULE id, const char *string)
{
        while (*string != '\0') {
                while (!UARTTransmitterIsReady(id));
                UARTSendDataByte(id, (char) *string);
                string++;
                while (!UARTTransmissionHasCompleted(id));
        }
}

void put_character(UART_MODULE id, const char character) {
        while (!UARTTransmitterIsReady(id));
        UARTSendDataByte(id, character);
        while (!UARTTransmissionHasCompleted(id));
}