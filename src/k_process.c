/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO SE 350 RTX LAB  
 *
 *           Copyright 2020-2022 Yiqing Huang and Thomas Reidemeister
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
 * @file        k_process.c 
 * @brief       kernel process management C file 
 *              
 * @version     V1.2022.01
 * @authors     Yiqing Huang, Thomas Reidemeister
 * @date        2022 JAN
 * @note        The example code shows one way of implementing context switching.
 *              The code only has minimal sanity check. 
 *              There is no stack overflow check.
 *              The implementation assumes only two simple user processes and 
 *              NO HARDWARE INTERRUPTS. 
 *              The purpose is to show one way of doing context switch
 *              under stated assumptions. 
 *              These assumptions are not true in the required RTX Project!!!
 *              If you decide to use this piece of code, 
 *              you need to understand the assumptions and the limitations.
 *
 *****************************************************************************/

#include "k_process.h"
#include "k_rtx.h"
#include "priority_queue.h"
#include "k_memory.h"
/*
 *==========================================================================
 *                            GLOBAL VARIABLES
 *==========================================================================
 */
 
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */
PCB *gp_pcb_timer_iproc = NULL; /* points to Timer iprocess pcb */ 
PCB *gp_pcb_uart_iproc = NULL;  /* points to the UART iprocess pcb */
PCB *gp_pcb_kcd_iproc = NULL;  /* points to the KCD pcb */
PCB *gp_pcb_crt_iproc = NULL;  /* points to the CRT pcb */
PCB *gp_pcb_interrupted;        /* interrupted process's pcb    */

/* process initialization table */
PROC_INIT g_proc_table[TOTAL_NUM_PROCS];
PriorityQueue ready_queue;
PriorityQueue blocked_on_receive_queue;
int num_of_processes;

/**************************************************************************//**
 * @biref initialize all processes in the system
 * @note  We assume there are only two user processes in the system in 
 *        this example starter code.
 *        The preprocessor ECE350_DEMO should be defined
 *****************************************************************************/
void process_init(PROC_INIT *proc_info, int num) 
{
	num_of_processes = num;
	int i;
	U32 *sp;
  
    /* fill out the initialization table */
#ifdef SE350_DEMO
	for ( i = 0; i < 2; i++ ) {
#else
    for ( i = 0; i < num; i++ ) {
#endif /* SE350_DEMO */
		g_proc_table[i].m_pid        = proc_info[i].m_pid;
		g_proc_table[i].m_stack_size = proc_info[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = proc_info[i].mpf_start_pc;
		g_proc_table[i].m_priority   = proc_info[i].m_priority;
	}
  
	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < NUM_TEST_PROCS+1; i++ ) {
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->m_has_mem_blk = 0;
		(gp_pcbs[i])->mp_next = NULL;
		MsgQueue msg_queue = msg_queue_Init();
		(gp_pcbs[i])->msg_queue = msg_queue;

		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( int j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
	}

#ifdef TIMER_IPROC
    /* Timer i-proc initialization */
    gp_pcbs[i]->m_pid = PID_TIMER_IPROC;
    gp_pcbs[i]->m_state = IPROC;
	gp_pcbs[i]->m_priority = PRI_IPROC;
	gp_pcbs[i]->m_has_mem_blk = 0;
	gp_pcbs[i]->mp_next = NULL;
	MsgQueue timer_msg_queue = msg_queue_Init();
	gp_pcbs[i]->msg_queue = timer_msg_queue;
    gp_pcbs[i]->mp_sp = alloc_stack(STACK_SIZE_IPROC);
	gp_pcb_timer_iproc = get_pcb_by_pid(PID_TIMER_IPROC);
    /* NOTE we do not need to create exception stack frame for an IPROC
       since they are running in handler mode and never get into the handler
       mode from the thread mode and they never exit from the handler mode
       back to thread mode either 
    */
	i++;
#endif

#ifdef UART_IPROC
    /* UART i-proc initialization */
    gp_pcbs[i]->m_pid = PID_UART_IPROC;
    gp_pcbs[i]->m_state = IPROC;
	gp_pcbs[i]->m_priority = PRI_IPROC; // The priority ,ight need to change?
	gp_pcbs[i]->m_has_mem_blk = 0;
	gp_pcbs[i]->mp_next = NULL;
	MsgQueue uart_msg_queue = msg_queue_Init();
	gp_pcbs[i]->msg_queue = uart_msg_queue;
    gp_pcbs[i]->mp_sp = alloc_stack(STACK_SIZE_IPROC);
	gp_pcb_uart_iproc = get_pcb_by_pid(PID_UART_IPROC);
    /* NOTE we do not need to create exception stack frame for an IPROC
       since they are running in handler mode and never get into the handler
       mode from the thread mode and they never exit from the handler mode
       back to thread mode either 
    */
	i++;

	/* KCD proc initialization */
	gp_pcbs[i]->m_pid = PID_KCD;
    gp_pcbs[i]->m_state = NEW;
	gp_pcbs[i]->m_priority = HIGH; // The priority ,ight need to change?
	gp_pcbs[i]->m_has_mem_blk = 0;
	gp_pcbs[i]->mp_next = NULL;
	MsgQueue kcd_msg_queue = msg_queue_Init();
	gp_pcbs[i]->msg_queue = kcd_msg_queue;
	sp = alloc_stack((g_proc_table[i]).m_stack_size);
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
	for (int j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[i])->mp_sp = sp;
	gp_pcb_kcd_iproc = get_pcb_by_pid(PID_KCD);
	i++;
	
	/* CRT proc initialization */
	gp_pcbs[i]->m_pid = PID_CRT;
    gp_pcbs[i]->m_state = NEW;
	gp_pcbs[i]->m_priority = HIGH; // The priority ,ight need to change?
	gp_pcbs[i]->m_has_mem_blk = 0;
	gp_pcbs[i]->mp_next = NULL;
	MsgQueue crt_msg_queue = msg_queue_Init();
	gp_pcbs[i]->msg_queue = crt_msg_queue;
    sp = alloc_stack((g_proc_table[i]).m_stack_size);
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
	for (int j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[i])->mp_sp = sp;
	gp_pcb_crt_iproc = get_pcb_by_pid(PID_CRT);
	i++;

	/* clock proc initialization */
	gp_pcbs[i]->m_pid = PID_CLOCK;
    gp_pcbs[i]->m_state = NEW;
	gp_pcbs[i]->m_priority = HIGH; // The priority ,ight need to change?
	gp_pcbs[i]->m_has_mem_blk = 0;
	gp_pcbs[i]->mp_next = NULL;
	MsgQueue clock_msg_queue = msg_queue_Init();
	gp_pcbs[i]->msg_queue = clock_msg_queue;
    sp = alloc_stack((g_proc_table[i]).m_stack_size);
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
	for (int j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[i])->mp_sp = sp;
	i++;

	/* set priority proc initialization */
	gp_pcbs[i]->m_pid = PID_SET_PRIO;
    gp_pcbs[i]->m_state = NEW;
	gp_pcbs[i]->m_priority = HIGH; // The priority ,ight need to change?
	gp_pcbs[i]->m_has_mem_blk = 0;
	gp_pcbs[i]->mp_next = NULL;
	MsgQueue set_priority_proc_msg_queue = msg_queue_Init();
	gp_pcbs[i]->msg_queue = set_priority_proc_msg_queue;
    sp = alloc_stack((g_proc_table[i]).m_stack_size);
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
	for (int j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[i])->mp_sp = sp;
	i++;

#endif

#ifdef STRESS_TESTING
	for (int k = 0; k < 3; k++) {
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->m_has_mem_blk = 0;
		(gp_pcbs[i])->mp_next = NULL;
		MsgQueue msg_queue = msg_queue_Init();
		(gp_pcbs[i])->msg_queue = msg_queue;

		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for (int j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
		i++;
	}
#endif
	
	// initialize ready queue
	ready_queue = priorityQueueInit();
	// add all processes to ready
	for (i = 0; i < num ; i++){
		if (gp_pcbs[i]->m_pid != PID_TIMER_IPROC && gp_pcbs[i]->m_pid != PID_UART_IPROC) {
			priorityEnqueue(&ready_queue, gp_pcbs[i]);
		}
	}
	// initialize this just for debugging
	blocked_on_receive_queue = priorityQueueInit();
}

void handle_process_ready(PCB *ptr) {
	// set to ready
	ptr->m_state = RDY;
	// add to ready queue
	priorityEnqueue(&ready_queue, ptr);
}

/**************************************************************************//**
 * @brief   scheduler, pick the pid of the next to run process
 * @return  current_process | highest on pq
 *          NULL if error happens
 * @post    if gp_current_process was NULL, then it gets set to pcbs[0].
 *          No other effect on other global variables.
 *****************************************************************************/
PCB *scheduler(void)
{
	// Assumption: current process is always running
	// Get the highest priority process
	PCB* highest_ready = priorityPeek(&ready_queue);
	if (highest_ready == NULL) {
		return gp_current_process;
	}

	if(gp_current_process->m_state == BLOCKED_ON_MEM || gp_current_process->m_state == BLOCKED_ON_RECEIVE){
		return highest_ready;
	}
	
	if (highest_ready->m_priority <= gp_current_process->m_priority || gp_current_process->m_pid == PID_UART_IPROC) {
		return highest_ready;
	} 
	
	return gp_current_process;
}

/**************************************************************************//**
 * @brief     pick the next pcb periodically 
 *            SIM_TARGET: period = 200 clock ticks
 *            otherwise:   period = 12000 clock ticks
 * @return    PCB pointer of the next to run process
 *            NULL if error happens
 * @post      if gp_current_process was NULL, then it gets set to pcbs[0].
 *            No other effect on other global variables.
 * @attention You should write your own scheduler, do not use this one in your
 *            project. This is only to demonstrate how a timer interrupt
 *            can affect the scheduling decision.
 *****************************************************************************/

PCB *scheduler_tms(void)
{
#ifdef SIM_TARGET
    if ( g_timer_count %200 == 0 ) {
#else
    if ( g_timer_count %12000 == 0 ) {
#endif
        return scheduler();
    } 
    return gp_current_process;
}

/**************************************************************************//**
 * @brief   switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 * @param   p_pcb_old, the old pcb that was in RUN
 * @return  RTX_OK upon success
 *          RTX_ERR upon failure
 * @pre     p_pcb_old and gp_current_process are pointing to valid PCBs.
 * @post    p_pcb_old == gp_current_process		Do nothing
 * 			p_pcb_old != gp_current_process		Update ready_queue and switch process
 *****************************************************************************/

int process_switch(PCB *p_pcb_old) 
{
	PROC_STATE_E state;
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			if( p_pcb_old->m_state == RUN ){
				p_pcb_old->m_state = RDY;
			}
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
        priorityDequeue(&ready_queue);

        U32 ctrl = __get_CONTROL();
        ctrl &= ~BIT(0);    // clear bit 0, want to be at priv. level when exit from the kernel
        __set_CONTROL(ctrl);
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == RDY){ 		
			if( p_pcb_old->m_state == RUN ){
				p_pcb_old->m_state = RDY;
			}
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack  
			priorityRemove(&ready_queue, gp_current_process->m_pid);
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}

/**************************************************************************//**
 * @brief   release_processor(). 
 * @return  RTX_ERR on error and zero on success
 * @post    gp_current_process gets updated to next to run process
 *****************************************************************************/

int k_release_processor(void)
{
	PCB *p_pcb_old = NULL;
	
	p_pcb_old = gp_current_process;

	if(gp_current_process->m_state == RUN){
		gp_current_process->m_state = RDY;
		priorityEnqueue(&ready_queue, gp_current_process);
	}

	gp_current_process = scheduler();
	
	if (gp_current_process == p_pcb_old) {
		gp_current_process->m_state = RUN;
		priorityRemove(&ready_queue, gp_current_process->m_pid);
	}
	
	if ( gp_current_process == NULL  ) {
		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}

	if ( p_pcb_old == NULL ) {
		p_pcb_old = gp_current_process;
	}

	process_switch(p_pcb_old);
	return RTX_OK;
}

// /**************************************************************************//**
//  * @brief   run a new process based on the scheduler_tms() 
//  * @return  RTX_ERR on error and zero on success
//  * @post    gp_current_process gets updated to next to run process
//  *****************************************************************************/
int k_run_new_process(void)
{
    PCB *p_new_pcb = NULL;
    PCB *p_old_pcb = gp_current_process;
    
    if (gp_current_process == NULL) {
        return RTX_ERR;
    }
    
    // making scheduling decision
    p_new_pcb = scheduler_tms();
    
    if ( p_new_pcb == gp_current_process ) {
        return RTX_OK;
    }
    
    if ( p_new_pcb == NULL) {
        return RTX_ERR;
    }
    
    gp_current_process = p_new_pcb;
    process_switch(p_old_pcb);
    return RTX_OK;
}

int k_get_process_priority(int pid)
{
	if (pid == PID_NULL || pid == PID_TIMER_IPROC) {
		return RTX_ERR;
	}

	for (int i = 0; i < num_of_processes; i++){
		if (pid == (gp_pcbs[i])->m_pid) {
			return (gp_pcbs[i])->m_priority;
		}
	}
    return -1;
}

int k_set_process_priority(int pid, int prio_new) 
{
	if (prio_new > LOWEST || prio_new < HIGH) {
		printf("Invalid command input\n");
		return RTX_ERR;
	}

	if (pid == PID_NULL || pid == PID_TIMER_IPROC || pid == PID_UART_IPROC
		|| pid == PID_CRT || pid == PID_KCD) {
		return RTX_ERR;
	}

	for (int i = 0; i < num_of_processes; i++){
		if (pid == (gp_pcbs[i])->m_pid) {
			if (gp_pcbs[i]->m_priority == prio_new) {
				return RTX_OK;
			}
			(gp_pcbs[i])->m_priority = prio_new;

			// remove from its queue
			PCB *ptr_ready = priorityRemove(&ready_queue, pid);
			PCB *ptr_blocked = priorityRemove(&blocked_on_mem_queue, pid);

			// add to new location based on new priority
			if (ptr_ready ) {
				priorityEnqueue(&ready_queue, ptr_ready);
			} else if (ptr_blocked) {
				priorityEnqueue(&blocked_on_mem_queue, ptr_blocked);
			} else if (gp_pcbs[i] -> m_state == RUN){
				PCB* highest_ready = priorityPeek(&ready_queue);
				if(highest_ready->m_priority < gp_pcbs[i]->m_priority){
					gp_pcbs[i]->m_state = RDY;
					priorityEnqueue(&ready_queue, gp_pcbs[i]);
					k_release_processor();
					return RTX_OK;
				}
			}

			// if process changed has new priority, we need to reschedule. This line should have no effect if new prio is lower than current running process
			// if there are other process with the same priority as current process, then the current process would be preempted. 
			if (gp_current_process->m_priority > prio_new) {
				k_release_processor();
			}
			return RTX_OK;
		}
	}
    return RTX_ERR;
}

PCB *get_pcb_by_pid(int pid) {
	for (int i = 0; i < num_of_processes; i++){
		if (pid == (gp_pcbs[i])->m_pid) {
			return gp_pcbs[i];
		}
	}
	return NULL;
}

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
