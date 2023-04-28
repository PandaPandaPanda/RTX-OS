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
 * @file        ae_proc*.c
 * @brief       test processes template file
 *              
 * @version     V1.2022.01
 * @authors     Yiqing Huang
 * @date        2022 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 *              This file needs to be completed by students.
 *
 *****************************************************************************/
/*---------------------------------------------------------------------------- 
 * Expected COM1 Output 
 * Assume we have 32 memory blocks in the system.
 * Expected UART output: (assuming memory block has ownership.):
 * proc1: 
 * allocate one block of memory
 * set proc1 to LOW priority
 * proc2: 
 * call requests for 32 memory blocks
 * A
 * B
 * C
 * D
 * E
 * F
 * G
 * H
 * I
 * J
 * K
 * L
 * M
 * N
 * O
 * P
 * Q
 * R
 * S
 * T
 * U
 * V
 * W
 * X
 * Y
 * Z
 * A
 * B
 * C
 * D
 * E
 * back to proc1
 * release one block of memory
 * F
 * back to proc2
 *-------------------------------------------------------------------------------*/

#include "rtx.h"
#include "uart_polling.h"
#include "printf.h"
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


const char   PREFIX[]      = "G8-TS2";
const char   PREFIX_LOG[]  = "G8-TS2-LOG ";
const char   PREFIX_LOG2[] = "G8-TS2-LOG2";

AE_XTEST     g_ae_xtest;                // test data, re-use for each test
AE_CASE      g_ae_cases[NUM_TESTS];
AE_CASE_TSK  g_tsk_cases[NUM_TESTS];

#endif // AE_ENABLE


/* initialization table item */
void set_test_procs(PROC_INIT *procs, int num)
{
    int i;
    for( i = 0; i < num; i++ ) {
        procs[i].m_pid        = (U32)(i+1);
        procs[i].m_priority   = LOWEST;
        procs[i].m_stack_size = USR_SZ_STACK;
    }
  
    procs[0].mpf_start_pc = &proc1;
    procs[0].m_priority   = HIGH; 
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = MEDIUM;
    procs[2].mpf_start_pc = &proc3;
    procs[3].mpf_start_pc = &proc4;
    procs[4].mpf_start_pc = &proc5;
    procs[5].mpf_start_pc = &proc6;
}

/*
set P1(HIGH) to running and allocate one memory block
set priority of P1 to LOW
set P2(MEDIUM) to running and make it blocked by requesting 32 blocks of memory
P1 should run next
P1 should release the one block of memory it owns

Expected behaviour:
 - P2 should start to run 
*/

void proc1(void)
{
    uart1_put_string("proc1: \n\r");
    uart1_put_string("allocate one block of memory\n\r");
    void* p_mem_blk = request_memory_block();
    uart1_put_string("set proc1 to LOW priority\n\r");
    set_process_priority(PID_P1, LOW);

    uart1_put_string("back to proc1\n\r");
    uart1_put_string("release one block of memory\n\r");
    int ret_val = release_memory_block(p_mem_blk);
}

void proc2(void)
{
    uart1_put_string("proc2: \n\r");
    const int num_mem_blks = 32;
    uart1_put_string("call requests for 32 memory blocks\n\r");
    for (int i = 0; i < num_mem_blks; i++) {
        void* p_mem_blk = request_memory_block();
				uart1_put_char('A' + i%26);
				uart1_put_string("\n\r");
    }

    uart1_put_string("back to proc2\n\r");
    test_exit();
}

void proc3(void)
{
    
    while(1) {
        uart1_put_string("proc3: \n\r");
        release_processor();
    }
}

void proc4(void)
{
    while(1) {
        uart1_put_string("proc4: \n\r");
        release_processor();
    }
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
