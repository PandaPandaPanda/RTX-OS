#include "rtx.h"
#include "uart_polling.h"
#include "kcd_proc.h"
#include "uart_irq.h"
#include "wall_clock.h"
// each idx maps to a character, for example index 0 maps to 'A', and what stores in the array is a pid of the sender process.
int command_map[52]; 
// internal buffer of KCD, used to queue up the message until receiving the enter key
char recv_buffer[128]; 
extern uint8_t g_send_char;
extern int wall_clock_running;
int buffer_idx = 0;

void clear_buffer(void) {
    buffer_idx = 0;
    for(int i = 0; i < 128; i++) {
        recv_buffer[i] = '\0';
    }
}


void* create_msg_for_user(void) {
    void    *p_blk;
    MSG_BUF *p_msg;
    char    *ptr;
    p_blk = request_memory_block();
    p_msg = p_blk;
    p_msg->mtype = KCD_CMD;
    int i = 0;
    ptr = p_msg->mtext;
    while (recv_buffer[i] != '\0') {
        *ptr++ = recv_buffer[i];
        i++;
    }
    *ptr = '\0'; 
    return p_blk;
}

/* pre: char c is a valid ascii character */
int getCmdMappingIndex(char c) {
    int cNum = (int)c;
    if (cNum >= 65 && cNum <= 90) {
        return cNum % 65;
    } else if (cNum >= 97 && cNum <= 122) {
        return cNum % 97 + 26; // second half of the command table
    } else {
        return RTX_ERR;
    }
}

void kcd_proc(void) {
    int sender_pid;
    for(int i = 0; i < 52; i++) {
        command_map[i] = -1;
    }
    clear_buffer();

    while (1) {
        // receives message from kcd or user process
        MSG_BUF* recv_blk =  (MSG_BUF*)receive_message(&sender_pid);

        // test if msg is of the right type, if not free mem
        if (recv_blk != NULL) { 
            // wall_clock_running = 0;
            if (recv_blk->mtype == KEY_IN) { // KEYBOARD INPUT
                // send to Crt iproc
                recv_buffer[buffer_idx] = recv_blk -> mtext[0];
                buffer_idx++;
                g_send_char = 0;
                recv_blk -> mtype = CRT_DISPLAY;
                send_message(PID_CRT, recv_blk);
                if (recv_blk -> mtext[0] == '\r') {
                    recv_buffer[buffer_idx] = '\0';
                    // check if it is a command, and the command is registered, then send to the process
                    if (recv_buffer[0] == '%' && buffer_idx <= 64) {
#ifdef DEBUG_0
                        printf("consuming command for user process %c\r\n", getCmdMappingIndex(recv_buffer[1]));
#endif
                        if (command_map[getCmdMappingIndex(recv_buffer[1])] != -1) {
                            int user_proc_pid = command_map[getCmdMappingIndex(recv_buffer[1])];
                            void* env = create_msg_for_user();
                            send_message(user_proc_pid, env);
                        } else {
                            // command not found
                            uart1_put_string("Command not found\r\n");
                        }
                    } else {
                        // input is not a command
                        uart1_put_string("Invalid command input\r\n");
                    }
                    clear_buffer();
                }
            } else if (recv_blk -> mtype == KCD_REG) { // COMMAND REGISTRATION
                // register a user command
#ifdef DEBUG_0
                printf("registering command for user process %d\r\n", sender_pid);
#endif
                char cmd = recv_blk -> mtext[1];
                command_map[getCmdMappingIndex(cmd)] = sender_pid;
                release_memory_block(recv_blk);
            } else {
                release_memory_block(recv_blk);
            }
        }
    }
}
