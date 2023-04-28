#include "wall_clock.h"
#include "rtx.h"
#include "k_msg.h"
#include "k_memory.h"
#include "utils.h"

int wall_clock_running = 0;

void wall_clock() {
    // register command with KCD
    MSG_BUF *p;
    p = (MSG_BUF*)request_memory_block();
    p->mtype = KCD_REG;
    p->mtext[0] = '%';
    p->mtext[1] = 'W';
    p->mtext[2] = '\0';
    uart1_put_string("wallclock: registering %%W command\r\n");
    send_message(PID_KCD, p);
    
    MSG_BUF* recv_blk;
    int sender_pid;

    int current_time = 0;
    int waiting = 0;
    while (1) {
        recv_blk = (MSG_BUF*)receive_message(&sender_pid);
        if (recv_blk != NULL) { 
            if (recv_blk->mtype == KCD_CMD) {
                uart1_put_string("wallclock: command received\r\n");
                char* buff = recv_blk->mtext;
                buff += 2;
                MSG_BUF *p;
                switch (buff[0]) {
                    case 'R': // Reset wall clock
                        current_time = 0;
                        wall_clock_running = 1;

                        p = create_wall_clock_msg_print(current_time);
                        send_message(PID_CRT, p);
                        recv_blk->mtype = DEFAULT;
                        if (0 == waiting) {
                            delayed_send(PID_CLOCK, recv_blk, 1000);
                            waiting = 1;
                        }
                        current_time++;
                        break;
                    case 'S': // Set wall clock
                        if (buff[1] != ' ' || buff[4] != ':' || buff[7] != ':' ) {
                            uart1_put_string("Invalid command input\r\n");
                            break;
                        } 

                        buff += 2;
                        int hrs = (int)(buff[0] - '0') * 10 + (int)(buff[1] - '0');
                        buff += 3;
                        int mins = (int)(buff[0] - '0') * 10 + (int)(buff[1] - '0');
                        buff += 3;
                        int secs = (int)(buff[0] - '0') * 10 + (int)(buff[1] - '0');

                        if (hrs < 0 || hrs > 23 || mins < 0 || mins > 59 || secs < 0 || secs > 59) {
                            uart1_put_string("Invalid command input\r\n");
                            break;
                        }

                        secs += mins * 60 + hrs * 3600;
                        current_time = secs;
                        wall_clock_running = 1;

                        p = create_wall_clock_msg_print(current_time);
                        send_message(PID_CRT, p);
                        recv_blk->mtype = DEFAULT;
                        if (0 == waiting) {
                            delayed_send(PID_CLOCK, recv_blk, 1000);
                            waiting = 1;
                        }
                        break;
                    case 'T': // Terminate wall clock
                        wall_clock_running = 0;
                        waiting = 0;
                        break;
                    default:
                        break;
                }
            } else if (recv_blk->mtype == DEFAULT && wall_clock_running == 1) {
                MSG_BUF *p;
                p = create_wall_clock_msg_print(current_time);
                send_message(PID_CRT, p);
                delayed_send(PID_CLOCK, recv_blk, 1000);
                current_time++;
            }
        }

        /* Previous implementation that interfaces timer interrupt process
         * that performs privelege level actions, but theoretically more time-accurate 
         */
        // recv_blk =  (MSG_BUF*)receive_message(&sender_pid);
        // if (recv_blk != NULL) { 
        //     uart1_put_string("wallclock: command received\r\n");
        //     if (recv_blk->mtype == KCD_CMD) {
        //         uart1_put_string("wallclock: command actually received\r\n");
        //         char* buff = recv_blk->mtext;
        //         buff += 2;
        //         printf("current command is: %c\n", buff[0]);
        //         switch (buff[0]) {
        //             case 'R':
        //                 uart1_put_string("wallclock: reset\r\n");
        //                 wall_clock_running = 1;
        //                 g_reference_time = g_timer_count;
        //                 g_offset_time = 0;
        //                 break;
        //             case 'S':
        //                 uart1_put_string("wallclock: set\r\n");

        //                 wall_clock_running = 1;
        //                 buff += 3;
        //                 int hrs = (int)(buff[0] - '0') * 10 + (int)(buff[1] - '0');
        //                 buff += 3;
        //                 int mins = (int)(buff[0] - '0') * 10 + (int)(buff[1] - '0');
        //                 buff += 3;
        //                 int secs = (int)(buff[0] - '0') * 10 + (int)(buff[1] - '0');

        //                 secs += mins * 60 + hrs * 3600;
        //                 g_reference_time = g_timer_count;
        //                 g_offset_time = g_timer_count + secs *1000;
        //                 break;
        //             case 'T':
        //                 uart1_put_string("wallclock: terminate\r\n");

        //                 // send a message to timer iproc telling it to halt
        //                 wall_clock_running = 0;
        //                 break;
        //             default:
        //                 break;
        //         }
        //     }
        // }

    }

}
