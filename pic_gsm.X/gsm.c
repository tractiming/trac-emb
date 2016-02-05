#include <string.h>
#include <stdlib.h>
#include "picsetup.h"
#include "gsm.h"
#include "comm.h"

GsmState gsm_state;
unsigned gsm_on;

//const char apn[] = "Internetd.gdsp"; //vodafone
const char apn[] = "apn.konekt.io"; //konekt
const char split_endpoint[] = "https://trac-us.appspot.com/api/splits/";
const char time_endpoint[] = "https://trac-us.appspot.com/api/updates/";

const char http_header[] = "POST /api/splits/ HTTP/1.1\r\n"
                           "Host: trac-us.appspot.com\r\n"
                           "Authorization: Bearer rwtOjDavSaqaBqp5J79tNVdwj96Zoe\r\n"
                           "Accept: */*\r\n"
                           "User-Agent: QUECTEL_MODULE\r\n"
                           "Connection: Keep-Alive\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n\r\n";
char post_content[GSM_HEADER_LEN+GSM_MAX_HTTP_LEN];

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
        if (chk1) {
                return 1;
        } else {
                // If not, the word might be wrapped around the end.
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

        else if (compare_response(s, "GET:"))
                s->resp = GSM_GET;

}

/* Wait for the GSM to give response. Timeout if not received. */
static int gsm_wait_for_response(GsmState *s, GsmResponse resp, unsigned timeout)
{
        s->resp = GSM_NONE;

        WriteCoreTimer(0);
        while (1) {
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
                            const char *command, unsigned timeout)
{
        gsm_clear_buffer(s);
        write_string(GSM_UART, command);
        return gsm_wait_for_response(s, response, timeout);
}

/* Send multiple commands to the GSM with a delay between commands. */
static int gsm_send_multiple(GsmState *s, GsmResponse response,
                             const char **cmds, int n, unsigned timeout,
                             unsigned delay)
{
        int i;
        for (i=0; i<n; i++) {
                if(gsm_send_command(s, response, cmds[i], timeout))
                        return -1;
                delay_ms(delay);
        }

        return 0;
}

/* Turn on the gsm by toggling power pin (pulse high for > 0.1 sec). */
void gsm_pwr_on(void)
{
        delay_ms(1000);
        POWERKEY = 1;
        delay_ms(250);
        POWERKEY = 0;
        delay_ms(8000);
        gsm_on = 1;
}

/* Turn off the gsm by toggling power pin. There are two options for powering
 * down. The first is by toggling the powerkey, and the second is by issuing
 * a software command. We use the latter approach here.
 */
int gsm_pwr_off(GsmState *s)
{
        int err;
        err = gsm_send_command(s, GSM_PWR_DOWN, "AT+QPOWD=1\r", GSM_TIMEOUT);
        delay_ms(1000);
        gsm_on = 0;
        return err;
}

/* GSM hardware shutdown. */
void gsm_pwr_off_hard(void)
{
        delay_ms(250);
        POWERKEY = 1;
        delay_ms(1000);
        POWERKEY = 0;
        delay_ms(2000);
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

/* Configure an SSL context for making HTTPS requests. */
static int gsm_cfg_ssl(GsmState *s)
{
        const char * cmds[] = {
                "AT+QHTTPCFG=\"sslctxid\",1\r",
                "AT+QSSLCFG=\"sslversion\",1,1\r",
                "AT+QSSLCFG=\"ciphersuite\",1,0X0005\r",
                "AT+QSSLCFG=\"seclevel\",1,0\r"
        };

        if (gsm_send_multiple(s, GSM_OK, cmds, 4, GSM_TIMEOUT, 200))
                return -1;
        delay_ms(250);
        return 0;
}

/* Send an HTTP POST request to the server. This assumes the http url is set.*/
int gsm_http_post(GsmState *s, char* data) {

        int ld = strlen(data);
        if (ld > GSM_MAX_HTTP_LEN)
                return -1;

        // Add the content length to the HTTP header. Combine header with data.
        sprintf(post_content, http_header, ld);
        strcat(post_content, data);

        // Note that the timeout for the message send needs to be less than or
        // equal to the amount of time we are waiting for a response. If it is
        // longer, the GSM will be stuck waiting for data even after the
        // function has returned. We do not care about the read time for right
        // now.
        char msg[100];
        int latency = GSM_TIMEOUT/2; // timeout is given in milliseconds
        int lt = strlen(post_content);
        sprintf(msg, "AT+QHTTPPOST=%i,%i,1\r", lt, latency/1000);

        if (gsm_send_command(s, GSM_CONNECT, msg, GSM_TIMEOUT))
                return -1;

        delay_ms(250); // This pause is really important!

        return gsm_send_command(s, GSM_OK, post_content, GSM_TIMEOUT);
}

/* Set up HTTP header and splits url. */
int gsm_cfg_split_endpoint(GsmState *s)
{
        if (gsm_send_command(s, GSM_OK, "AT+QHTTPCFG=\"requestheader\",1\r",
                             GSM_TIMEOUT))
                return -1;
        delay_ms(200);

        if (gsm_set_http_url(s, split_endpoint))
                return -1;
        delay_ms(200);

        return 0;
}

/* Repeatedly send the "AT" command to sync baudrates. */
static int sync_baudrate(GsmState *s)
{
        gsm_clear_buffer(s);
        s->resp = GSM_NONE;

        WriteCoreTimer(0);
        while (s->resp != GSM_OK) {
                write_string(GSM_UART, "AT\r");
                delay_ms(1000);
                gsm_update_state(s);

                if (ReadCoreTimer() > 40000*2*GSM_TIMEOUT)
                        return -1;
        }
        delay_ms(1500);
        return 0;
}

/* Initialize GSM module. */
int gsm_init(GsmState *s)
{
        char msg[MAX_STR_LEN];

        gsm_pwr_on();
        if (sync_baudrate(s))
                return -1;

        if (gsm_send_command(s, GSM_OK, "AT+QHTTPCFG=\"contextid\",1\r",
                             GSM_TIMEOUT))
                return -2;
        delay_ms(200);

        sprintf(msg, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r", apn);
        if (gsm_send_command(s, GSM_OK, msg, GSM_TIMEOUT))
                return -3;
        delay_ms(200);

        // Bring up network connection.
        if (gsm_send_command(s, GSM_OK, "AT+QIACT=1\r", GSM_TIMEOUT))
                return -4;
        delay_ms(5000);

        if (gsm_cfg_ssl(s))
                return -5;

        return 0;
}

int gsm_get_time(GsmState *s, char *ctime)
{
    if (gsm_set_http_url(s, time_endpoint))
        return -1;
    delay_ms(250);

    // Query the server for the current time.
    char str[GSM_BUFFER_LEN+1];
    gsm_send_command(s, GSM_GET, "AT+QHTTPGET\r", GSM_TIMEOUT);
    delay_ms(100);
    gsm_send_command(s, GSM_OK, "AT+QHTTPREAD\r", GSM_TIMEOUT);
    delay_ms(100);
    strncpy(str, s->buf, (s->indx+1));

    // Parse the response for the time.
    char *ts = str;
    int sv=0, j=0;
    char tms[50];
    while (*ts)
    {
        if (*ts == ']')
            sv = 0;

        if (sv)
            tms[j++] = *ts;

        if (*ts == '[')
            sv = 1;

        ts++;
    }
    tms[j] = '\0';

    int yr, mon, day, hr, min, sec;
    sscanf(tms, "\"%d-%2d-%2d %2d:%2d:%2d\"", &yr, &mon, &day, &hr, &min, &sec);
    sprintf(ctime, "%d/%02d/%02d %02d:%02d:%02d", yr, mon, day, hr, min, sec);

    return 0;

}
