#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <arch/process.h>

void schedule_next_thread(void);
void schedule_idle_thread(void);
thread_t* pop_thread_from_queue(void);
void add_thread_to_queue(thread_t* thread);
void thread_entry(thread_t* thread);

#endif // SCHEDULER_H
