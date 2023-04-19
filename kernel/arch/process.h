#ifndef PROCESS_H
#define PROCESS_H

#include <arch/cpu.h>
#include <libc/vector.h>
#include <stdbool.h>
#include <stdint.h>

#define IDLE_THREAD_TID         0
#define PROCESS_MAX             256

typedef int pstate_t;
typedef int pid_t;

enum process_state {
    ALIVE =     0,
    DEAD =      1
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

typedef struct pcb {
    pid_t pid;
    pstate_t state;
    VECTOR_DECLARE(threads);
    VECTOR_DECLARE(children);
    struct pagemap* pmap;
} pcb_t;

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
    bool isKernel;

    struct tcb *prev;
    struct tcb *next;
} thread_t;

struct pcb* get_kernel_process(void);
struct pcb *create_user_process(const char *elf_path);
void process_destroy(pcb_t *proc);
int process_alloc_pid(void);
void process_free_pid(uint16_t pid);
void init_kernel_process(void);

thread_t* alloc_idle_thread(void);
thread_t* create_kernel_thread(void* entry, void* param);
thread_t* get_idle_thread(void);
uint32_t get_new_tid(void);
void thread_destroy(thread_t *thread);

#endif // PROCESS_H