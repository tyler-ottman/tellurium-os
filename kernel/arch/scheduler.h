#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <arch/process.h>

void schedule_next_thread(void);
thread_t* pop_thread_from_queue(void);
void schedule_add_thread(thread_t* thread);
void thread_entry(thread_t* thread);
void thread_switch(struct core_local_info* cpu_info);
void thread_wrapper(void *entry, void *param);

void schedule_thread_block(void);
void schedule_thread_yield(void);
void schedule_thread_terminate(void);

#endif // SCHEDULER_H
