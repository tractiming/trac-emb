#include <plib.h>
#include <stdlib.h>
#include "comm.h"
#include "gsm.h"
#include "rfid.h"

#define MAX_NO_TAGS 2
#define MAX_TAG_LEN 30
#define MAX_MSG_LEN 50

#define NXT_INDX(i,mx) ((i+1) % mx)

char msg_bfr[MAX_MSG_LEN];
int msg_head;
int msg_tail;

int tag_read=0;

/* Get index of nth previous char in buffer, accounting for wraparound. */
// TODO: check that we are not reading random bytes.
int rfid_indx_prev(int n) {
    int i =  msg_head - n;
    if (i >= 0)
        return i;
    else
        return MAX_MSG_LEN+i;
}

/* Checks to see if the "Alien>" prompt is displayed. */
int rfid_is_ready(void) {
    if ((msg_bfr[msg_head] == '>') &&
            (msg_bfr[rfid_indx_prev(1)] == 'n') &&
            (msg_bfr[rfid_indx_prev(2)] == 'e') &&
            (msg_bfr[rfid_indx_prev(3)] == 'i') &&
            (msg_bfr[rfid_indx_prev(4)] == 'l') &&
            (msg_bfr[rfid_indx_prev(5)] == 'A'))
        return 1;

    else
        return 0;

}

/* Initialize the RFID reader. */
// TODO: Make sure alien startup has completed.
// NOTE: This assumes uart has already been opened.
int rfid_init(void) {

    // Clear the buffer.
    delay_ms(2000);
    msg_bfr_clr();

    // TODO: Test this!
    // Wait for the reader to respond when enter is sent.
    int num_att = 100;
    int k=0;
    while (1) {

        write_string(RFID_UART, "\r");
        delay_ms(1500);

        if (rfid_is_ready()) {
            msg_bfr_clr();
            return 0;
        }

        if (k > num_att)
            return -1;

        k++;

    }
}

/* Sets the reader in auto-notification mode to send a message when a tag is 
 * read.*/
// TODO: Make sure these commands are read.
int rfid_reader_config(void) {

    // Configure auto mode.
    write_string(RFID_UART, "PersistTime=10\r\n");
    delay_ms(600);
    write_string(RFID_UART, "AutoModeReset\r");
    delay_ms(600);
    write_string(RFID_UART, "AutoAction = Acquire\r");
    delay_ms(600);
    write_string(RFID_UART, "AutoStartTrigger = 0,0\r");
    delay_ms(600);
    write_string(RFID_UART, "AutoStopTimer = 0\r");
    delay_ms(600);
    write_string(RFID_UART, "NotifyAddress = serial\r");
    delay_ms(600);
    write_string(RFID_UART, "NotifyTrigger = Add\r");
    delay_ms(600);
    write_string(RFID_UART, "NotifyMode = On\r");
    delay_ms(600);
    write_string(RFID_UART, "AutoMode = On\r");
    delay_ms(600);

    return 0;
}

/*void rfid_get_response(char data) {

    // Add byte to buffer
    rfid_msg_indx++;
    if (rfid_buffer_indx == MAX_TAG_LEN)
        rfid_buffer_indx = -1;
    rfid_response_buffer[rfid_buffer_indx] = data;

    if (rfid_response_buffer[rfid_buffer_indx]);


}*/

/* Clear the message buffer. */
void msg_bfr_clr(void) {

    msg_head = 0;
    msg_tail = 0;

}


/* Returns 1 if buffer is empty, 0 otherwise. */
int rfid_msg_empty(void) {

    if (msg_head == msg_tail)
        return 1;
    else
        return 0;

}

/* Writes a byte to the head of the buffer. */
void rfid_write_bfr(char c) {

    msg_head = NXT_INDX(msg_head, MAX_MSG_LEN);
    msg_bfr[msg_head] = c;

}

/* Prints tail of buffer to GSM. */
void rfid_read_bfr(void) {

    put_character(GSM_UART, msg_bfr[msg_tail]);
    msg_tail = NXT_INDX(msg_tail, MAX_MSG_LEN);

}
