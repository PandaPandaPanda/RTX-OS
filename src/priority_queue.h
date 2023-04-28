#ifndef PRIORITY_QUEUE_H_INCLUDED
#define PRIORITY_QUEUE_H_INCLUDED

#include "common.h"
#include "k_inc.h"

typedef struct priority_queue
{
    int numBlocks;
    PCB *head;
    PCB *tail;
}PriorityQueue;

PriorityQueue priorityQueueInit(void);
void priorityEnqueue(PriorityQueue *, PCB *);
PCB* priorityDequeue(PriorityQueue *);
int isEmpty(PriorityQueue *);
PCB* priorityPeek(PriorityQueue *);
PCB* priorityRemove(PriorityQueue *, int);
void printPriorityQueue(PriorityQueue* pq);


/*
TODO: change style
*/

#endif /* QUEUE_H_INCLUDED */
