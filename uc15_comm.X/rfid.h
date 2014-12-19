#ifndef RFID_H
#define RFID_H

#define NUM_SPLITS 6
#define NEXT_SPLIT_INDX(i) ((i+1) % NUM_SPLITS)
#define BUF_LEN1 20
#define BUF_LEN2 200
#define NEXT_BUF1_INDX(i) ((i+1) % BUF_LEN1)
#define NEXT_BUF2_INDX(i) ((i+1) % BUF_LEN2)
#define MAX_MSG_LEN 250
#define BOOT_WAIT 100

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

void get_split_msg(Split *, char *);
void add_char_to_buffer(LineBuffer *, char);
void update_splits(SplitQueue *, LineBuffer *);
//void post_splits_to_server(GsmState *, SplitQueue *, const char*);
void rfid_init(void);

#endif	/* RFID_H */
