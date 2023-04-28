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
 * @file        main_svc.c
 * @brief       main routine to start up the RTX and processes 
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 * @note        Standdard C library is not allowed in the final kernel code.
 *              A tiny printf function for embedded application development
 *              taken from http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
 *              is configured to use UAR1 to output.
 *              Note that init_printf(NULL, putc) must be called to initialize 
 *              the printf function.
 *****************************************************************************/

#include <LPC17xx.h>
#include "rtx.h"
#include "uart_polling.h"
#include "printf.h"
#include "ae.h"
#include "null_proc.h"
#include "timer.h"
#include "uart_irq.h"
#include "kcd_proc.h"
#include "crt_proc.h"
#include "k_inc.h"
#include "wall_clock.h"
#include "set_priority_proc.h"
#include "procs_abc.h"

/**************************************************************************//**
 * @brief   	system set up before calling rtx_init()          
 * @return      0 on success and non-zero on failure
 * @note        leave empty if there is nothing to be done 
 *****************************************************************************/
int pre_rtx_init(void *arg) 
{
    U32 ctrl = 0;
    ctrl = __get_CONTROL();
    __set_CONTROL(ctrl | BIT(0));

    // set arg (which is test_proc) to the null process
    PROC_INIT nullProcess;
    // populate fields of null process
    nullProcess.m_pid =  0;
    nullProcess.m_priority = PRI_NULL;
    nullProcess.m_stack_size = USR_SZ_STACK;
    nullProcess.mpf_start_pc = &null_process;
    // add to test_proc
    PROC_INIT *test_procs = (PROC_INIT*)arg;
	test_procs[6] = nullProcess;
    int i = 7;

#ifdef TIMER_IPROC 
    // timer proc init setup
    PROC_INIT timer;
    timer.m_pid = PID_TIMER_IPROC;
    timer.m_priority = PRI_IPROC;
    timer.m_stack_size = STACK_SIZE_IPROC;
    timer.mpf_start_pc = &c_TIMER0_IRQHandler;
    test_procs[i] = timer;
    i++;
#endif 

#ifdef UART_IPROC
    // uart proc init setup
    PROC_INIT uart;
    uart.m_pid = PID_UART_IPROC;
    uart.m_priority = PRI_IPROC;
    uart.m_stack_size = STACK_SIZE_IPROC;
    uart.mpf_start_pc = &c_UART0_IRQHandler;
    test_procs[i] = uart;
    i++;

    // KCD proc init setup
    PROC_INIT kcd;
    kcd.m_pid = PID_KCD;
    kcd.m_priority = HIGH;
    kcd.m_stack_size = USR_SZ_STACK;
    kcd.mpf_start_pc = &kcd_proc;
    test_procs[i] = kcd;
    i++;

    // CRT proc init setup
    PROC_INIT crt;
    crt.m_pid = PID_CRT;
    crt.m_priority = HIGH;
    crt.m_stack_size = USR_SZ_STACK;
    crt.mpf_start_pc = &crt_proc;
    test_procs[i] = crt;
    i++;
  
    // clock proc init setup
    PROC_INIT clock;
    clock.m_pid = PID_CLOCK;
    clock.m_priority = HIGH;
    clock.m_stack_size = USR_SZ_STACK;
    clock.mpf_start_pc = &wall_clock;
    test_procs[i] = clock;
    i++;

    // set priority proc init setup
    PROC_INIT set_prio;
    set_prio.m_pid = PID_SET_PRIO;
    set_prio.m_priority = HIGH;
    set_prio.m_stack_size = USR_SZ_STACK;
    set_prio.mpf_start_pc = &set_priority_proc;
    test_procs[i] = set_prio;
    i++;
#endif

#ifdef STRESS_TESTING
    // process A init setup
    PROC_INIT a;
    a.m_pid = PID_A;
    a.m_priority = HIGH;
    a.m_stack_size = USR_SZ_STACK;
    a.mpf_start_pc = &proc_a;
    test_procs[i] = a;
    i++;

    // process B init setup
    PROC_INIT b;
    b.m_pid = PID_B;
    b.m_priority = HIGH;
    b.m_stack_size = USR_SZ_STACK;
    b.mpf_start_pc = &proc_b;
    test_procs[i] = b;
    i++;

    // process C init setup
    PROC_INIT c;
    c.m_pid = PID_C;
    c.m_priority = HIGH;
    c.m_stack_size = USR_SZ_STACK;
    c.mpf_start_pc = &proc_c;
    test_procs[i] = c;
    i++;
#endif

    // add to test_proc
    arg = (void*)test_procs;
    // Anything you want to do before calling rtx_init() 
    return NULL;
}

/**************************************************************************//**
 * @brief   	main routine
 *          
 * @return      0 on success and non-zero on failure
 *****************************************************************************/
int main() 
{   
    /* user test process initialization table */
    PROC_INIT test_procs[TOTAL_NUM_PROCS];//add one for null process
    U32 ctrl = 0;
    
    /* CMSIS system initialization */
    SystemInit();
    
    __disable_irq();
    
    /* uart1 by polling */  
    uart1_init();                        
    
    /* initialize printf to use uart1 by polling */
    init_printf(NULL, putc);
    
    __enable_irq();
    
#ifdef SE350_DEMO
    printf("Dereferencing Null to get inital SP = 0x%x\r\n", *(U32 *)(IROM_BASE));
	printf("Derefrencing Reset vector to get intial PC = 0x%x\r\n", *(U32 *)(IROM_BASE + 4));
    ctrl = __get_CONTROL();
    printf("ctrl = %d, We are at privileged level, so we can access SP.\r\n", ctrl); 
	printf("Read MSP = 0x%x\r\n", __get_MSP());
	printf("Read PSP = 0x%x\r\n", __get_PSP());
	
	/* transit to unprivileged level, default MSP is used */
    __set_CONTROL(ctrl | BIT(0));
    ctrl = __get_CONTROL();
    printf("ctrl= %d, We are at unprivileged level, we cannot access SP.\r\n", ctrl);
	printf("Cannot read MSP = 0x%x\r\n", __get_MSP());
	printf("Cannot read PSP = 0x%x\r\n", __get_PSP());

#endif /* SE350_DEMO */
    
    /* initialize the third-party testing framework */
    ae_init(test_procs, NUM_TEST_PROCS, &pre_rtx_init, test_procs); // only initialize user test procs and exclude NULL process
    
    /* start the RTX and built-in processes */
    rtx_init(test_procs, TOTAL_NUM_PROCS);  
  
    /* We should never reach here!!! */
    return RTX_ERR;  
}
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
