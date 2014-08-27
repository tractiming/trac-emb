#ifndef GSM_H
#define	GSM_H

#define GSM_BUFFER_LEN 25
#define GSM_TIMEOUT 10000
#define GSM_RETRY_ATT 2
#define GSM_MAX_HTTP_LEN 100

typedef enum {
    GSM_NONE = -1,
    GSM_OK = 0,
    GSM_READY = 1,
    GSM_ERROR = 2,
    GSM_UNREAD = 3,
    GSM_CONNECT = 4,
    GSM_CLOSED = 5,
    GSM_PWR_DOWN = 6,
} GSMResponse;

void delay_ms(unsigned int);
void gsm_update_state(char);
void gsm_pwr_on(void);
void gsm_pwr_off(void);
int gsm_init(void);
int gsm_tcp_connect(const char *, int);
GSMResponse gsm_get_response(void);
int gsm_send_command(char*, GSMResponse, unsigned);
int gsm_tcp_send_data(char*);
unsigned gsm_get_data_mode(void);
unsigned gsm_chk_tcp_conn(void);
int gsm_tcp_reset(void);
int gsm_gprs_init(void);
int gsm_set_http_url(void);
int gsm_http_post(char*);

#endif	/* GSM_H */

