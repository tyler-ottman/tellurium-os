#ifndef PROCESS_H
#define PROCESS_H

#include <libc/vector.h>
#include <stdint.h>

#define KERNEL_PROCESS_ID       0

typedef int pstate_t;
typedef int pid_t;

enum process_state {
    alive =     0,
    dead =      1
};

struct pcb {
    pid_t pid;
    pstate_t state;
    VECTOR_DECLARE(threads);
    VECTOR_DECLARE(children);
    struct pagemap* pmap;
};

struct pcb* get_kernel_process(void);
void init_kernel_process(void);

#endif // PROCESS_H