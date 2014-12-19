#include <plib.h>
#include <string.h>
#include <stdlib.h>
#include "NU32.h"
#include "gsm.h"
#include "comm.h"

GsmState gsm_state;
const char apn[] = "ATT.MVNO";
//const char post_domain_name[] = "http://traclock.no-ip.biz:8000/api/updates/";
const char post_domain_name[] = "http://trac-us.appspot.com/api/updates/";

void gsm_clear_buffer(GsmState *s)
{
    s->indx = 0;
    memset(s->buf, 0, GSM_BUFFER_LEN*sizeof(char));
    //s->buf[0] = '\0';
}

void gsm_add_to_buffer(GsmState *s, char c)
{
    s->buf[s->indx] = c;
    s->indx = NEXT_GSM_INDX(s->indx);
}

int compare_response(GsmState *s, const char *str)
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

/* Update the state of the gsm module. */
void gsm_update_state(GsmState *s)
{
    /* For the most part, the module will return "OK" followed by line return
     * and new line. However, the http post function does not give the return/
     * new line. Therefore we explicitly only check for the two characters "OK".
     */
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


/* Delay for a given number of msecs. */
/*void delay_ms(long int msec)
{
    // It is important that we use long ints here because the clock frequency
    // is quite large and we want to allow for delays on the order of minutes.
    long int delay = (SYS_FREQ/2000)*msec;
    WriteCoreTimer(0);
    while (ReadCoreTimer()<delay);
}*/

/* Turn on the gsm by toggling power pin. */
//void gsm_pwr_on(void)
//{
//    // To turn on, we pull power pin low, wait 2.5 sec and then pull high.
//    POWERKEY = 0;
//    delay_ms(2500);
//    POWERKEY = 1;
//    delay_ms(200);
//}

/* Turn off the gsm by toggling power pin. */
// NOTE: The GSM2 click does not provide a pin for EMERG_OFF.
int gsm_pwr_off(GsmState *s)
{
    // There are two options for powering down. The first is by toggling the 
    //  powerkey, and the second is by issuing a software command. We use the 
    // latter approach here.
    int err = gsm_send_command(s, GSM_PWR_DOWN, "AT+QPOWD=1\r", GSM_TIMEOUT);
    delay_ms(1000);
    return err;
}

/* Checks if sim card is present. */
//int gsm_chk_sim(void) {
//
//    if (gsm_send_command("AT+CPIN?\r", GSM_OK, GSM_TIMEOUT))
//        return -1;
//
//    return 0;
//}

/* Wait for the GSM to give response. Timeout if not received. */
int gsm_wait_for_response(GsmState *s, GsmResponse resp, unsigned timeout)
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
int gsm_send_command(GsmState *s, GsmResponse response,
                     char *command, unsigned timeout)
{
    gsm_clear_buffer(s);
    WriteString(UC15_UART, command);//write_string(GSM_UART, command);
    return gsm_wait_for_response(s, response, timeout);
}


/* Brings up connection with the GPRS network. */
int gsm_gprs_init(GsmState *s, const char *apn) {

    int delay_time = 100;
    char msg[100];

    // Set the context 0 as FGCNT.
    if (gsm_send_command(s, GSM_OK, "AT+QIFGCNT=0\r", GSM_TIMEOUT))
        return -1;
    delay_ms(delay_time);

    // Set APN.
    strcpy(msg, "AT+QICSGP=1,\"");
    strcat(msg, apn);
    strcat(msg, "\"\r");
    if (gsm_send_command(s, GSM_OK, "AT+QICSGP=1,\"ATT.MVNO\"\r", GSM_TIMEOUT))
        return -1;
    delay_ms(delay_time);

    // Register the TCP/IP stack.
    if (gsm_send_command(s, GSM_OK, "AT+QIREGAPP\r", 10*GSM_TIMEOUT))
        return -1;

    /* The following pause is essential. We need to give the module some time
     * after registering the network and before asking for a gprs connection.
     * (3 sec does not seem to work, but 5 does.)
     */
    delay_ms(6000);

    // Activate FGCNT. Brings up the wireless connection.
    if (gsm_send_command(s, GSM_OK, "AT+QIACT\r", 20*GSM_TIMEOUT))
        return -1;
    delay_ms(delay_time);

    return 0;
}

/* Deactivate the GPRS. */
int gsm_grps_deact(GsmState *s)
{
    return gsm_send_command(s, GSM_OK, "AT+QIDEACT\r", 3*GSM_TIMEOUT);
}

/* Sets the HTTP url to send data to. */
int gsm_set_http_url(GsmState *s, const char *url) {

    int len = strlen(url);
    char msg[100];
    sprintf(msg, "AT+QHTTPURL=%i,%i\r", len, 20);

    if (gsm_send_command(s, GSM_CONNECT, msg, 5*GSM_TIMEOUT))
        return -1;
    delay_ms(500);

    WriteString(UC15_UART, (char *) url);
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

/* Establish TCP connection with server. This assumes that the GPRS is already
 * activated and the server is listening on the correct port.
 */
int gsm_tcp_connect(GsmState *s, const char* ip, int port) {

    char itoa_bfr[8];
    itoa(itoa_bfr, port, 10);
    char msg[100];

    strcpy(msg, "AT+QIOPEN=\"TCP\",\"");
    strcat(msg, ip);
    strcat(msg, "\",");
    strcat(msg, itoa_bfr);
    strcat(msg, "\r");

    if (gsm_send_command(s, GSM_OK, msg, 5*GSM_TIMEOUT))
        return -1;
    return 0;
}

/* Initialize GSM module. */
int gsm_init(GsmState *s)
{
    // Pull powerkey low to turn on module.
    //gsm_pwr_on();

    // Reset buffer and state.
    gsm_clear_buffer(s);
    s->resp = GSM_NONE;

    // Send "AT" command to sync baudrates.
    WriteCoreTimer(0);
    while (s->resp != GSM_OK)
    {
      //  write_string(GSM_UART, "AT\r");
        delay_ms(100);
        gsm_update_state(s);

        // If no response, return failure.
        if (ReadCoreTimer() > 40000*2*GSM_TIMEOUT)
            return -1;
    }
    delay_ms(1500);

    // Connect to the GPRS network.
    if (gsm_gprs_init(s, apn))
        return -2;
    delay_ms(1000);

    // Set the url for http posts.
    if (gsm_set_http_url(s, post_domain_name))
        return -3;
    delay_ms(1000);

    return 0;
}



// Closes the tcp connection and deactivates gprs.
/*int gsm_tcp_close(void) {

    // Switch back to AT command mode.
    if (gsm_data_mode) {
        //delay_ms(500);
        if (gsm_send_command("+++", GSM_OK, GSM_TIMEOUT))
            return -1;
        //delay_ms(500);
        gsm_data_mode = 0;
    }

    // Close the connection.
    if (gsm_send_command("AT+QICLOSE\r", GSM_OK, GSM_TIMEOUT))
        return -1;

}*/

/* Writes a message over tcp. */
/*int gsm_tcp_send_data(char *data) {

    // If we are not in data mode, we cannot write the message directly.
    if (gsm_data_mode != 1)
        return -1;

    // Write the message and terminating character.
    write_string(GSM_UART, data);
    write_string(GSM_UART, "\032");
    
    return 0;
    
}*/

/* Returns whether the gsm is in data or text mode. */
//unsigned gsm_get_data_mode(void) {
//    return gsm_data_mode;
//}

/* Returns 1 if the connection has been lost. */
//unsigned gsm_chk_tcp_conn(void) {
//    return gsm_tcp_lost;
//}

/* Resets the tcp connection after a failure. */
/*int gsm_tcp_reset(void) {

    // Assume the abnormaility was caught by the "closed" command,
    // then the gsm is already in command mode.
    gsm_data_mode = 0;

    // Since connection is already closed, deactivate GPRS context.
    if (gsm_send_command("AT+QIDEACT\r", GSM_OK, 3*GSM_TIMEOUT))
        return -1;

    delay_ms(1000);

    // Reactivate the GPRS.
    if (gsm_send_command("AT+QIACT\r", GSM_OK, 10*GSM_TIMEOUT))
        return -1;

    delay_ms(1000);

    // Try to connect to the server.
    if (gsm_send_command("AT+QIOPEN=\"TCP\",\"76.12.155.219\",36740\r", GSM_OK, 5*GSM_TIMEOUT))
        return -1;

    gsm_data_mode = 1;
    return 0;

}*/

// The following is no longer in use, but kept for reference.
//Deprecated.
/* Send a command to the gsm module and wait for a response. */
/*
int gsm_send_command_old(char *command, GSMResponse response) {

    write_string(GSM_UART, command);

    int k = 0;
    while(gsm_get_state() != response) {
        if (k == GSM_TIMEOUT)
            return -1;
        k++;
    }
    return 0;

}
*/
//Deprecated
/* Establish tcp connection. Important comments in function body! */
/*
int gsm_tcp_connect(void) {

    // Set the context 0 as FGCNT.
    if (gsm_send_command("AT+QIFGCNT=0\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    delay_ms(100);

    // Set APN.
    if (gsm_send_command("AT+QICSGP=1,\"ATT.MVNO\"\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    delay_ms(100);

    // Disable function of MUXIP. (Set as single connection mode.) )
    if (gsm_send_command("AT+QIMUX=0\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    delay_ms(100);

    // Set the session mode as transparent.
    // NOTE: for some reason, this always fails whenever the status of the SIM card
    // is queried before calling this function. don't ask...
    if (gsm_send_command("AT+QIMODE=1\r", GSM_OK, 2*GSM_TIMEOUT))
        return -1;
    delay_ms(100);

    // Other internal settings.
    if (gsm_send_command("AT+QITCFG=3,2,512,1\r", GSM_OK, GSM_TIMEOUT))
        return -1;

    // NOTE: The default behavior is to expect a dotted IP address.
    // Use IP address (not domain name) as the address to establish TCP/UDP session.
    //if (gsm_send_command("AT+QIDNSIP=0\r", GSM_OK, GSM_TIMEOUT))
    //    return -1;
    //delay_ms(100);

    // Register the TCP/IP stack.
    if (gsm_send_command("AT+QIREGAPP\r", GSM_OK, 3*GSM_TIMEOUT))
        return -1;

    // The following pause is essential. We need to give the module some time
    // after registering the network and before asking for a gprs connection.
    delay_ms(2000);

    // Activate FGCNT. Brings up the wireless connection.
    if (gsm_send_command("AT+QIACT\r", GSM_OK, 18*GSM_TIMEOUT))
        return -1;
    delay_ms(100);

    // Establish TCP connection with server.
    //TODO: Don't hardcode these.
    //TODO: Wait for "connect" status.
    //if (gsm_send_command("AT+QIOPEN=\"TCP\",\"traclock.no-ip.biz\",36740\r", GSM_OK, 5*GSM_TIMEOUT))
    if (gsm_send_command("AT+QIOPEN=\"TCP\",\"76.12.155.219\",36740\r", GSM_OK, 5*GSM_TIMEOUT))
        return -1;

    // By default, establishing the tcp connection puts us in data mode.
    gsm_data_mode = 1;

    return 0;

}
*/
/* Update the state of the gsm module. */
/*void gsm_update_state_old(GsmState *s, char data) {

    // Add byte to buffer
    s->indx++;
    if (s->indx == GSM_BUFFER_LEN)
        s->indx = -1;
    gsm_response_buffer[gsm_buffer_indx] = data;

    // Check for changes in state.
    // For the most part, the module will return "OK" followed by line return
    // and new line. However, the http post function does not give the return/
    // new line. Therefore we explicitly only check for the two characters "OK".
    if ((gsm_response_buffer[gsm_buffer_indx]     ==  'K') &&
        (gsm_response_buffer[buffer_indx_prev(1)] ==  'O')) {
        response_rcvd = 1;
        gsm_state = GSM_OK;
    }

    else if ((gsm_response_buffer[gsm_buffer_indx] == '>')) {
        response_rcvd = 1;
        gsm_state = GSM_READY;
    }

    else if ((gsm_response_buffer[gsm_buffer_indx]     == 'T') &&
             (gsm_response_buffer[buffer_indx_prev(1)] == 'C') &&
             (gsm_response_buffer[buffer_indx_prev(2)] == 'E') &&
             (gsm_response_buffer[buffer_indx_prev(3)] == 'N') &&
             (gsm_response_buffer[buffer_indx_prev(4)] == 'N') &&
             (gsm_response_buffer[buffer_indx_prev(5)] == 'O') &&
             (gsm_response_buffer[buffer_indx_prev(6)] == 'C')) {
        response_rcvd = 1;
        gsm_state = GSM_CONNECT;
    }

    else if ((gsm_response_buffer[gsm_buffer_indx]     == 'R') &&
             (gsm_response_buffer[buffer_indx_prev(1)] == 'O') &&
             (gsm_response_buffer[buffer_indx_prev(2)] == 'R') &&
             (gsm_response_buffer[buffer_indx_prev(3)] == 'R') &&
             (gsm_response_buffer[buffer_indx_prev(4)] == 'E')) {
        response_rcvd = 1;
        gsm_state = GSM_ERROR;
    }

    else if ((gsm_response_buffer[gsm_buffer_indx]     == 'D') &&
             (gsm_response_buffer[buffer_indx_prev(1)] == 'E') &&
             (gsm_response_buffer[buffer_indx_prev(2)] == 'S') &&
             (gsm_response_buffer[buffer_indx_prev(3)] == 'O') &&
             (gsm_response_buffer[buffer_indx_prev(4)] == 'L') &&
             (gsm_response_buffer[buffer_indx_prev(5)] == 'C')) {
        response_rcvd = 1;
        gsm_state = GSM_CLOSED;
        if (gsm_data_mode)
            gsm_tcp_lost = 1;
    }

}*/

/* Returns the state of the gsm module. */
/*GSMResponse gsm_get_state(void) {
    if (response_rcvd) {
        response_rcvd = 0;
        return gsm_state;
    }
    else
        return GSM_NONE;
}
*/

/* Get index of nth previous char in buffer, accounting for wraparound. */
/*int prev_indx(GsmState *s, int n) {
    int i = s->indx - n;
    if (i >= 0)
        return i;
    else
        return GSM_BUFFER_LEN+i;
}*/