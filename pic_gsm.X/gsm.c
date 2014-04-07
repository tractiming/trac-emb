#include "picsetup.h"
#include "gsm.h"
#include "comm.h"
#include <plib.h>

//const char APN[] = "att.mvno";

int gsm_buffer_indx = 0;
char gsm_response_buffer[GSM_BUFFER_LEN];
GSMResponse gsm_state;
int response_rcvd = 0;

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
        gsm_buffer_indx = 0;
    gsm_response_buffer[gsm_buffer_indx] = data;

    // Check for change in state.
    if ((gsm_response_buffer[gsm_buffer_indx]   ==  10) &&
        (gsm_response_buffer[buffer_indx_prev(1)] ==  13) &&
        (gsm_response_buffer[buffer_indx_prev(2)] == 'K') &&
        (gsm_response_buffer[buffer_indx_prev(3)] == 'O')) {
        response_rcvd = 1;
        gsm_state = GSM_OK;
    }
    else if ((gsm_response_buffer[gsm_buffer_indx] == ' ') &&
             (gsm_response_buffer[buffer_indx_prev(1)] == '>')) {
        response_rcvd = 1;
        gsm_state = GSM_READY;
    }

    else if ((gsm_response_buffer[gsm_buffer_indx] == 'T') &&
             (gsm_response_buffer[gsm_buffer_indx] == 'C') &&
             (gsm_response_buffer[gsm_buffer_indx] == 'E') &&
             (gsm_response_buffer[gsm_buffer_indx] == 'N') &&
             (gsm_response_buffer[gsm_buffer_indx] == 'N') &&
             (gsm_response_buffer[gsm_buffer_indx] == 'O') &&
             (gsm_response_buffer[gsm_buffer_indx] == 'C')) {
        response_rcvd = 1;
        gsm_state = GSM_CONNECT;
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

/* Initialize GSM module. */
void gsm_init(void) {

    // Reset buffer and state.
    gsm_buffer_indx = 0;
    gsm_state = GSM_NONE;
    GSM_LED = 0;

    // Pull powerkey low to turn on module.
    POWERKEY = 0;
    delay_ms(2500);
    POWERKEY = 1;
    delay_ms(200);
  
    // Send "AT" command to sync baudrates.
    while (gsm_get_state() != GSM_OK) {
        write_string(UART2, "AT\r");
        delay_ms(100);
    }
    delay_ms(6000);

    // Set mode to text.
    //write_string(UART2, "AT+CMGF=1\r");
    gsm_send_command("AT+CMGF=1\r", GSM_OK);
    
    // Turn on LED to indicate GSM is initialized.
    //GSM_LED = 1;

}

/* Send a command to the gsm module and wait for a response. */
int gsm_send_command(char *command, GSMResponse response) {

    write_string(UART2, command);

    int k = 0;
    while(gsm_get_state() != response) {
        if (k == GSM_TIMEOUT)
            return -1;
        k++;
    }
    return 0;
    
}

int gsm_tcp_connect(void) {

    // Set the context 0 as FGCNT.
    if (gsm_send_command("AT+QIFGCNT=0\r", GSM_OK))
        return -1;

    // Set APN.
    if (gsm_send_command("AT+QICSGP=1,\"ATT.MVNO\"\r", GSM_OK))
        return -1;

    // Disable function of MUXIP. (Set as single connection mode.) )
    if (gsm_send_command("AT+QIMUX=0\r", GSM_OK))
        return -1;

    // Set the session mode as tranparent.
    if (gsm_send_command("AT+QIMODE=1\r", GSM_OK))
        return -1;

    // Other internal settings.
    if (gsm_send_command("AT+QITCFG=3,2,512,1\r", GSM_OK))
        return -1;

    // Use domain name as the address to establish TCP/UDP session.
    if (gsm_send_command("AT+QIDNSIP=1\r", GSM_OK))
        return -1;

    // Register the TCP/IP stack.
    if (gsm_send_command("AT+QIREGAPP\r", GSM_OK))
        return -1;

    // Activate FGCNT. Brings up the wireless connection.
    if (gsm_send_command("AT+QIACT\r", GSM_OK))
        return -1;

    // Establish TCP connection with server.
    if (gsm_send_command("AT+QIOPEN=\"TCP\",\"traclock.no-ip.biz\",36740\r", GSM_OK))
        return -1;

    return 0;


}

int gsm_tcp_send_data(char *data) {
    return 0;

}





