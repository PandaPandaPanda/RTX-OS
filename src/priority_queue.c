#include "priority_queue.h"
#include "common.h"

PriorityQueue priorityQueueInit(void) {
    PriorityQueue q;
    q.head = NULL;
    q.tail = NULL;
    q.numBlocks = 0;
    return q;
}

void priorityEnqueue(PriorityQueue *q, PCB *mb) {
    PCB* start = q->head;
    if (!q->head) {
        q->head = mb;
    }
    else if(start->m_priority > mb->m_priority){
        mb->mp_next = q->head;
        q->head = mb;
    }
    else{
        while(start ->mp_next != NULL && start->mp_next->m_priority <= mb->m_priority){
            start = start->mp_next;
        }

        mb->mp_next = start->mp_next;

        start->mp_next = mb;
    }

    if(mb->mp_next == NULL){
        q->tail = mb;
    }
    q->numBlocks++;
}

PCB* priorityDequeue(PriorityQueue *q) {
    if (q->numBlocks == 0) {
        return NULL;
    }
    PCB *tmp = q->head;
    if (q->numBlocks == 1) {
        q->tail = NULL;
    }
    q->head = q->head->mp_next;
    q->numBlocks--;
    tmp->mp_next = NULL;
    return tmp;
}

int isEmpty(PriorityQueue *q) {
    return q->head == NULL;
}

PCB* priorityPeek(PriorityQueue *q) {
    return q->head;
}

PCB* priorityRemove(PriorityQueue *q, int pid){
    PCB dummy;
    dummy.mp_next = q->head;

    PCB *prev = &dummy;
    PCB *cur = q->head; 

    while (cur && cur->m_pid != pid) {
        prev = cur;
        cur = cur->mp_next;
    }

    // if not found
    if (!cur || cur->m_pid != pid) {
        return NULL;
    }

    // remove the node from queue
    if (cur == q->head) {
        return priorityDequeue(q);
    } else if (cur == q->tail) {
        q->tail = prev;
    }
    prev->mp_next = cur->mp_next;
    q->numBlocks -= 1;
    return cur;
}

void printPriorityQueue(PriorityQueue* pq) {
    PCB* current_pcb = pq->head;
    while(current_pcb != NULL) {
        printf("pid: %d, priority: %d\n", current_pcb -> m_pid, current_pcb -> m_priority);
        current_pcb = current_pcb -> mp_next;
    }

}
