#ifndef GSM_H
#define	GSM_H

#define GSM_BUFFER_LEN 15
#define GSM_TIMEOUT 10000

typedef enum {
    GSM_NONE = -1,
    GSM_OK = 0,
    GSM_READY = 1,
    GSM_ERROR = 2,
    GSM_UNREAD = 3,
    GSM_CONNECT = 4,
    GSM_CLOSED = 5,
} GSMResponse;

void delay_ms(unsigned int);
void gsm_update_state(char);
void gsm_pwr_on(void);
void gsm_pwr_off(void);
int gsm_init(void);
int gsm_tcp_connect(void);
GSMResponse gsm_get_response(void);
int gsm_send_command(char*, GSMResponse, unsigned);
int gsm_tcp_send_data(char*);
unsigned gsm_get_data_mode(void);

#endif	/* GSM_H */

