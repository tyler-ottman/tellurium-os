#ifndef PROCESS_H
#define PROCESS_H

#include <arch/cpu.h>
#include <libc/vector.h>
#include <stdint.h>

#define KERNEL_PROCESS_ID       0
#define IDLE_THREAD_TID     0

typedef int pstate_t;
typedef int pid_t;

enum process_state {
    alive =     0,
    dead =      1
};

enum thread_state {
    CREATED,
    RUNNABLE,
    BLOCKED,
    WAITING,
    DELAYED,
    RUNNING,
    ZOMBIE
};

struct pcb {
    pid_t pid;
    pstate_t state;
    VECTOR_DECLARE(threads);
    VECTOR_DECLARE(children);
    struct pagemap* pmap;
};

typedef struct tcb {
    uint32_t tid;
    int cpu_id;
    struct pcb* parent;
    uint64_t* thread_base_sp;
    uint64_t* thread_sp;
    uint64_t* kernel_base_sp;
    uint64_t* kernel_sp;
    void (*thread_entry)(void*);
    struct ctx context;
    uint64_t* fs_base;
    uint32_t state;
    uint32_t time_slice;

    struct tcb* next;
    struct tcb* prev;
} thread_t;

struct pcb* get_kernel_process(void);
void init_kernel_process(void);

thread_t* alloc_idle_thread(void);
thread_t* create_kernel_thread(void* entry, void* param);
thread_t* get_idle_thread(void);
uint32_t get_new_tid(void);

#endif // PROCESS_H