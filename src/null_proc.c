#include "rtx.h"
#include "null_proc.h"

void null_process(void) {
    while(1) {
        k_release_processor();
    }
}
