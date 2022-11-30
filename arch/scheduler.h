#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/terminal.h>
#include <memory/vmm.h>

#define MAX_FILE_NAME       256

struct pcb {
    struct pagemap* pmap;
    int pid;
    int state;
    uint64_t* ksp;
    char filename[MAX_FILE_NAME];
} __attribute__((packed));



#endif // SCHEDULER_H