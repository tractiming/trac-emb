#include "picsetup.h"

#ifndef COMM_H
#define COMM_H

#define GSM_BAUDRATE  115200
#define RFID_BAUDRATE 115200

#define GSM_UART      UART1
#define GSM_UART_VEC  _UART_1_VECTOR
#define GSM_RX_INT    INT_U1RX
#define GSM_INT_VEC   INT_UART_1_VECTOR

#define RFID_UART     UART2
#define RFID_UART_VEC _UART_2_VECTOR
#define RFID_RX_INT   INT_U2RX
#define RFID_INT_VEC  INT_UART_2_VECTOR

void delay_ms(unsigned int);
void uart_init(UART_MODULE, long int, INT_SOURCE, INT_VECTOR,
               INT_PRIORITY, INT_SUB_PRIORITY);
void write_string(UART_MODULE, char*);
void put_character(UART_MODULE, const char);

#endif /* COMM_H */