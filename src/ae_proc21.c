/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTX LAB  
 *
 *                     Copyright 2020-2022 Yiqing Huang
 *                          All rights reserved.
 *---------------------------------------------------------------------------
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice and the following disclaimer.
 *
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/**************************************************************************//**
 * @brief   P2 public testing code TS201
 * @author  Yiqing Huang
 * @date    2022/02/04
 * @note    Each process is in an infinite loop. Processes never terminate.
 *****************************************************************************/

/* expected output at COM1 (polled terminal) 
proc2: receiving messages ...
proc1: requesting a mem_blk...
proc1: send messages to proc2...
proc1: send a message to proc2...
proc1: send a message to proc2...
proc1: send a message to proc2...
proc1: send a message to proc2...
proc1: send a message to proc2...

proc2: got a message - A
proc2: got a message - 0
proc2: got a message - 1
proc2: got a message - 2
proc2: got a message - 3
proc2: got a message - 4
*/


#include "rtx.h"
#include "ae_proc.h"
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


const char   PREFIX[]      = "G99-TS201";
const char   PREFIX_LOG[]  = "G99-TS201-LOG ";
const char   PREFIX_LOG2[] = "G99-TS201-LOG2";

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
    procs[0].m_priority   = HIGH;
    
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = MEDIUM;
    
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


/**
 * @brief: a process that prints five uppercase letters on each line
 *         and sends a message to P2
 */
void proc1(void)
{
    void    *p_blk;
    MSG_BUF *p_msg;
    char    *ptr;
    
    for (int i = 0; i < 5; i++) {
        //ret_val = release_processor();
        p_blk = request_memory_block();
        p_msg = p_blk;
        p_msg->mtype = DEFAULT;
        ptr = p_msg->mtext;
        *ptr++ = ('A' + (i)%10);
        *ptr++ = '\r';
        *ptr++ = '\n';
        *ptr++ = '\0';
        uart1_put_string("proc1: send a message to proc2 - ");
        uart1_put_string(p_msg->mtext);
        send_message(PID_P2, p_blk);
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
    MSG_BUF *p_msg;
    void    *p_blk;
    
    for (int i = 0; i < 5; i++) {
        p_blk = receive_message(NULL);
        p_msg = p_blk;
        uart1_put_string("proc2: got a message - ");
        uart1_put_string(p_msg->mtext);
        release_memory_block(p_blk);
    }
    test_exit();
}

void proc3(void)
{
    while(1) {
        uart1_put_string("proc5:\r\n");
        release_processor();
    }
}

void proc4(void)
{
    while(1) {
        uart1_put_string("proc5:\r\n");
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
    while(1) {
        uart1_put_string("proc6:\r\n");
        release_processor();
    }
}
