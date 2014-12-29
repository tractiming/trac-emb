#include <stdlib.h>
#include <string.h>
#include "picsetup.h"
#include "rfid.h"
#include "comm.h"

SplitQueue rfid_split_queue;
LineBuffer rfid_line_buffer;

static void clear_queue(SplitQueue *q)
{
    q->head = 0;
    q->tail = 0;
}

static int queue_is_empty(SplitQueue *q)
{
    return ((q->head) == (q->tail));
}

static void get_split_msg(Split *s, char *str)
{
    sprintf(str, "id=%s&time=%s&ant=%s", s->tag_id, s->time, s->ant);
}

static void pop_split_from_queue(SplitQueue *q, char *dest)
{
    if (queue_is_empty(q))
        return;

    get_split_msg(&(q->queue[q->tail]), dest);
    q->tail = NEXT_SPLIT_INDX(q->tail);
}

static int parse_split_data(char *data, Split *s)
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

static void save_split(SplitQueue *q, char *m)
{
    // Only accept the split if all information has been found in the
    // notification message.
    if (!parse_split_data(m, &(q->queue[q->head])))
        q->head = NEXT_SPLIT_INDX(q->head);
}

static void clear_buffer(LineBuffer *b)
{
    b->head = 0;
    b->tail = 0;
    b->indx = 0;
}

void rfid_add_to_buffer(LineBuffer *b, char c)
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

int get_next_split_msg(SplitQueue *q, const char *r_id, char *msg)
{
    if (queue_is_empty(q))
        return 0;

    pop_split_from_queue(q, msg);
    strcat(msg, "&r=");
    strcat(msg, r_id);
    return 1;

}

void rfid_init(void)
{
    delay_ms(BOOT_WAIT*1000);
    clear_queue(&rfid_split_queue);
    clear_buffer(&rfid_line_buffer);
}