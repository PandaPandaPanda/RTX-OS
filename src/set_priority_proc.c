#include "set_priority_proc.h"
#include "rtx.h"
#include "k_msg.h"
#include "k_memory.h"
#include "k_process.h"

void set_priority_proc() {
    // register command with KCD
    MSG_BUF *p;
    p = (MSG_BUF*)request_memory_block();
    p->mtype = KCD_REG;
    p->mtext[0] = '%';
    p->mtext[1] = 'C';
    p->mtext[2] = '\0';
    uart1_put_string("set priority command process: registering %%C command\r\n");
    send_message(PID_KCD, p);

    MSG_BUF* recv_blk;
    int sender_pid;

    while (1) {
        recv_blk = (MSG_BUF*)receive_message(&sender_pid);
        if (recv_blk != NULL && recv_blk->mtype == KCD_CMD) { // %C process_id new_priority
            uart1_put_string("set priority command process: command received\r\n");
            char* buff = recv_blk->mtext;
            buff += 3; // skip space
            int pid = 0;
            while (1) {
                if (buff[0] == ' ') {
                    buff++;
                    break;
                }
                int digit = (int)(buff[0] - '0');
                if (digit >= 0 && digit <= 9) {
                    pid = pid * 10 + digit;
                    buff++;
                } else {
                    pid = -1; // invalid state
                    break;
                }
            }
            printf("pid: %d\r\n", pid);
            if (pid < 0 || pid > PID_UART_IPROC) {
                uart1_put_string("Invalid command input\r\n");
            } else {
                int priority = 0;
                while (1) {
                    if (buff[0] == '\r' || buff[0] == '\n' || buff[0] == '\0') {
                        break;
                    }
                    int digit = (int)(buff[0] - '0');
                    if (digit >= 0 && digit <= 9) {
                        priority = priority * 10 + digit;
                        buff++;
                    } else {
                        priority = -1; // invalid state
                        break;
                    }
                }
                printf("priority: %d\r\n", priority);
                if (priority < 0 || priority >= PRI_NULL) {
                    uart1_put_string("Invalid command input\r\n");
                } else if (set_process_priority(pid, priority) == RTX_ERR) {
                    uart1_put_string("Invalid command input\r\n");
                }
            }
        }
    }
}
