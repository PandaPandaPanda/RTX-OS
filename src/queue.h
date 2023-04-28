
#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "common.h"

typedef struct mem_blk {
    struct mem_blk *next;
    /* other kernel pointers */
    int m_pid;
} mem_blk;

// typedef struct msg_blk {
//     struct msg_blk *next;
//     /* other kernel pointers */
//     MSG_BUF* blk;
// } msg_blk;

typedef struct Queue
{
    int numBlocks;
    struct mem_blk *head;
    struct mem_blk *tail;
}Queue;

typedef struct MsgQueue
{
    int numBlocks;
    MSG_BUF *head;
    MSG_BUF *tail;
}MsgQueue;

Queue queueInit(void);
void enqueue(Queue *, mem_blk *);
mem_blk* dequeue(Queue *);

MsgQueue msg_queue_Init(void);
void enqueue_msg(MsgQueue *, MSG_BUF *);
MSG_BUF* dequeue_msg(MsgQueue *);


/*
TODO: change style
*/

#endif /* QUEUE_H_INCLUDED */
