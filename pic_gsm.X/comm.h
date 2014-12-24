#include "picsetup.h"

#ifndef COMM_H
#define COMM_H

#define GSM_BAUDRATE 115200
#define ALIEN_BAUDRATE 115200

#define GSM_UART (UART1)
#define GSM_UART_VEC (_UART_1_VECTOR)
#define GSM_RX_INT (INT_U1RX)
#define GSM_INT_VEC (INT_UART_1_VECTOR)

#define RFID_UART (UART2)
#define RFID_UART_VEC (_UART_2_VECTOR)
#define RFID_RX_INT (INT_U2RX)
#define RFID_INT_VEC (INT_UART_2_VECTOR)

void delay_ms(unsigned int);
void gsm_init_uart(void);
void rfid_init_uart(void);
void write_string(UART_MODULE id, char *string);
void put_character(UART_MODULE id, char character);
void read_uart(UART_MODULE id, char * message, int maxLength);

#endif /* COMM_H */
