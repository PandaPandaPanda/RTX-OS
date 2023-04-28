#ifndef KCD_PROC_H_
#define KCD_PROC_H_

void clear_buffer(void);
void* create_msg_for_user(void);
int getCmdMappingIndex(char c);
extern void kcd_proc(void);

#endif
