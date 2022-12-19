#ifndef THREAD_H
#define THREAD_H

#include <arch/cpu.h>
#include <arch/process.h>

#define IDLE_THREAD_TID     0

enum thread_state {
    CREATED     = 0,
    READY       = 1,
    RUNNING     = 2,
    WAITING     = 3,
    BLOCKED     = 4,
    ZOMBIE      = 5
};

struct tcb {
    uint32_t tid;
    int cpu_id;
    struct pcb* parent;
    uint64_t* thread_base_sp;
    uint64_t* thread_sp;
    uint64_t* kernel_base_sp;
    struct context context;
    uint64_t* fs_base;
    uint32_t state;
    uint32_t time_slice;

    struct tcb* next;
    struct tcb* prev;
};

struct tcb* alloc_idle_thread(void);
struct tcb* create_kernel_thread(void* entry, void* param);
struct tcb* get_idle_thread(void);
uint32_t get_new_tid(void);

#endif // THREAD_H