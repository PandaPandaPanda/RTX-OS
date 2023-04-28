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
 * @file        k_inc.h
 * @brief       Kernel Macros and Data Structure Header file          
 * @version     V1.2022.01
 * @authors     Yiqing Huang
 * @date        2022 JAN
 *
 * @note        all kernel .c files include this one
 *
 *****************************************************************************/
 
#ifndef K_INC_H_
#define K_INC_H_
#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "common.h"
#include "uart_polling.h"
#include "printf.h"
#include "queue.h"

#define PCB_MSP_OFFSET      4       /* mp_sp is 4B offset from pcb struct starting addr.    */
#define PCB_STATE_OFFSET    12      /* m_state is 12B offset from pcb struct starting addr. */
#define STACK_SIZE_IPROC    0x200   /* iprocess stack size */

/* process states, note we only assume three states in this example */

typedef enum {NEW = 0, RDY, RUN, BLOCKED_ON_MEM, BLOCKED_ON_RECEIVE, IPROC} PROC_STATE_E;  

/**
 * @brief PCB data structure definition.
 * @note  You will need to add your own member variables
 *         in order to finish the project 
 */
typedef struct pcb 
{ 
    struct pcb   *mp_next;  /**> next pcb, not used in this example */  
    U32          *mp_sp;    /**> stack pointer of the process       */
    U32          m_pid;     /**> process id                         */
    PROC_STATE_E m_state;   /**> state of the process               */
    // add memory-ownership list (assume only 1 block per pcb)      
    // struct mem_blk *m_mem_blk;
    BOOL m_has_mem_blk;
    int m_priority;
    MsgQueue msg_queue;
} PCB;

/*
 *==========================================================================
 *                   GLOBAL VARIABLES DECLARATIONS
 *==========================================================================
 */
/* This symbol is defined in the scatter file (see ARM Linker User Guide)*/  
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit; 
extern U32          *gp_stack;            /* see k_memory.c for details   */  
extern PCB          **gp_pcbs;            /* array of pcbs */
extern PCB          *gp_current_process;  /* the current RUN process      */
extern PCB          *gp_pcb_timer_iproc;  /* points to Timer iprocess pcb */ 
extern PCB          *gp_pcb_interrupted;  /* interrupted process's pcb    */
extern PCB          *gp_pcb_uart_iproc;  /* points to UART iprocess pcb */ 
extern PCB          *gp_pcb_kcd_iproc;  /* points to KCD pcb */ 
extern PCB          *gp_pcb_crt_iproc;  /* points to CRT iprocess pcb */ 

extern PROC_INIT    g_proc_table[TOTAL_NUM_PROCS];
                                          /* process initialization table */
extern volatile uint32_t g_timer_count;   /* increment every 1 ms         */

#endif /* ! K_INC_H_ */
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
