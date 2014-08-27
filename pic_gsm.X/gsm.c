#include <plib.h>
#include <string.h>
#include <stdlib.h>
#include "picsetup.h"
#include "gsm.h"
#include "comm.h"

int gsm_buffer_indx = 0;
char gsm_response_buffer[GSM_BUFFER_LEN];
GSMResponse gsm_state;
int response_rcvd = 0;
unsigned int gsm_data_mode=0;
unsigned int gsm_tcp_lost=0;

const char IP[] = "http://traclock.no-ip.biz";
const char APN[] = "ATT.MVNO";
int PORT = 8000;
const char post_domain_name[] = "http://traclock.no-ip.biz:8000/update/";

/* Get index of nth previous char in buffer, accounting for wraparound. */
int buffer_indx_prev(int n) {
    int i = gsm_buffer_indx - n;
    if (i >= 0)
        return i;
    else
        return GSM_BUFFER_LEN+i;
}

/* Update the state of the gsm module. */
void gsm_update_state(char data) {

    // Add byte to buffer
    gsm_buffer_indx++;
    if (gsm_buffer_indx == GSM_BUFFER_LEN)
        gsm_buffer_indx = -1;
    gsm_response_buffer[gsm_buffer_indx] = data;

    // Check for change in state.
    if ((gsm_response_buffer[gsm_buffer_indx]   ==  10) &&
        (gsm_response_buffer[buffer_indx_prev(1)] ==  13) &&
        (gsm_response_buffer[buffer_indx_prev(2)] == 'K') &&
        (gsm_response_buffer[buffer_indx_prev(3)] == 'O')) {
        response_rcvd = 1;
        gsm_state = GSM_OK;
    }
    else if ((gsm_response_buffer[gsm_buffer_indx] == '>')) {
        response_rcvd = 1;
        gsm_state = GSM_READY;
    }

    else if ((gsm_response_buffer[gsm_buffer_indx] == 'T') &&
             (gsm_response_buffer[buffer_indx_prev(1)] == 'C') &&
             (gsm_response_buffer[buffer_indx_prev(2)] == 'E') &&
             (gsm_response_buffer[buffer_indx_prev(3)] == 'N') &&
             (gsm_response_buffer[buffer_indx_prev(4)] == 'N') &&
             (gsm_response_buffer[buffer_indx_prev(5)] == 'O') &&
             (gsm_response_buffer[buffer_indx_prev(6)] == 'C')) {
        response_rcvd = 1;
        gsm_state = GSM_CONNECT;
    }

    else if ((gsm_response_buffer[gsm_buffer_indx] == 'D') &&
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

    else if ((gsm_response_buffer[gsm_buffer_indx] == 'N') &&
             (gsm_response_buffer[buffer_indx_prev(1)] == 'W') &&
             (gsm_response_buffer[buffer_indx_prev(2)] == 'O') &&
             (gsm_response_buffer[buffer_indx_prev(3)] == 'D')) {
        response_rcvd = 1;
        gsm_state = GSM_PWR_DOWN;
    }

}

/* Returns the state of the gsm module. */
GSMResponse gsm_get_state(void) {
    if (response_rcvd) {
        response_rcvd = 0;
        return gsm_state;
    }
    else
        return GSM_NONE;
}

/* Delay for a given number of msecs. */
void delay_ms(unsigned int msec) {
    unsigned int delay = (SYS_FREQ/2000)*msec;
    WriteCoreTimer(0);
    while (ReadCoreTimer()<delay);
}

/* Turn on the gsm by toggling power pin. */
void gsm_pwr_on(void) {
    // To turn on, we pull power pin low, wait 2.5 sec and then pull high.
    POWERKEY = 0;
    delay_ms(2500);
    POWERKEY = 1;
    delay_ms(200);
}

//FIXME!
/* Turn off the gsm by toggling power pin.
   NOTE: The GSM2 click does not provide a pin for EMERG_OFF. */
void gsm_pwr_off(void) {
    // Pull power key to low. Safe to shut off after 12s.
    gsm_send_command("AT+QPOWD=1\r", GSM_PWR_DOWN, GSM_TIMEOUT);
    //POWERKEY = 0;
    //delay_ms(750);
    //POWERKEY = 1;
    //delay_ms(12000);
}

/* Checks if sim card is present. */
int gsm_chk_sim(void) {
    
    if (gsm_send_command("AT+CPIN?\r", GSM_OK, GSM_TIMEOUT))
        return -1;

    return 0;
}

/* Initialize GSM module. */
int gsm_init(void) {

    // Reset buffer and state.
    gsm_buffer_indx = 0;
    gsm_data_mode = 0;
    gsm_tcp_lost = 0;
    gsm_state = GSM_NONE;

    // Pull powerkey low to turn on module.
    gsm_pwr_on();
  
    // Send "AT" command to sync baudrates.
    WriteCoreTimer(0);
    while (gsm_get_state() != GSM_OK) {
        write_string(GSM_UART, "AT\r");
        delay_ms(100);
        
        // If no response, return failure.
        if (ReadCoreTimer() > 40000*GSM_TIMEOUT)
            return -1;
    }
    delay_ms(5000);

    return 0;

}

/* Wait for the GSM to give response. Timeout if not received. */
int gsm_wait_for_response(GSMResponse resp, unsigned timeout) {

    WriteCoreTimer(0);
    while (1) {
        if (gsm_get_state() == resp)
            return 0;

        if (ReadCoreTimer() > timeout*20000)
            return -1;
    }

}

/* Send command to GSM and wait for response or timeout. */
int gsm_send_command(char *command, GSMResponse response, unsigned timeout) {

    write_string(GSM_UART, command);
    return gsm_wait_for_response(response, timeout);

}

/* Brings up connection with the GPRS network. */
int gsm_gprs_init(void) {

    int delay_time = 100;

    // Set the context 0 as FGCNT.
    if (gsm_send_command("AT+QIFGCNT=0\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    delay_ms(delay_time);

    // Set APN.
    if (gsm_send_command("AT+QICSGP=1,\"ATT.MVNO\"\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    delay_ms(delay_time);

    // Register the TCP/IP stack.
    if (gsm_send_command("AT+QIREGAPP\r", GSM_OK, 3*GSM_TIMEOUT))
        return -1;

    // The following pause is essential. We need to give the module some time
    // after registering the network and before asking for a gprs connection.
    delay_ms(2000);

    // Activate FGCNT. Brings up the wireless connection.
    if (gsm_send_command("AT+QIACT\r", GSM_OK, 18*GSM_TIMEOUT))
        return -1;
    delay_ms(delay_time);

    return 0;

}

/* Sets the HTTP url to send data to. */
int gsm_set_http_url(void) {

    int len = strlen(post_domain_name);
    char msg[100];
    sprintf(msg, "AT+QHTTPURL=%i,15\r", len);

    if (gsm_send_command(msg, GSM_CONNECT, 5*GSM_TIMEOUT))
        return -1;
    delay_ms(100);

    write_string(GSM_UART, post_domain_name);
    return gsm_wait_for_response(GSM_OK, GSM_TIMEOUT);

}

/* Sends an HTTP POST request to the server. This assumes the http url is set.*/
int gsm_http_post(char* data) {

    int len = strlen(data);
    if (len > GSM_MAX_HTTP_LEN)
        return -1;
    
    char msg[100];
    sprintf(msg, "AT+QHTTPPOST=%i,5,1\r", len);

    if (gsm_send_command(msg, GSM_CONNECT, GSM_TIMEOUT))
        return -1;
    delay_ms(25); // This pause is really important!

    write_string(GSM_UART, data);
    return gsm_wait_for_response(GSM_OK, GSM_TIMEOUT/100);

}

/* Establish TCP connection with server. This assumes that the GPRS is already
 * activated and the server is listening on the correct port.
 */
int gsm_tcp_connect(const char* ip, int port) {

    char itoa_bfr[8];
    itoa(itoa_bfr, port, 10);
    char msg[100];

    strcpy(msg, "AT+QIOPEN=\"TCP\",\"");
    strcat(msg, ip);
    strcat(msg, "\",");
    strcat(msg, itoa_bfr);
    strcat(msg, "\r");

    if (gsm_send_command(msg, GSM_OK, 5*GSM_TIMEOUT))
        return -1;
    return 0;
}

// Closes the tcp connection and deactivates gprs.
int gsm_tcp_close(void) {

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

    // Deactivate gprs.
    //if (gsm_send_command("AT+QIDEACT\r", GSM_OK, GSM_TIMEOUT))
    //    return -1;
    //return 0;

}

/* Writes a message over tcp. */
int gsm_tcp_send_data(char *data) {

    // If we are not in data mode, we cannot write the message directly.
    if (gsm_data_mode != 1)
        return -1;

    // Write the message and terminating character.
    write_string(GSM_UART, data);
    write_string(GSM_UART, "\032");
    
    return 0;
    
}

/* Returns whether the gsm is in data or text mode. */
unsigned gsm_get_data_mode(void) {
    return gsm_data_mode;
}

/* Returns 1 if the connection has been lost. */
unsigned gsm_chk_tcp_conn(void) {
    return gsm_tcp_lost;
}

/* Resets the tcp connection after a failure. */
int gsm_tcp_reset(void) {

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

}

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
