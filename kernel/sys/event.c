#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/terminal.h>
#include <arch/scheduler.h>
#include <stdbool.h>
#include <sys/event.h>

static bool attach_listener(thread_t *thread, event_t *event) {
    spinlock_acquire(&event->lock);

    bool ret = false;
    for (size_t i = 0; i < LISTEN_CAPACITY; i++) {
        if (!event->listeners[i]) {
            event->listeners[i] = thread;
            event->num_listeners++;
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
            event->num_listeners--;
            ret = true;
            break;
        }
    }

    spinlock_release(&event->lock);

    return ret;
}

int event_wait(event_t *event) {
    int err = EVENT_OK;

    disable_interrupts();

    thread_t *thread = get_thread_local();

    attach_listener(thread, event);

    // Thread waits until event received
    schedule_thread_wait();

    if (thread->received_event != event) {
        err = EVENT_ERR;
    }

    remove_listener(thread, event);

    enable_interrupts();
        
    return err;
}

int event_signal(event_t *event) {
    disable_interrupts();

    spinlock_acquire(&event->lock);

    if (event->num_listeners == 0) {
        spinlock_release(&event->lock);
        enable_interrupts();
        return EVENT_NO_LISTENERS;
    }

    for (size_t i = 0; i < LISTEN_CAPACITY; i++) {
        thread_t *thread = event->listeners[i];
        if (thread) {
            int err = schedule_notify(event, thread);
            
            // Trying to notify a thread that is not yet blocked
            if (err) {
                spinlock_release(&event->lock);
                enable_interrupts();
                return EVENT_BAD_THREAD_STATE;
            }
        }
    }

    spinlock_release(&event->lock);

    enable_interrupts();

    return EVENT_OK;
}