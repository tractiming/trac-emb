#include <plib.h>
#include "picsetup.h"
#include "gsm.h"
#include "comm.h"

//const char APN[] = "att.mvno";

int gsm_buffer_indx = 0;
char gsm_response_buffer[GSM_BUFFER_LEN];
GSMResponse gsm_state;
int response_rcvd = 0;
unsigned int gsm_data_mode=0;

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
    else if ((gsm_response_buffer[gsm_buffer_indx] == ' ') &&
             (gsm_response_buffer[buffer_indx_prev(1)] == '>')) {
        response_rcvd = 1;
        gsm_state = GSM_READY;
    }

    //else if ((gsm_response_buffer[gsm_buffer_indx] == 'T') &&
    //         (gsm_response_buffer[gsm_buffer_indx] == 'C') &&
    //         (gsm_response_buffer[gsm_buffer_indx] == 'E') &&
    //         (gsm_response_buffer[gsm_buffer_indx] == 'N') &&
    //         (gsm_response_buffer[gsm_buffer_indx] == 'N') &&
    //         (gsm_response_buffer[gsm_buffer_indx] == 'O') &&
    //         (gsm_response_buffer[gsm_buffer_indx] == 'C')) {
    //    response_rcvd = 1;
    //    gsm_state = GSM_CONNECT;
    //}

    //else if ((gsm_response_buffer[gsm_buffer_indx] == 'D') &&
    //         (gsm_response_buffer[buffer_indx_prev(1)] == 'E') &&
    //         (gsm_response_buffer[buffer_indx_prev(2)] == 'S') &&
    //         (gsm_response_buffer[buffer_indx_prev(3)] == 'O') &&
    //         (gsm_response_buffer[buffer_indx_prev(4)] == 'L') &&
    //         (gsm_response_buffer[buffer_indx_prev(5)] == 'C')) {
    //    response_rcvd = 1;
    //    gsm_state = GSM_CLOSED;
    //}

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

/* Turn off the gsm by toggling power pin.
   NOTE: The GSM2 click does not provide a pin for EMERG_OFF. */
void gsm_pwr_off(void) {
    // Pull power key to low. Safe to shut off after 12s.
    POWERKEY = 0;
    delay_ms(12000);
}

/* Checks if sim card is present and network is active. */
int gsm_chk_sim(void) {
    
    // Check if sim card is present.
    if (gsm_send_command("AT+CPIN?\r", GSM_OK, GSM_TIMEOUT))
        return -1;

    return 0;
}

/* Initialize GSM module. */
int gsm_init(void) {

    // Reset buffer and state.
    gsm_buffer_indx = 0;
    gsm_data_mode = 0;
    gsm_state = GSM_NONE;
    GSM_LED = 0;

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

    // Set mode to text.
    if (gsm_send_command("AT+CMGF=1\r", GSM_OK, 5*GSM_TIMEOUT))
        return -1;
    delay_ms(1000);
    //    return -1;
    
    // Turn on LED to indicate GSM is initialized.
    return 0;

}

/* Send a command to the gsm module and wait for a response. */
int gsm_send_command2(char *command, GSMResponse response) {

    write_string(GSM_UART, command);

    int k = 0;
    while(gsm_get_state() != response) {
        if (k == GSM_TIMEOUT)
            return -1;
        k++;
    }
    return 0;
    
}

/* Send command to GSM and wait for response or timeout. */
int gsm_send_command(char *command, GSMResponse response, unsigned timeout) {

    write_string(GSM_UART, command);

    WriteCoreTimer(0);
    while (1) {
        if (gsm_get_state() == response)
            return 0;

        if (ReadCoreTimer() > timeout*20000)
            return -1;
    }

}

/* Establish tcp connection. Important comments in function body! */
int gsm_tcp_connect(void) {
    
    // Set the context 0 as FGCNT.
    if (gsm_send_command("AT+QIFGCNT=0\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    
    // Set APN.
    if (gsm_send_command("AT+QICSGP=1,\"ATT.MVNO\"\r", GSM_OK, GSM_TIMEOUT))
        return -1;

    // Disable function of MUXIP. (Set as single connection mode.) )
    if (gsm_send_command("AT+QIMUX=0\r", GSM_OK, GSM_TIMEOUT))
        return -1;

    // Set the session mode as tranparent.
    if (gsm_send_command("AT+QIMODE=1\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    
    // Other internal settings.
    // This line was not working properly. I am removing.
    //if (gsm_send_command("AT+QITCFG=3,2,512,1\r", GSM_OK, GSM_TIMEOUT))
    //    return -1;
    
    // Use domain name as the address to establish TCP/UDP session.
    if (gsm_send_command("AT+QIDNSIP=1\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    
    // Register the TCP/IP stack.
    if (gsm_send_command("AT+QIREGAPP\r", GSM_OK, 3*GSM_TIMEOUT))
        return -1;
    
    // The following pause is essential. We need to give the module some time
    // after registering the network and asking for a gprs connection.
    delay_ms(2000);
    
    
    // Activate FGCNT. Brings up the wireless connection.
    if (gsm_send_command("AT+QIACT\r", GSM_OK, 18*GSM_TIMEOUT))
        return -1;
    RFID_LED = 1;

    // Establish TCP connection with server.
    //TODO: Don't hardcode these.
    //if (gsm_send_command("AT+QIOPEN=\"TCP\",\"traclock.no-ip.biz\",36740\r", GSM_OK, 5*GSM_TIMEOUT))
    if (gsm_send_command("AT+QIOPEN=\"TCP\",\"76.12.155.219\",36740\r", GSM_OK, 5*GSM_TIMEOUT))
        return -1;

    // By default, establishing the tcp connection puts us in data mode.
    gsm_data_mode = 1; 

    return 0;

}

// Closes the tcp connection and deactivates gprs.
int gsm_tcp_close(void) {
    // Switch back to AT command mode.
    if (gsm_send_command("+++", GSM_OK, GSM_TIMEOUT))
        return -1;
    gsm_data_mode = 0;

    // Close the connection.
    if (gsm_send_command("AT+QICLOSE\r", GSM_OK, GSM_TIMEOUT))
        return -1;

    // Deactivate gprs.
    if (gsm_send_command("AT+QIDEACT\r", GSM_OK, GSM_TIMEOUT))
        return -1;
    return 0;

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


