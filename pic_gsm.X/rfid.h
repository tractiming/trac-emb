#ifndef RFID_H
#define RFID_H

#define NUM_SPLITS 50
#define NEXT_SPLIT_INDX(i) ((i+1) % NUM_SPLITS)
#define BUF_LEN1 10
#define BUF_LEN2 200
#define NEXT_BUF1_INDX(i) ((i+1) % BUF_LEN1)
#define NEXT_BUF2_INDX(i) ((i+1) % BUF_LEN2)
#define BOOT_WAIT 130   // Number of seconds to wait for Alien to turn on.
#define MAX_MSG_SPLITS 10
#define MAX_MSG_LEN 750

typedef struct
{
    char tag_id[40];
    char time[30];
    char ant[2];   
} Split;

typedef struct 
{
    Split queue[NUM_SPLITS];
    int head;
    int tail;
} SplitQueue;

typedef struct
{
    char buf[BUF_LEN1][BUF_LEN2];
    int head;
    int tail;
    int indx;

} LineBuffer;

extern SplitQueue rfid_split_queue;
extern LineBuffer rfid_line_buffer;

void rfid_add_to_buffer(LineBuffer *, char);
void update_splits(SplitQueue *, LineBuffer *);
void rfid_init(void);
int get_next_split_msg(SplitQueue *, const char *, char *);
char *strtok_r (char *, const char *, char **);
int get_update_msg(SplitQueue *, const char *, char *);
void rfid_set_time(const char *);

#endif	/* RFID_H */
