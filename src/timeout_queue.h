
#ifndef TIMEOUT_QUEUE_H_INCLUDED
#define TIMEOUT_QUEUE_H_INCLUDED

#include "common.h"
#include "k_inc.h"

typedef struct timeout_queue
{
    int numBlocks;
    MSG_BUF *head;
    MSG_BUF *tail;
}TimeoutQueue;

TimeoutQueue timeoutQueueInit(void);
void timeoutEnqueue(TimeoutQueue *, MSG_BUF *);
MSG_BUF * timeoutDequeueExpired(TimeoutQueue *);
void timeoutDecrement(TimeoutQueue *q);

#endif
