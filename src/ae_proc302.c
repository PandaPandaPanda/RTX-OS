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
 * @file        ae_proc302.c
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
 * 
 * should start with proc 1 (PRIORITY_MEDIUM = 1), which prints
 * proc1
 * proc1
 * ...
 * and should switch to proc 2 (PRIORITY_LOW = 2) when a command is entered to set 
 * the priority of proc 1 to LOWEST = 3
 * proc 2 should print
 * proc2
 * proc2
 * ...
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
    procs[0].m_priority   = MEDIUM; 
    procs[1].mpf_start_pc = &proc2;
    procs[1].m_priority   = LOW;
    procs[2].mpf_start_pc = &proc3;
    procs[3].mpf_start_pc = &proc4;
    procs[4].mpf_start_pc = &proc5;
    procs[5].mpf_start_pc = &proc6;
}

void proc1(void) // starts with PRIORITY_MEDIUM = 1
{
    int i = 0;
    while(1) {
        if (i % 100000 == 0) {
            uart1_put_string("proc1: \n\r");
        }
        release_processor();
        i++;
    }
}

void proc2(void) // starts with PRIORITY_LOW = 2
{
    int i = 0;
    while(1) {
        if (i % 100000 == 0) {
            uart1_put_string("proc2: \n\r");
        }
        release_processor();
        i++;
    }
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
