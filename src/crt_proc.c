#include "rtx.h"
#include "crt_proc.h"
#include "k_memory.h"
#include "k_process.h"

/*
* Privileged process
* responds to CRT_DISPLAY msgs
* forwards the msgs to the uart i process
*/ 
extern uint8_t g_tx_irq; 
extern uint8_t g_send_char; 
void crt_proc(void) {
    // __disable_irq();
    while (1) {
        int sender_pid;
        // receives message from kcd or user process
        MSG_BUF* recv_blk =  (MSG_BUF*)receive_message(&sender_pid);

        // test if msg is of the right type, if not free mem
        while (recv_blk == NULL || recv_blk->mtype != CRT_DISPLAY) {
            if (recv_blk != NULL) {
                release_memory_block(recv_blk);
            }
            recv_blk = (MSG_BUF*)receive_message(&sender_pid);
        }

        // send to uart iproc
        LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *) LPC_UART0; 
        // char output_char = recv_blk -> mtext[0];
        if (recv_blk -> mtext[0] == '\r') {
            recv_blk -> mtext[1] = '\n';
            recv_blk -> mtext[2] = '\0';
        }
        pUart->THR = recv_blk -> mtext[0];
        send_message(PID_UART_IPROC, recv_blk);
        g_tx_irq = 1; 
        g_send_char = 0;

        // set the interrupt 
        pUart->IER |= IER_THRE;
    }
    // __enable_irq();
}
