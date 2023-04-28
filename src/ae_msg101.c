/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTOS LAB  
 *
 *                     Copyright 2020-2022 Yiqing Huang
 *
 *          This software is subject to an open source license and 
 *          may be freely redistributed under the terms of MIT License.
 ****************************************************************************
 */

/**************************************************************************//**
 * @file        ae_proc_101.c
 * @brief       Two auto test processes: proc1 and proc2
 *              
 * @version     V1.2022.01
 * @authors     Yiqing Huang
 * @date        2022 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 * @details
 *
 *****************************************************************************/
/*---------------------------------------------------------------------------- 

 *---------------------------------------------------------------------------*/
#include "rtx.h"
#include "uart_polling.h"
#include "ae_proc.h"
#include "ae_util.h"

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


const char   PREFIX[]      = "G99-TS101";
const char   PREFIX_LOG[]  = "G99-TS101-LOG ";
const char   PREFIX_LOG2[] = "G99-TS101-LOG2";

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
        procs[i].m_stack_size = USR_SZ_STACK;
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
	procs[5].m_priority   = LOWEST;

    g_iterations = 5;
}

/**************************************************************************//**
 * @brief: a process that prints two lines of five uppercase letters
 *         and then changes P2's priority to HIGH
 *         and then yields the cpu.
 *****************************************************************************/
void proc1(void)
{
    int i = 0;
    int j = 0;
    int ret_val = 10;
    void *p_mem_blk;
    void *p_blk;
    MSG_BUF *p_msg;
    char *ptr;
   
    uart1_put_string("proc1: requesting a mem_blk...\n\r");
    p_blk = request_memory_block();
    p_msg = p_blk;
    p_msg->mtype = DEFAULT;
    ptr = p_msg->mtext;
    *ptr++ = 'A';
    *ptr++ = '\n';
    *ptr++ = '\r';
    *ptr++ = '\0';

    uart1_put_string("proc1: send messages to proc2...\n\r");
    send_message(PID_P2, p_blk);

    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            //ret_val = release_processor();
            p_blk = request_memory_block();
            p_msg = p_blk;
            p_msg->mtype = DEFAULT;
            ptr = p_msg->mtext;
            *ptr++ = ('0' + (j++)%10);
            *ptr++ = '\n';
            *ptr++ = '\r';
            *ptr++ = '\0';
            uart1_put_string("proc1: send a message to proc2...\n\r");
            send_message(PID_P2, p_blk);
#ifdef DEBUG_0
            printf("proc1: p_mem_blk=0x%x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        if ( j == g_iterations ) {
            break;
        }
        uart1_put_char('A' + i%26);
        i++;
    }
    set_process_priority(PID_P1, LOWEST);
    while (1) {
            #ifdef DEBUG_0
                printf("proc 1\n");
            #endif
        release_processor();
    }
}

/**************************************************************************//**
 * @brief  a process that prints five numbers, change P1's priority to HIGH
 *         and then yields the cpu.
 *****************************************************************************/
void proc2(void)
{
    int i = 0;
    int j = 0;
    int ret_val = 20;
    void *p_mem_blk;
    MSG_BUF *p_msg;
    void *p_blk;

    uart1_put_string("proc2: receiving messages ...\n\r");
    p_blk = receive_message(NULL);
    p_msg = p_blk;

    uart1_put_string("proc2: got a message - ");
    uart1_put_string(p_msg->mtext);
    release_memory_block(p_blk);
    while (1) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            p_blk = receive_message(NULL);
            p_msg = p_blk;
            uart1_put_string("proc2: got a message - ");
            uart1_put_string(p_msg->mtext);
            release_memory_block(p_blk);
            j++;
#ifdef DEBUG_0
            printf("proc2: ret_val=%d\n", ret_val);
#endif /* DEBUG_0 */
        }
        if ( j == g_iterations ) {
            break;
        }
        uart1_put_char('a' + i%26);
        i++;
    }

    set_process_priority(PID_P2, LOW);

    while(1) {
            #ifdef DEBUG_0
                printf("proc 2\n");
            #endif
        release_processor();
    }
}

void proc3(void)
{
    char *ptr;
    int delay[] =  {5000, 4000, 3000, 2000, 1000};//{60, 45, 30, 15, 0};
    int i=0;
    int num_msgs = 5;
    MSG_BUF *p_msg;
    void *p_blk;

    uart1_put_string("proc3: entering..., starting delay_send\n\r");   

    while( i < num_msgs ) {

#ifdef DEBUG_0

        uart1_put_string("proc3: request mem_blk\n\r");

#endif // DEBUG_0

        p_blk = (MSG_BUF *)request_memory_block();

#ifdef DEBUG_0

        printf("=0%x, i =[%d], delay= %d\n\r",p_blk, i, delay[i]);

#endif // DEBUG_0
        p_msg = p_blk;
        p_msg->mtype = DEFAULT;
        ptr = (char *)p_msg->mtext;
        *ptr++ = 'a' + i;
        *ptr++ = '\n';
        *ptr++ = '\r';
        *ptr++ = '\0';
#ifdef DEBUG_0
        uart1_put_string("proc3: delayed_send to proc4...\n\r");
#endif // DEBUG_0
        delayed_send(PID_P4, p_blk, delay[i++]);
    }
    uart1_put_string("proc3: done with delay_send\n\r");

    while ( 1 ) {
            #ifdef DEBUG_0
            printf("proc 3\n");
            #endif
        release_processor();
    }
}

void proc4(void)
{
    MSG_BUF *p_msg;

    void * p_blk;

    int send_id;

    uart1_put_string("proc4: entering...\n\r");

		int cnt = 0;
    while(cnt < 5) {
        p_blk = receive_message(&send_id);
        p_msg = p_blk;
        uart1_put_string("proc4: received ");
        uart1_put_string(p_msg->mtext) ;
        release_memory_block(p_blk);
				++cnt;
    }
		test_exit();
}


void proc5(void)
{
    while(1) {
        uart1_put_string("proc5: \n\r");
        release_processor();
    }
}

void proc6(void)
{
    while(1) {
        uart1_put_string("proc6: \n\r");
        release_processor();
    }
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */