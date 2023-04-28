#include "timeout_queue.h"
#include "queue.h"
#include "common.h"

TimeoutQueue timeoutQueueInit(void) {
    TimeoutQueue q;
    q.head = NULL;
    q.tail = NULL;
    q.numBlocks = 0;
    return q;
}

void timeoutEnqueue(TimeoutQueue *q, MSG_BUF *mb) {
    MSG_BUF* start = q->head;
    if (!q->head) { // no node to 1 node
        q->head = mb;
				start = q->head;
    } else if (start->m_clock_ticks > mb->m_clock_ticks){
        mb->mp_next = q->head;
        q->head = mb;
    } else{
        MSG_BUF *prev = NULL;
        while(start != NULL && start->m_clock_ticks <= mb->m_clock_ticks){
            mb->m_clock_ticks -= start->m_clock_ticks;
            prev = start;
            start = start->mp_next;
        }
        
        prev->mp_next = mb;
        mb->mp_next = start;

        // update relative tick difference after inserted MSG_BUF
        start = mb->mp_next;
        while(start != NULL){
            start->m_clock_ticks -= mb->m_clock_ticks;
            start = start->mp_next;
        }
    }

    if(mb->mp_next == NULL){
        q->tail = mb;
    }
    
    q->numBlocks++;
}

MSG_BUF* timeoutDequeueExpired(TimeoutQueue *q) {
    if (q->numBlocks == 0) {
        return NULL;
    }
    MSG_BUF *tmp = q->head;
    if (q->numBlocks == 1) {
        q->tail = NULL;
    }
    if (tmp-> m_clock_ticks <= 0) {
        q->head = q->head->mp_next;
        q->numBlocks--;
        tmp->mp_next = NULL;
        return tmp;
    } else {
        return NULL;
    }
}

void timeoutDecrement(TimeoutQueue *q) {
    if (q->numBlocks == 0) {
        return;
    }
    MSG_BUF *tmp = q->head;
    tmp->m_clock_ticks--;
}
