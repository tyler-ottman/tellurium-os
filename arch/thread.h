#ifndef THREAD_H
#define THREAD_H

#include <arch/cpu.h>
#include <arch/process.h>

enum thread_state {
    READY       = 0,
    RUNNING     = 1,
    WAITING     = 2,
    BLOCKED     = 3,
    ZOMBIE      = 4
};

struct tcb {
    uint32_t tid;
    int cpu_id;
    struct pcb* parent;
    uint64_t* thread_base_sp;
    uint64_t* thread_sp;
    uint64_t* kernel_base_sp;
    struct context* context;
    uint64_t* fs_base;
    uint32_t state;
    uint32_t time_slice;

    struct tcb* next;
    struct tcb* prev;
};

uint32_t get_new_tid(void);

#endif // THREAD_H