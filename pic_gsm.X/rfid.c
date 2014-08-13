#include <plib.h>
#include <stdlib.h>
#include "comm.h"
#include "gsm.h"
#include "rfid.h"

#define MAX_NO_TAGS 2
#define MAX_TAG_LEN 30
#define MAX_MSG_LEN 50
#define BOOT_WAIT 90

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

/* Initialize the RFID reader. */
int rfid_init(void) {

    // Note: the auto-notification settings are saved to flash memory on the
    // reader, so it will automatically send messages as soon as boot is
    // completed. Here we simply wait for the boot to finish.
    msg_bfr_clr();
    delay_ms(BOOT_WAIT*1000);
    return 0;
    
}

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