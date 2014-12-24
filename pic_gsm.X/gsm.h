#ifndef GSM_H
#define	GSM_H

#define GSM_BUFFER_LEN 25
#define GSM_TIMEOUT 10000
#define GSM_MAX_HTTP_LEN 150
#define NEXT_GSM_INDX(i) ((i+1) % GSM_BUFFER_LEN)

typedef enum
{
    GSM_NONE = -1,
    GSM_OK = 0,
    GSM_READY = 1,
    GSM_ERROR = 2,
    GSM_UNREAD = 3,
    GSM_CONNECT = 4,
    GSM_CLOSED = 5,
    GSM_PWR_DOWN = 6,
    GSM_CME = 7,
} GsmResponse;

typedef struct
{
    char buf[GSM_BUFFER_LEN];
    int indx;
    GsmResponse resp;
} GsmState;

extern const char apn[];
extern const char post_domain_name[];
extern GsmState gsm_state;
extern unsigned gsm_on;

void gsm_add_to_buffer(GsmState *, char);
int gsm_init(GsmState *);
int gsm_http_post(GsmState *, char *);
void gsm_pwr_on(void);
int gsm_pwr_off(GsmState *);

#endif	/* GSM_H */
