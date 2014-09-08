#include <plib.h>
#include <stdlib.h>
#include "comm.h"
#include "gsm.h"
#include "rfid.h"
#include "picsetup.h"



#define NXT_INDX(i,mx) ((i+1) % mx)

char msg_bfr[MAX_MSG_LEN];
int msg_head;
int msg_tail;
int tag_read=0;

char tag_data[MAX_NO_TAGS][MAX_TAG_LEN];
int tag_head;
int tag_tail;
int s_indx;

/* Get index of nth previous char in buffer, accounting for wraparound. */
int rfid_indx_prev(int n) {
    int i =  msg_head - n;
    if (i >= 0)
        return i;
    else
        return MAX_MSG_LEN+i;
}

/* Initialize the RFID reader. */
int rfid_init(void) {

    // Note: the auto-notification settings are saved to flash memory on the
    // reader, so it will automatically send messages as soon as boot is
    // completed. Here we simply wait for the boot to finish.
    delay_ms(BOOT_WAIT*1000);
    rfid_clear_tags();
    return 0;
   
}

/* Clear the message buffer. */
void rfid_clear_tags(void) {

    tag_head = 0;
    tag_tail = 0;
    s_indx = 0;

}

/* Returns 1 if buffer is not empty, 0 otherwise. */
int rfid_tag_ready(void) {

    if (tag_head == tag_tail)
        return 0;
    else
        return 1;

}

/* Writes a byte to the head of the buffer. */
void rfid_write_bfr(char c) {

    tag_data[tag_head][s_indx] = c;

    // Check if the message is complete.
    if (c == '1') {
        tag_data[tag_head][s_indx] = '\0';
        tag_head = NXT_INDX(tag_head, MAX_NO_TAGS);
        s_indx = 0;
    }

    else {
        s_indx = NXT_INDX(s_indx, MAX_TAG_LEN);
    }

}

/* Reads a single message out of the buffer, posting it to the server. */
void rfid_read_bfr(void) {
    if (rfid_tag_ready()) {

        RFID_LED = 0;
        delay_ms(500);
        RFID_LED = 1;

        // Only move our place in the list if the POST succeeded.
        if (gsm_http_post(tag_data[tag_tail])) {
            //TODO: fix this to handle failed post command.
            GSM_LED = 0;
        }
        else {
            tag_tail = NXT_INDX(tag_tail, MAX_NO_TAGS);
    
        }
    }
}


/* Clear the message buffer. */
void msg_bfr_clr_old(void) {

    msg_head = 0;
    msg_tail = 0;

}

/* Returns 1 if buffer is empty, 0 otherwise. */
int rfid_msg_empty_old(void) {

    if (msg_head == msg_tail)
        return 1;
    else
        return 0;

}

/* Writes a byte to the head of the buffer. */
void rfid_write_bfr_old(char c) {

    msg_head = NXT_INDX(msg_head, MAX_MSG_LEN);
    msg_bfr[msg_head] = c;

}

/* Prints tail of buffer to GSM. */
void rfid_read_bfr_old(void) {

    put_character(GSM_UART, msg_bfr[msg_tail]);
    msg_tail = NXT_INDX(msg_tail, MAX_MSG_LEN);

}


/* Checks to see if the "Alien>" prompt is displayed. */
/*void rfid_ready_check(void) {
    if ((msg_bfr[msg_head] == '>') &&
            (msg_bfr[rfid_indx_prev(1)] == 'n') &&
            (msg_bfr[rfid_indx_prev(2)] == 'e') &&
            (msg_bfr[rfid_indx_prev(3)] == 'i') &&
            (msg_bfr[rfid_indx_prev(4)] == 'l') &&
            (msg_bfr[rfid_indx_prev(5)] == 'A'))
        alien_rdy = 1;
}
*/
/* Sets the reader in auto-notification mode to send a message when a tag is
 * read.*/
// TODO: Make sure these commands are read.
// OBSOLETE!
/*int rfid_reader_config(void) {

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
}*/

/*
//rfid_reader_config();
    //delay_ms(2000);

    // TODO: Test this!
    // Wait for the reader to respond when enter is sent.
    int num_att = 100;
    int k=0;
    while (1) {

        write_string(RFID_UART, "\r\n");
        delay_ms(1500);

        if (alien_rdy) {
            msg_bfr_clr();
            check_response = 0;
            return 0;
        }

        if (k > num_att)
            return -1;

        k++;

    }
*/