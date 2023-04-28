/**
 * @brief:  P3 public testing code
 * @author: Yiqing Huang
 * @date:   2022/03/01
 * @note:   Each process is in an infinite loop. Processes never terminate.
 */

/* 
-----------------------------------------
expected output at COM1 (polled terminal)
-----------------------------------------
proc2: receiving messages ...
proc6: registering %A command
proc1: requesting a mem_blk...
proc1: send messages to proc2...
proc2: got a message - A
abcde
ABCDE
proc1: send a message to proc2...
proc2: got a message - 0
fghij
FGHIJ
proc1: send a message to proc2...
proc2: got a message - 1
klmno
KLMNO
proc1: send a message to proc2...
proc2: got a message - 2
pqrst
PQRST
proc1: send a message to proc2...
proc2: got a message - 3
uvwxy
UVWXY
proc1: send a message to proc2...
proc2: got a message - 4
proc3: entering..., starting delay_send
proc3: done with delay_send
proc4: entering...
proc4: received a
proc6: received a
proc4: received b
proc6: received b
proc4: received c
proc6: received c
proc4: received d
proc6: received d
proc4: received e
proc6: received e
proc6: type %A in RTX console now.
proc6: asking CRT to display :-):-D.
proc6: type %A in RTX console now.
....
------------------------------------------------------
expected output at COM0 (interrupt-driven terminal)
------------------------------------------------------
%A
:-):-D
-------------------------------------------------------
It allows one to type %A<ENTER> multiple times.
*/


#include "rtx.h"
#include "ae_proc.h"
#include "ae_proc_ext.h"
#include "ae_util.h"
#include "uart_polling.h"
#include "printf.h"

/*
 *===========================================================================
 *                             MACROS
 *===========================================================================
 */

#ifdef AE_ENABLE
    
#define NUM_TESTS       2       // number of tests

#ifdef AE_ECE350
#define NUM_INIT_TASKS  2       // number of tasks during initialization
#endif // AE_ECE350

#endif // AE_ENABLE
/*
 *===========================================================================
 *                             GLOBAL VARIABLES 
 *===========================================================================
 */
 
#ifdef AE_ENABLE

#ifdef AE_ECE350
TASK_INIT    g_init_tasks[NUM_INIT_TASKS];
#endif


const char   PREFIX[]      = "G99-TS301";
const char   PREFIX_LOG[]  = "G99-TS301-LOG ";
const char   PREFIX_LOG2[] = "G99-TS301-LOG2";

AE_XTEST     g_ae_xtest;                // test data, re-use for each test
AE_CASE      g_ae_cases[NUM_TESTS];
AE_CASE_TSK  g_tsk_cases[NUM_TESTS];

#endif // AE_ENABLE

int g_iterations;
/* initialization table item */
void set_test_procs(PROC_INIT *procs, int num)
{
    int i;
    for( i = 0; i < num; i++ ) {
        procs[i].m_pid        = (U32)(i+1);
        procs[i].m_stack_size = 0x200;
    }
  
    procs[0].mpf_start_pc = &proc1;
    procs[0].m_priority   = MEDIUM;
    
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = HIGH;
    
    procs[2].mpf_start_pc = &proc3;
    procs[2].m_priority   = LOW;
    
    procs[3].mpf_start_pc = &proc4;
    procs[3].m_priority   = LOW;
    
    procs[4].mpf_start_pc = &proc5;
    procs[4].m_priority   = LOWEST;
    
    procs[5].mpf_start_pc = &proc6;
    procs[5].m_priority   = HIGH;
    
    g_iterations = 5;
}


/**
 * @brief: a process that prints five uppercase letters on each line
 *         and sends a message to P2
 */
void proc1(void)
{
    int     i = 0;
    int     j = 0;
    void    *p_blk;
    MSG_BUF *p_msg;
    char    *ptr;
    
    uart1_put_string("proc1: requesting a mem_blk...\r\n");
    p_blk = request_memory_block();
    p_msg = p_blk;
    p_msg->mtype = DEFAULT;
    ptr = p_msg->mtext;
    *ptr++ = 'A';
    *ptr++ = '\r';
    *ptr++ = '\n';
    *ptr++ = '\0';
    
    uart1_put_string("proc1: send messages to proc2...\r\n");
    send_message(PID_P2, p_blk);
    
    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\r\n");
            //ret_val = release_processor();
            p_blk = request_memory_block();
            p_msg = p_blk;
            p_msg->mtype = DEFAULT;
            ptr = p_msg->mtext;
            *ptr++ = ('0' + (j++)%10);
            *ptr++ = '\r';
            *ptr++ = '\n';
            *ptr++ = '\0';
            uart1_put_string("proc1: send a message to proc2...\r\n");
            send_message(PID_P2, p_blk);
        }
        if ( j == g_iterations ) {
            break;
        }
        uart1_put_char('A' + i%26);
        i++;
    }
    
    set_process_priority(PID_P1, LOWEST);
    while (1) {
        release_processor();
    }
}

/**
 * @brief: a process that prints five lower case letters
 *         and receives messages.
 */
void proc2(void)
{
    int     i = 0;
    int     j = 0;
    MSG_BUF *p_msg;
    void    *p_blk;
    
    uart1_put_string("proc2: receiving messages ...\r\n");
    p_blk = receive_message(NULL);
    p_msg = p_blk;

    uart1_put_string("proc2: got a message - ");
    uart1_put_string(p_msg->mtext);
    release_memory_block(p_blk);
    
    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\r\n");
            p_blk = receive_message(NULL);
            p_msg = p_blk;
            uart1_put_string("proc2: got a message - ");
            uart1_put_string(p_msg->mtext);
            release_memory_block(p_blk);
            j++;
        }
        if ( j == g_iterations ) {
            break;
        }
        uart1_put_char('a' + i%26);
        i++;
    }
    set_process_priority(PID_P2, LOW);
    while(1) {
        release_processor();
    }
}

/**
 * @brief a process that does delayed_send 5 messages in decreasing order of delay
 */

void proc3(void)
{
    char *ptr;
    int delay[] =  {0, 20, 30, 50, 60};
    int i=0;
    int num_msgs = 5;
    MSG_BUF *p_msg;
    void *p_blk;
    

    uart1_put_string("proc3: entering..., starting delay_send\r\n");    
    while( i < num_msgs ) {
        p_blk = request_memory_block();
#ifdef DEBUG_0
        printf("=0%x, i =[%d], delay= %d\r\n",p_blk, i, delay[i]);
#endif // DEBUG_0
        
        p_msg = p_blk;
        p_msg->mtype = DEFAULT;
        ptr = (char *)p_msg->mtext;
        *ptr++ = 'a' + i;
        *ptr++ = '\r';
        *ptr++ = '\n';
        *ptr++ = '\0';
#ifdef DEBUG_0
        uart1_put_string("proc3: delayed_send to proc4...\r\n");
#endif // DEBUG_0
        delayed_send(PID_P4, p_blk, delay[i++]); 
    }
    uart1_put_string("proc3: done with delay_send\r\n");
    
    while ( 1 ) {
        release_processor();
    }
}

/**
 * @brief a process that receives messages
 */

void proc4(void)
{
    MSG_BUF *p_msg;
    void    *p_blk;
    int     send_id;
    
    uart1_put_string("proc4: entering...\r\n");
    while(1) {
        p_blk = receive_message(&send_id);
        p_msg = p_blk;
        uart1_put_string("proc4: received ");
        uart1_put_string(p_msg->mtext);
#ifdef DEBUG_0
        uart1_put_string("proc4: forwarding it to p6\r\n");
#endif // DEBUG_0
        send_message(PID_P6, p_msg);
        release_processor();

    }
}

void proc5(void)
{
    while(1) {
        uart1_put_string("proc5:\r\n");
        release_processor();
    }
}

void proc6(void)
{
    MSG_BUF *p;
    int send_id;
    int num_msgs = 0;
  
    p = request_memory_block();
    p->mtype = KCD_REG;
    p->mtext[0] = '%';
    p->mtext[1] = 'A';
    p->mtext[2] = '\0';
    uart1_put_string("proc6: registering %A command\r\n");
    send_message(PID_KCD, p);
  
    while (1) {
        p = receive_message(&send_id);
        if ( p->mtype == KCD_CMD || p->mtype == DEFAULT ) {
            if ( p->mtext[0] == '%' && p->mtext[1] == 'A') {
                uart1_put_string("proc6: asking CRT to display :-):-D.\r\n");
                p->mtype = CRT_DISPLAY;
                p->mtext[0] = ':';
                p->mtext[1] = '-';
                p->mtext[2] = ')';
                p->mtext[3] = ':';
                p->mtext[4] = '-';
                p->mtext[5] = 'D';
                p->mtext[6] = '\r';
                p->mtext[7] = '\n';
                p->mtext[8] = '\0';         
                send_message(PID_CRT, p);
            } else {
                uart1_put_string("proc6: received ");
                uart1_put_string(p->mtext);
                release_memory_block(p);
                num_msgs++;
            }
            if (num_msgs == 5) {
                uart1_put_string("proc6: type %A in RTX console now.\r\n");
            }
        }
    }
}
