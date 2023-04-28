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
 * 01234
 * 54321
 * abcde
 * ABCDE
 * fedcb
 * FEDCB
 * abcde
 * ABCDE
 * fedcb
 * FEDCB
 * abcde
 * ABCDE
 * fedcb
 * FEDCB
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


const char   PREFIX[]      = "G8-TS3";
const char   PREFIX_LOG[]  = "G8-TS3-LOG ";
const char   PREFIX_LOG2[] = "G8-TS3-LOG2";

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
	procs[3].m_priority   = LOWEST;
	
	procs[4].mpf_start_pc = &proc5;
	procs[4].m_priority   = LOWEST;
	
	procs[5].mpf_start_pc = &proc6;
	procs[5].m_priority   = LOWEST;
}

/**************************************************************************//**
 * @brief: a process that allocating one block of memory and them release it
 *****************************************************************************/
void proc1(void)
{
    int i = 0;
    void *p_mem_blk;
    while ( 1 ) {
        if ( i != 0 && i%5 == 0 ) {
            uart1_put_string("\n\r");
            p_mem_blk = request_memory_block();
            break;
#ifdef DEBUG_0
            printf("proc1: p_mem_blk=0x%x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_char('0' + i%10);
        i++;
    }
    while ( 1 ) {
        if ( i == 0 ) {
            uart1_put_string("\n\r");
            release_memory_block(p_mem_blk);
            break;
#ifdef DEBUG_0
            printf("proc1: p_mem_blk=0x%x\n", p_mem_blk);
#endif /* DEBUG_0 */
        }
        uart1_put_char('0' + i%10);
        i--;
    }
		
		set_process_priority(PID_P1, LOWEST);
}
/**************************************************************************//**
 * @brief  a process that alternately request and free memory and release processor 3 times
 *****************************************************************************/
void proc2(void)
{
    int i = 0;
		int counter = 0;
    int is_requesting = 1;
    void *p_mem_blk;
    while ( 1 ) {
        if (is_requesting) {
            if ( i != 0 && i%5 == 0 ) {
								uart1_put_string("\n\r");
                p_mem_blk = request_memory_block();
                is_requesting = 0;
								release_processor();
								continue;
            }

            uart1_put_char('a' + i%26);
            i++;
        } else {
            if ( i == 0 ) {
								++counter;
								
								uart1_put_string("\n\r");
                release_memory_block(p_mem_blk);
                is_requesting = 1;
								release_processor();
								continue;
            }
            
            uart1_put_char('a' + i%26);
            --i;
        }

        if (counter >= 3) {
            break;
        }
    }
		test_exit();
}

/**************************************************************************//**
 * @brief  a process that alternately request and free memory and release processor 3 times
 *****************************************************************************/
void proc3(void)
{
    int i = 0;
		int counter = 0;
    int is_requesting = 1;
    void *p_mem_blk;
    while ( 1 ) {
        if (is_requesting) {
            if ( i != 0 && i%5 == 0 ) {
								uart1_put_string("\n\r");
                p_mem_blk = request_memory_block();
                is_requesting = 0;
								release_processor();
								continue;
            }

            uart1_put_char('A' + i%26);
            i++;
        } else {
            if ( i == 0 ) {
								++counter;
								
								uart1_put_string("\n\r");
                release_memory_block(p_mem_blk);
                is_requesting = 1;
								release_processor();
								continue;
            }
            
            uart1_put_char('A' + i%26);
            --i;
        }

        if (counter >= 3) {
            break;
        }
    }
		test_exit();
}

void proc4(void)
{
    for (int i = 0; i < 5; i++) {
        uart1_put_string("proc4: \n\r");
        release_processor();
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
