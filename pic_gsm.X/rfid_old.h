/* 
 * File:   rfid.h
 * Author: elliot
 *
 * Created on April 10, 2014, 10:19 PM
 */

#ifndef RFID_H
#define	RFID_H

#define MAX_NO_TAGS 20
#define MAX_TAG_LEN 150
#define MAX_MSG_LEN 50
#define BOOT_WAIT 5
#define RFID_TIMEOUT 10000

typedef struct {
    char tag_str[MAX_TAG_LEN];
    char date_str[MAX_TAG_LEN];
    char ant_str[1];
} TagData;

int rfid_init(void);
void rfid_clear_tags(void);
int rfid_tag_ready(void);
void rfid_write_bfr(char c);
void rfid_read_bfr(void);


#endif	/* RFID_H */

