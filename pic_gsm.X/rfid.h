/* 
 * File:   rfid.h
 * Author: elliot
 *
 * Created on April 10, 2014, 10:19 PM
 */

#ifndef RFID_H
#define	RFID_H

#define MAX_TAG_MSG_LEN 50
#define RFID_TIMEOUT 10000

typedef struct {
    char tag_str[MAX_TAG_MSG_LEN];
    int str_indx;
} TagData;

int rfid_init(void);
void rfid_clear_tags(void);
int rfid_tag_ready(void);
void rfid_write_bfr(char c);
void rfid_read_bfr(void);


#endif	/* RFID_H */

