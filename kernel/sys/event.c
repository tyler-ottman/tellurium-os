#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/scheduler.h>
#include <stdbool.h>
#include <sys/event.h>

static bool attach_listener(thread_t *thread, event_t *event) {
    spinlock_acquire(&event->lock);

    bool ret = false;
    for (size_t i = 0; i < LISTEN_CAPACITY; i++) {
        if (!event->listeners[i]) {
            event->listeners[i] = thread;
            ret = true;
            break;
        }
    }

    spinlock_release(&event->lock);

    return ret;
}

static bool remove_listener(thread_t *thread, event_t *event) {
    spinlock_acquire(&event->lock);

    bool ret = false;
    for (size_t i = 0; i < LISTEN_CAPACITY; i++) {
        if (event->listeners[i] == thread) {
            event->listeners[i] = NULL;
            ret = true;
            break;
        }
    }

    spinlock_release(&event->lock);

    return ret;
}

int event_wait(event_t *event) {
    int err = EVENT_OK;
    thread_t *thread = get_core_local_info()->current_thread;

    disable_interrupts();

    attach_listener(thread, event);

    // Block thread until signal
    schedule_thread_block();

    // Verify event that waked up thread matches
    if (thread->received_event != event) {
        err = EVENT_ERR;
    }

    remove_listener(thread, event);

    enable_interrupts();

    return err;
}

void event_signal(event_t *event) {
    spinlock_acquire(&event->lock);

    for (size_t i = 0; i < LISTEN_CAPACITY; i++) {
        thread_t *thread = event->listeners[i];
        if (!thread) {
            continue;
        }

        thread->state = RUNNABLE;
    }

    spinlock_release(&event->lock);
}