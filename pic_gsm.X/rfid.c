#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rfid.h"
#include "gsm.h"
#include "picsetup.h"
#include "comm.h"

SplitQueue rfid_split_queue;
LineBuffer rfid_line_buffer;

char test_msg[200];

void clear_queue(SplitQueue *q)
{
    q->head = 0;
    q->tail = 0;
}

int queue_is_empty(SplitQueue *q)
{
    return ((q->head) == (q->tail));
}

void add_split_to_queue(SplitQueue *q, Split *s)
{
    memcpy(&(q->queue[q->head]), s, sizeof(Split));
    q->head = NEXT_SPLIT_INDX(q->head);
}

void get_split_msg(Split *s, char *str)
{
    sprintf(str, "id=%s&time=%s&ant=%s\0", s->tag_id, s->time, s->ant);
}

void pop_split_from_queue(SplitQueue *q, char *dest)
{
    if (queue_is_empty(q))
        return;

    get_split_msg(&(q->queue[q->tail]), dest);
    q->tail = NEXT_SPLIT_INDX(q->tail);
}

int parse_split_data(char *data, Split *s)
{
    char *tok1, *tok2;
    char *end_str, *end_tok;
    const char sep[] = ",";
    int tag=0, last=0, ant=0;

    tok1 = (char *) strtok_r(data, sep, &end_str);
    while (tok1 != NULL)
    {
        if (strstr(tok1, "Tag"))
        {
            tok2 = (char *) strtok_r(tok1, ":", &end_tok);
            tok2 = (char *)strtok_r(NULL, "\0", &end_tok);
            strcpy(s->tag_id, tok2);
            tag = 1;
        }
        else if (strstr(tok1, "Last"))
        {
            tok2 = (char *)strtok_r(tok1, ":", &end_tok);
            tok2 = (char *)strtok_r(NULL, "\0", &end_tok);
            strcpy(s->time, tok2);
            last = 1;
        }
        else if (strstr(tok1, "Ant"))
        {
            tok2 = (char *)strtok_r(tok1, ":", &end_tok);
            tok2 = (char *)strtok_r(NULL, "\0", &end_tok);
            strcpy(s->ant, tok2);
            ant = 1;
        }

        tok1 = (char *)strtok_r(NULL, sep, &end_str);
    }

    if (!(tag && last && ant))
        return -1;

    return 0;
}

void save_split(SplitQueue *q, char *m)
{
    // Only accept the split if all information has been found in the
    // notification message.
    if (!parse_split_data(m, &(q->queue[q->head])))
        q->head = NEXT_SPLIT_INDX(q->head);
}

void clear_buffer(LineBuffer *b)
{
    b->head = 0;
    b->tail = 0;
    b->indx = 0;
}

void add_char_to_buffer(LineBuffer *b, char c)
{

    if (c == '\n')
    {
        b->buf[b->head][b->indx] = '\0';
        b->head = NEXT_BUF1_INDX(b->head);
        b->indx = 0;
    }

    // Here we ignore any extra characters not in a normal notification message.
    else if ((c < 32) || (c > 127))
    {}
    
    else
    {
        b->buf[b->head][b->indx] = c;
        b->indx = NEXT_BUF2_INDX(b->indx);
    }
}

void update_splits(SplitQueue *q, LineBuffer *b)
{
   while (b->tail != b->head)
   {
       save_split(q, b->buf[b->tail]);
       b->buf[b->tail][0] = '\0';
       b->tail = NEXT_BUF1_INDX(b->tail);
   }

}

void post_splits_to_server(GsmState *s, SplitQueue *q, const char *r_id)
{
    char msg[MAX_MSG_LEN];
    while (!queue_is_empty(q))
    {
        pop_split_from_queue(q, msg);
        strcat(msg, "&r=");
        strcat(msg, r_id);
        gsm_http_post(s, msg);
    }
}

void rfid_init(void)
{
    delay_ms(BOOT_WAIT*1000);
    clear_queue(&rfid_split_queue);
    clear_buffer(&rfid_line_buffer);
}