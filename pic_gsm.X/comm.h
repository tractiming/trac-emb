#ifndef COMM_H
#define COMM_H

#define RS232_USB_BAUDRATE 115200    //38400//9600//230400

// Functions for pic/gsm communication.
void uart2_init(void);
void write_string(UART_MODULE id, const char *string);
void put_character(UART_MODULE id, const char character);
void read_uart2(char * message, int maxLength);

#endif /* COMM_H */