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
 * @file        k_msg.c
 * @brief       kernel message passing source code template
 *              
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 *****************************************************************************/
#include "k_msg.h"
#include "k_rtx.h"
#include "k_process.h"

extern PriorityQueue ready_queue;
extern PriorityQueue blocked_on_receive_queue;
/*
 *==========================================================================
 *                            TO BE IMPLEMENTED
 *==========================================================================
 */
int k_send_message(int pid, void *p_msg)
{   
    __disable_irq();
    // cast p_msg into a message buffer, setting the sender and receiver pid
    MSG_BUF *env = (MSG_BUF* ) p_msg;
    if (gp_current_process->m_pid != PID_TIMER_IPROC) {
        env->m_send_pid = gp_current_process->m_pid;
    }
    if (pid != PID_TIMER_IPROC) {
        env->m_recv_pid = pid;
        mem_blk *env_mem_blk = (mem_blk *)((U32 *)env - 2);
        env_mem_blk->m_pid = pid;
    }

    env->mp_next = NULL;
    // find the pcb by pcb id
    PCB* recv_pcb = get_pcb_by_pid(pid);
    enqueue_msg (&(recv_pcb->msg_queue), env);
    // if the receiver process is blocked on receive, we set the state to ready
    // enqueue the process onto the ready queue
    if (recv_pcb->m_state == BLOCKED_ON_RECEIVE) {
        recv_pcb->m_state = RDY;
        priorityRemove(&blocked_on_receive_queue, recv_pcb->m_pid);
        priorityEnqueue(&ready_queue, recv_pcb);
        // prevents nested interrupts, we only switch process until IPROC finishes
        // so previous process can run
        if (gp_current_process->m_pid != PID_UART_IPROC) {
            k_release_processor();
        }
    }
    __enable_irq();
    return RTX_OK;
}

int k_delayed_send(int pid, void *p_msg, int delay)
{
    if (p_msg == NULL || delay < 0) {
        return RTX_ERR;
    }

    if (pid < 0 || pid > PID_CRT) {
        return RTX_ERR;
    }

    __disable_irq();
    MSG_BUF *env = (MSG_BUF* ) p_msg;
    env->m_send_pid = gp_current_process->m_pid;
    env->m_recv_pid = pid;
    env->m_timer_pid = PID_TIMER_IPROC;
    env->m_clock_ticks = delay;
    __enable_irq();
    
    // send message to i-proc
    k_send_message(PID_TIMER_IPROC, env);//p_msg);
    return RTX_OK;
}

void *k_receive_message(int *p_pid)
{
    __disable_irq();

    while(gp_current_process->msg_queue.numBlocks == 0){
        if (gp_current_process->m_state != BLOCKED_ON_RECEIVE) {
            priorityEnqueue(&blocked_on_receive_queue, gp_current_process);
            gp_current_process->m_state = BLOCKED_ON_RECEIVE;
        }
        k_release_processor();
        // process switch ends, comes back to this line
    }
    
    MSG_BUF* mb =  dequeue_msg(&(gp_current_process->msg_queue));

    if (p_pid) {
        *p_pid = mb->m_send_pid;
    }

    __enable_irq();
    return mb;
}

// TODO: non blocking receive
void *k_receive_message_nb(int *p_pid) {
    // only called by i-process
    // returns null if the invoking process is an i-process and there is no message waiting
    __disable_irq();

    if (gp_current_process->msg_queue.numBlocks == 0){
        __enable_irq();
        return NULL;
    }
    
    MSG_BUF* mb =  dequeue_msg(&(gp_current_process->msg_queue));

    if (p_pid) {
        *p_pid = mb->m_send_pid;
    }

    __enable_irq();
    return mb;
}
 
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
