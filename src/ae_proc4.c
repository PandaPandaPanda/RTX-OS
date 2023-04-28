/*
 **************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTOS LAB  
 *
 *                     Copyright 2020-2022 Yiqing Huang
 *
 *          This software is subject to an open source license and 
 *          may be freely redistributed under the terms of MIT License.
 **************************************************************************
 */

/**************************************************************************//**
 * @file        ae_proc*.c
 * @brief       Two auto test processes: proc1 and proc2
 *              
 * @version     V1.2022.01
 * @authors     Kevin Xu
 * @date        2023 JAN
 * @note        Each process is in an infinite loop. Processes never terminate.
 * @details
 *
 *****************************************************************************/
 /*---------------------------------------------------------------------------- 
 * Test single request, single release. Concurrent request, concurrent request
 * Expected COM1 Output 
 * Assume we only have SIX memory blocks in the system.
 * Expected UART output: (assuming memory block has ownership.):
 * 
 *-------------------------------------------------------------------------------*/ 

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


const char   PREFIX[]      = "G8-TS4";
const char   PREFIX_LOG[]  = "G8-TS4-LOG ";
const char   PREFIX_LOG2[] = "G8-TS4-LOG2";

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
        procs[i].m_stack_size = USR_SZ_STACK;
    }
  
    procs[0].mpf_start_pc = &proc1;
	procs[0].m_priority   = HIGH;
	
	procs[1].mpf_start_pc = &proc2;
	procs[1].m_priority   = MEDIUM;
	
	procs[2].mpf_start_pc = &proc3;
	procs[2].m_priority   = MEDIUM;
	
	procs[3].mpf_start_pc = &proc4;
	procs[3].m_priority   = LOW;
	
	procs[4].mpf_start_pc = &proc5;
	procs[4].m_priority   = LOW;
	
	procs[5].mpf_start_pc = &proc6;
	procs[5].m_priority   = LOWEST;
}

/*
Expected order of operation: 
1. Proc1 gets memory
2. Premption to P2
3. P2 gets blocked, next ready process runs (P3)
4. P3 gets blocked. P2 gets set to lowest in blocked queue. next ready process runs (P4)
5. P4 gets blocked. P3 gets set to Med in blocked queue. next ready process runs (P5)
6. P5 gives control back to P1
7. P1 releases three blocks of memory so P2, P3, P4 gets unblocked
8. The unblocked processes gets run in the order of P3, P4, P2

Expected output:

proc1 starting
proc1: priority to lowest
proc2 starting
ABCDE
FGHIJ
KLMNO
PQRST
UVWXY
ZABCD
EFGHI
JKLMN
OPQRS
TUVWX
YZABC
DEFGH
IJKLM
NOPQR
STUVW
XYZAB
CDEFG
HIJKL
MNOPQ
RSTUV
WXYZA
BCDEF
GHIJK
LMNOP
QRSTU
VWXYZ
ABCDE
FGHIJ
KLMNO
PQRST
proc3 starting
proc4 starting
proc5 starting
Shifting control to proc1
proc1: restarting
proc3: end of testing
proc2: end of testing
*/

/**************************************************************************//**
 * @brief: a process that allocating one block of memory and them release it
 *****************************************************************************/
void proc1(void)
{
    uart1_put_string("proc1 starting\n\r");
    int i = 0;
    void *p_mem_blk1 = request_memory_block();
    void *p_mem_blk2 = request_memory_block();
    void *p_mem_blk3 = request_memory_block();
    uart1_put_string("proc1: priority to lowest\n\r");
    set_process_priority(PID_P1, LOWEST);

    uart1_put_string("proc1: restarting\n\r");
    release_memory_block(p_mem_blk1); // gives memory to p3
    release_memory_block(p_mem_blk2); // gives memory to p4
    release_memory_block(p_mem_blk3); // gives memory to p2
    set_process_priority(PID_P1, LOWEST);
}
/**************************************************************************//**
 * @brief  a process that alternately request and free memory and release processor 3 times
 *****************************************************************************/
void proc2(void)
{
    uart1_put_string("proc2 starting\n\r");
    int i = 0;
    void *p_mem_blk;
    while ( 1 ) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            p_mem_blk = request_memory_block(); // gets blocked

            if (!p_mem_blk) {
                break;
            }
#ifdef DEBUG_0
            printf("proc2: p_mem_blk=0x%x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_char('A' + i%26);
        i++;
    }
    uart1_put_string("proc2: end of testing\n\r");
    test_exit();
}

/**************************************************************************//**
 * @brief  a process that alternately request and free memory and release processor 3 times
 *****************************************************************************/
void proc3(void)
{
    uart1_put_string("proc3 starting\n\r");
    set_process_priority(PID_P2, LOW);
    void *p_mem_blk = request_memory_block(); // gets blocked
    uart1_put_string("proc3: end of testing\n\r");
    set_process_priority(PID_P3, LOWEST);
}

void proc4(void)
{
    uart1_put_string("proc4 starting\n\r");
    set_process_priority(PID_P3, MEDIUM);
    void *p_mem_blk = request_memory_block(); // gets blocked
    uart1_put_string("proc4: end of testing\n\r");
    set_process_priority(PID_P4, LOWEST);
}

void proc5(void)
{
    uart1_put_string("proc5 starting\n\r");
    uart1_put_string("Shifting control to proc1\n\r");
    set_process_priority(PID_P1, HIGH);
    set_process_priority(PID_P5, LOWEST);
    test_exit();
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