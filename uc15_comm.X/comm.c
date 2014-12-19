#include <stdio.h>
#include <plib.h>
#include <string.h>
#include "NU32.h"
#include "comm.h"

char message[100];

/* Sets up UART for communication with GSM. By default it is configured with
 * an RX interrupt.
 */
void init_uc15_uart(void) {

    UARTConfigure(UC15_UART, UART_ENABLE_PINS_TX_RX_ONLY );
    UARTSetFifoMode(UC15_UART, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UC15_UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UC15_UART, SYS_FREQ, UC15_BAUDRATE);
    UARTEnable(UC15_UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    INTEnable(UC15_RX_INT, INT_ENABLED);
    INTSetVectorPriority(UC15_INT_VEC, INT_PRIORITY_LEVEL_6);
    INTSetVectorSubPriority(UC15_INT_VEC, INT_SUB_PRIORITY_LEVEL_0);
};

/* Initialize RFID communication. */
void init_serial_uart(void) {

    UARTConfigure(SERIAL_UART, UART_ENABLE_PINS_TX_RX_ONLY );
    UARTSetFifoMode(SERIAL_UART, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(SERIAL_UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(SERIAL_UART, SYS_FREQ, SERIAL_BAUDRATE);
    UARTEnable(SERIAL_UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    INTEnable(SERIAL_RX_INT, INT_ENABLED);
    INTSetVectorPriority(SERIAL_INT_VEC, INT_PRIORITY_LEVEL_7);
    INTSetVectorSubPriority(SERIAL_INT_VEC, INT_SUB_PRIORITY_LEVEL_0);
};

/* Initialize Alien communication. */
void init_alien_uart(void) {

    UARTConfigure(ALIEN_UART, UART_ENABLE_PINS_TX_RX_ONLY );
    UARTSetFifoMode(ALIEN_UART, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(ALIEN_UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(ALIEN_UART, SYS_FREQ, ALIEN_BAUDRATE);
    UARTEnable(ALIEN_UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    INTEnable(ALIEN_RX_INT, INT_ENABLED);
    INTSetVectorPriority(ALIEN_INT_VEC, INT_PRIORITY_LEVEL_6);
    INTSetVectorSubPriority(ALIEN_INT_VEC, INT_SUB_PRIORITY_LEVEL_0);
};

/* Delay for a given number of msecs. */
void delay_ms(long int msec)
{
    long int delay = (SYS_FREQ/2000)*msec;
    WriteCoreTimer(0);
    while (ReadCoreTimer()<delay);
};
