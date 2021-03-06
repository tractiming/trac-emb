#ifndef GSM_H
#define	GSM_H

#define GSM_BUFFER_LEN     100
#define GSM_TIMEOUT      11000
#define GSM_MAX_HTTP_LEN  3000
#define GSM_HEADER_LEN     250
#define NEXT_GSM_INDX(i) ((i+1) % GSM_BUFFER_LEN)

#define GSM_LOW_SIGNAL      12

typedef enum {
        GSM_NONE     = -1,
        GSM_OK       =  0,
        GSM_READY    =  1,
        GSM_ERROR    =  2,
        GSM_UNREAD   =  3,
        GSM_CONNECT  =  4,
        GSM_CLOSED   =  5,
        GSM_PWR_DOWN =  6,
        GSM_CME      =  7,
        GSM_GET      =  8,
        GSM_READ     =  9
} GsmResponse;

typedef struct {
        char buf[GSM_BUFFER_LEN];
        int indx;
        GsmResponse resp;
} GsmState;

extern const char apn[];
extern const char split_endpoint[];
extern GsmState gsm_state;
extern unsigned gsm_on;

void gsm_add_to_buffer(GsmState *, char);
int gsm_init(GsmState *);
int gsm_http_post(GsmState *, char *);
void gsm_pwr_on(void);
int gsm_pwr_off(GsmState *);
void gsm_pwr_off_hard(void);
int gsm_get_time(GsmState *, char*, int);
int gsm_cfg_split_endpoint(GsmState *);
int gsm_get_signal_strength(GsmState *);

#endif	/* GSM_H */
