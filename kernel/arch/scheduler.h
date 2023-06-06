#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <arch/process.h>
#include <stdbool.h>
#include <sys/event.h>

#define SCHEDULE_OK                         0
#define SCHEDULE_ERR                        1

// Cause for IPI
#define YIELD_WAIT                          1
#define YIELD_TERMINATE                     2
#define YIELD_JOIN                          3

void schedule_next_thread(void);
thread_t *pop_thread_from_queue(void);
void schedule_add_thread(thread_t *thread);
void thread_entry(thread_t *thread);
void thread_switch(core_t *core);
void thread_wrapper(void *entry, void *param);

void schedule_thread_wait(void);
int schedule_notify(event_t *event, thread_t *thread);
void schedule_thread_yield(bool no_return, int cause);
void schedule_thread_terminate(void);
void schedule_thread_join(thread_t *thread);

#endif // SCHEDULER_H
