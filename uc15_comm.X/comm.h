#include <plib.h>

#ifndef COMM_H
#define COMM_H

#define UC15_BAUDRATE 115200    //38400//9600//230400
#define SERIAL_BAUDRATE 115200
#define ALIEN_BAUDRATE 115200

#define UC15_UART (UART1)
#define UC15_UART_VEC (_UART_1_VECTOR)
#define UC15_RX_INT (INT_U1RX)
#define UC15_INT_VEC (INT_UART_1_VECTOR)

// Set for UART2 Usage
#define SERIAL_UART (UART2)
#define SERIAL_UART_VEC (_UART_2_VECTOR)
#define SERIAL_RX_INT (INT_U2RX)
#define SERIAL_INT_VEC (INT_UART_2_VECTOR)

#define ALIEN_UART (UART5)
#define ALIEN_UART_VEC (_UART_5_VECTOR)
#define ALIEN_RX_INT (INT_U5RX)
#define ALIEN_INT_VEC (INT_UART_5_VECTOR)

// Functions for uart communication.
void init_uc15_uart(void);
void init_serial_uart(void);
void init_alien_uart(void);
void delay_ms(long int);

#endif /* COMM_H */
