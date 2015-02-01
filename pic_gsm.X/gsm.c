#include <string.h>
#include <stdlib.h>
#include "picsetup.h"
#include "gsm.h"
#include "comm.h"

GsmState gsm_state;
unsigned gsm_on;

const char apn[] = "Internetd.gdsp";
const char post_domain_name[] = "http://trac-us.appspot.com/api/updates/";

static void gsm_clear_buffer(GsmState *s)
{
    s->indx = 0;
    memset(s->buf, 0, GSM_BUFFER_LEN*sizeof(char));
}

void gsm_add_to_buffer(GsmState *s, char c)
{
    s->buf[s->indx] = c;
    s->indx = NEXT_GSM_INDX(s->indx);
}

static int compare_response(GsmState *s, const char *str)
{
    // TODO: optimize this since it is used very frequently.
    /* Note: here we do separate checks for the token in the wrapped and 
     * unwrapped versions of the buffer. 
     */

    // Check if word exists unwraped in buffer.
    char *chk1 = strstr(s->buf, str);
    if (chk1)
        return 1;

    // If not, the word might be wrapped around the end.
    else
    {
        char unwrap[2*GSM_BUFFER_LEN];
        strcpy(unwrap, s->buf);
        strcat(unwrap, s->buf);
        char *chk2 = strstr(unwrap, str);
        if (chk2)
            return 1;
    }
    
    return 0;
}

/* Update the state of the gsm module by comparing buffer contents to keywords. */
static void gsm_update_state(GsmState *s)
{
    if (compare_response(s, "OK"))
        s->resp = GSM_OK;

    else if (compare_response(s, "CONNECT"))
        s->resp = GSM_CONNECT;

    else if (compare_response(s, "ERROR"))
        s->resp = GSM_ERROR;

    else if (compare_response(s, "CLOSED"))
        s->resp = GSM_CLOSED;

    else if (compare_response(s, "DOWN"))
        s->resp = GSM_PWR_DOWN;

}

/* Wait for the GSM to give response. Timeout if not received. */
static int gsm_wait_for_response(GsmState *s, GsmResponse resp, unsigned timeout)
{
    s->resp = GSM_NONE;

    WriteCoreTimer(0);
    while (1)
    {
        gsm_update_state(s);

        if (s->resp == resp)
            return 0;

        else if (s->resp == GSM_ERROR)
            return -1;

        if (ReadCoreTimer() > timeout*20000)
            return -2;
    }

}

/* Send command to GSM and wait for response or timeout. */
static int gsm_send_command(GsmState *s, GsmResponse response,
                            char *command, unsigned timeout)
{
    gsm_clear_buffer(s);
    write_string(GSM_UART, command);
    return gsm_wait_for_response(s, response, timeout);
}

/* Turn on the gsm by toggling power pin. */
void gsm_pwr_on(void)
{
    // To turn on, send high pulse for >0.1 sec. Delay to ensure module starts.
    delay_ms(1000);
    POWERKEY = 1;
    delay_ms(250);
    POWERKEY = 0;
    delay_ms(8000);
    gsm_on = 1;
}

/* Turn off the gsm by toggling power pin. */
int gsm_pwr_off(GsmState *s)
{
    // There are two options for powering down. The first is by toggling the
    // powerkey, and the second is by issuing a software command. We use the
    // latter approach here.
    int err = gsm_send_command(s, GSM_PWR_DOWN, "AT+QPOWD=1\r", GSM_TIMEOUT);
    delay_ms(1000);
    gsm_on = 0;
    return err;
}

/* Set the HTTP url to which the GSM sends data. */
static int gsm_set_http_url(GsmState *s, const char *url) {

    int len = strlen(url);
    char msg[100];
    sprintf(msg, "AT+QHTTPURL=%i,%i\r", len, 15);

    if (gsm_send_command(s, GSM_CONNECT, msg, 5*GSM_TIMEOUT))
        return -1;
    delay_ms(100);

    write_string(GSM_UART, (char *) url);
    return gsm_wait_for_response(s, GSM_OK, GSM_TIMEOUT);
}

/* Sends an HTTP POST request to the server. This assumes the http url is set.*/
int gsm_http_post(GsmState *s, char* data) {

    int len = strlen(data);
    if (len > GSM_MAX_HTTP_LEN)
        return -1;

    char msg[100];
    // Note that the timeout for the message send needs to be less than or equal
    // to the amount of time we are waiting for a response. If it is longer, the
    // GSM will be stuck waiting for data even after the function has returned.
    // We do not care about the read time for right now. Note that the GSM
    // timeout is given in milliseconds.
    int latency = GSM_TIMEOUT/2;
    sprintf(msg, "AT+QHTTPPOST=%i,%i,5\r", len, latency/1000);

    if (gsm_send_command(s, GSM_CONNECT, msg, GSM_TIMEOUT))
        return -1;

    delay_ms(150); // This pause is really important!

    return gsm_send_command(s, GSM_OK, data, GSM_TIMEOUT);
}

/* Initialize GSM module. */
int gsm_init(GsmState *s)
{
    // Pull powerkey low to turn on module.
    gsm_pwr_on();

    // Reset buffer and state.
    gsm_clear_buffer(s);
    s->resp = GSM_NONE;

    // Send "AT" command to sync baudrates.
    WriteCoreTimer(0);
    while (s->resp != GSM_OK)
    {
        write_string(GSM_UART, "AT\r");
        delay_ms(1000);
        gsm_update_state(s);

        // If no response, return failure.
        if (ReadCoreTimer() > 40000*2*GSM_TIMEOUT)
            return -1;
    }
    delay_ms(1500);

    if (gsm_send_command(s, GSM_OK, "AT+QHTTPCFG=\"contextid\",1\r", GSM_TIMEOUT))
        return -2;
    delay_ms(200);

    // Set APN.
    char msg[MAX_STR_LEN];
    sprintf(msg, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r", apn);
    if (gsm_send_command(s, GSM_OK, msg, GSM_TIMEOUT))
        return -3;
    delay_ms(200);

    // Bring up network connection.
    if (gsm_send_command(s, GSM_OK, "AT+QIACT=1\r", GSM_TIMEOUT))
        return -4;
    delay_ms(5000);

    // Set HTTP URL.
    if (gsm_set_http_url(s, post_domain_name))
        return -5;
    delay_ms(200);

    return 0;
}
