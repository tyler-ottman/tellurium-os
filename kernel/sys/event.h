#ifndef EVENT_H
#define EVENT_H

#include <arch/lock.h>
#include <arch/process.h>

#define LISTEN_CAPACITY                     0x10

#define EVENT_OK                            0
#define EVENT_ERR                           1

typedef struct event {
    spinlock_t lock;
    struct thread *listeners[LISTEN_CAPACITY];
} event_t;

int event_wait(event_t *event);
void event_signal(event_t *event);

#endif // EVENT_H