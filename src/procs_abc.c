#include "procs_abc.h"
#include "rtx.h"
#include "k_msg.h"
#include "k_memory.h"
#include "k_process.h"
#include "utils.h"

void proc_a(void) {
    MSG_BUF *p;
    p = (MSG_BUF*)request_memory_block();
    p->mtype = KCD_REG;
    p->mtext[0] = '%';
    p->mtext[1] = 'Z';
    p->mtext[2] = '\0';
    printf("process a: registering %%Z command\r\n");
    send_message(PID_KCD, p);

    int sender_pid;

    p = (MSG_BUF*)receive_message(&sender_pid);
    if (p != NULL) {
        release_memory_block(p);
    }
    printf("process a: received message %%Z. \r\n");
    int num = 0;
    while(1) {
        p = (MSG_BUF* )request_memory_block();
        p -> mtype = COUNT_REPORT;
        write_num_to_str(num, p->mtext);
        printf("PROC A: received memory block and sending %s\r\n", p->mtext);
        send_message(PID_B, p);
        num++;
        release_processor();
    }
}

void proc_b(void) {
    int sender_pid;
    while(1) {
        MSG_BUF* p = (MSG_BUF*)receive_message(&sender_pid);
#ifdef _DEBUG_0
        printf("PROC B: received and passing message to C: %s\r\n", p->mtext);
#endif
        send_message(PID_C, p);
    }
}

void proc_c(void) {
    MsgQueue local_msg_queue = msg_queue_Init();
    MSG_BUF* p;
    int sender_pid;
    while(1) {
        if (local_msg_queue.numBlocks == 0) {
            p = (MSG_BUF*)receive_message(&sender_pid);
#ifdef _DEBUG_0
                printf("PROC C: Received the message: %s\r\n", p->mtext);
#endif
        } else {
            p = dequeue_msg(&local_msg_queue);
        }
        if (p->mtype == COUNT_REPORT){
            if (str_to_int(p->mtext) % 20 == 0) {
                write_str_to_buffer("Process C\r\n", p->mtext);
                p->mtype = CRT_DISPLAY;
                send_message(PID_CRT, p);
                // hibernate
                MSG_BUF* q = request_memory_block();
                q ->mtype = WAKEUP10;
#ifdef _DEBUG_0
                printf("Sending wake up delayed message to proc_c\r\n");
#endif
                delayed_send(PID_C, q, 10000);
                while(1) {
                    p = (MSG_BUF*) receive_message(&sender_pid);
                    if (p->mtype == WAKEUP10) {
#ifdef _DEBUG_0
                        printf("wakeup 10\r\n");
#endif
                        break;
                    }else {
#ifdef _DEBUG_0
                        printf("received message in process c: %s\r\n", p->mtext);
#endif
                        enqueue_msg(&local_msg_queue, p);
                    }
                }
            }
        }
        if (p != NULL) {
            printf("PROC C: p->text: %s\r\n", p->mtext);
            if (release_memory_block(p) == RTX_ERR) {
                printf("PROC C: unable to release memory block\r\n");
            } else {
                printf("PROC C: released memory block\r\n");
            }
        }
        release_processor();
    }
}
