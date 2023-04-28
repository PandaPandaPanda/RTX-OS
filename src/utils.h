#ifndef UTILS_H_
#define UTILS_H_

#include "k_msg.h"
#include "k_memory.h"

void write_num_to_str(int num, char* buffer);
void write_str_to_buffer(char* from, char* to);
int str_to_int(char* buffer);
void convert_seconds_to_timestamp(int total_seconds, char* timestamp);
MSG_BUF* create_wall_clock_msg_print(int total_seconds);

#endif
