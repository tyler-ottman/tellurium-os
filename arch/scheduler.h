#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <arch/process.h>

void schedule_next_thread(void);
void schedule_yield(void);
thread_t* pop_thread_from_queue(void);
void add_thread_to_queue(thread_t* thread);
void thread_entry(thread_t* thread);
void thread_switch(struct core_local_info* cpu_info);
void thread_init_entry(struct core_local_info* cpu_info);

#endif // SCHEDULER_H
