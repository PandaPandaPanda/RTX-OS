#include "queue.h"
#include "common.h"

// GENERAL QUEUE----------------------------------
Queue queueInit(void) {
    Queue q;
    q.head = NULL;
    q.tail = NULL;
    q.numBlocks = 0;
    return q;
}

void enqueue(Queue *q, mem_blk *blk) {
    if (!q->head) {
        q->head = blk;
    }
    else {
        q->tail->next = blk;
    }
    q->tail = blk;
    q->numBlocks++;
}

mem_blk* dequeue(Queue *q) {
    if (q->numBlocks == 0) {
        return NULL;
    }
    mem_blk *tmp = q->head;
    if (q->numBlocks == 1) {
        q->tail = NULL;
    }
    q->head = q->head->next;
    q->numBlocks--;
    tmp->next = NULL;
    return tmp;
}


// MESSAGE QUEUE----------------------------------
MsgQueue msg_queue_Init(void) {
    MsgQueue q;
    q.head = NULL;
    q.tail = NULL;
    q.numBlocks = 0;
    return q;
}

void enqueue_msg(MsgQueue *q, MSG_BUF *mb) {
    if (!q->head) {
        q->head = mb;
    }
    else {
        q->tail->mp_next = mb;
    }
    q->tail = mb;
    q->numBlocks++;
}

MSG_BUF* dequeue_msg(MsgQueue *q) {
    if (q->numBlocks == 0) {
        return NULL;
    }
    MSG_BUF *tmp = q->head;
    if (q->numBlocks == 1) {
        q->tail = NULL;
    }
    q->head = q->head->mp_next;
    q->numBlocks--;
    tmp->mp_next = NULL;
    return tmp;
}
