#include <plib.h>

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

//#define HB_COUNT 150 // send heartbeat every 15 sec

// Functions for uart communication.
void delay_ms(int);
void gsm_init_uart(void);
void rfid_init_uart(void);
void write_string(UART_MODULE id, char *string);
void put_character(UART_MODULE id, char character);
void read_uart(UART_MODULE id, char * message, int maxLength);
//void hb_tmr_init(void);

#endif /* COMM_H */
