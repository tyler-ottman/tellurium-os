#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <arch/thread.h>
#include <arch/process.h>

void create_kernel_thread(void* entry, void* param);
void schedule_next_thread(void);
void schedule_idle_thread(void);
struct tcb* pop_thread_from_queue(void);
void add_thread_to_queue(struct tcb* thread);

#endif // SCHEDULER_H
