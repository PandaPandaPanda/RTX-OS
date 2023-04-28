# RTX-OS

## Memory Pool
Memory is divided into fixed size blocks. Request and release user level function can be used for memory management.
````
void *request_memory_block();
````
````
int release_memory_block(void *memory_block);
````

## Process
Processes are managed in time slices, but primitive function is porivided for manual release
````
int release_processor();
````
Process states are managed as follows:

<img src='https://user-images.githubusercontent.com/34975856/235235322-329e72ec-9cdd-473e-addb-0f92e4d239ba.png' width='500'>

### Priority
Process priority convention is similar to UNIX (the smaller the higher).
User can change priority with
````
int set_process_priority(int process_id, int priority);
````

#### Preemption
Upon changing process priority, the ready queue is updated and scheduler is called right after.

#### Ownership
Ownership is given to processes so a process may not release other processes' memory. The onwership could be transfered on occasions like message passing.

### Special Processes
#### Null Process
The process runs as the lowest priority (hidden). Similar to the conventional idle task, it is always ready to run. It can be used for to provide accounting information and also prevents having special cases in the scheduler.

#### Timer i-Process

#### Uart i-Process

## Inter-process Communication

### Delay Send

## System Console I/O
<img src='https://user-images.githubusercontent.com/34975856/235239009-02110619-3f02-466c-a090-efecee592e05.png' width='500'>


