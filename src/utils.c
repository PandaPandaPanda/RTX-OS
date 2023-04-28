#include "utils.h"

void write_num_to_str(int num, char* buffer) {
    int i = 0;

    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    while (num != 0) {
        int rem = num % 10;
        buffer[i++] = rem + '0';
        num = num / 10;
    }

    buffer[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = *(buffer + start);
        *(buffer + start) = *(buffer + end);
        *(buffer + end) = temp;
        start++;
        end--;
    }
}

int str_to_int(char* buffer) {
    int ans = 0;
    while (*buffer != '\0') {
        ans *= 10;
        ans += *buffer - '0';
        buffer++;
    }
    return ans;
}

void write_str_to_buffer(char* from, char* to) {
    while(*from != '\0'){
        *to = *from;
        to++;
        from++;
    }
    *to = '\0';
}

MSG_BUF* create_wall_clock_msg_print(int total_seconds) {
    int hrs = (total_seconds / 3600) % 24; // ensure 24 hours time 
    int mins = (total_seconds % 3600) / 60;
    int secs = ((total_seconds % 3600) % 60);

    MSG_BUF *p = (MSG_BUF *) k_request_memory_block();
    p->mtype = CRT_DISPLAY;
    p->mtext[0] = hrs /10 +'0';
    p->mtext[1] = hrs % 10 +'0';
    p->mtext[2] = ':';
    p->mtext[3] = mins /10 +'0';
    p->mtext[4] = mins % 10 +'0';
    p->mtext[5] = ':';
    p->mtext[6] = secs /10 +'0';
    p->mtext[7] = secs % 10 +'0';
    p->mtext[8] = '\r';
    p->mtext[9] = '\n';
    p->mtext[10] = '\0';
    return p;
}
