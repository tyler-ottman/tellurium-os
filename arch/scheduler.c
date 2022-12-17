#include <arch/scheduler.h>
#include <libc/kmalloc.h>
#include <stddef.h>

struct tcb* create_kernel_thread(void* entry, void* param) {
    if (entry == NULL) {
        return NULL;
    }

    struct tcb* thread = kmalloc(sizeof(struct tcb));
    if (thread == NULL) {
        return NULL;
    }

    thread->tid = get_new_tid();
    thread->cpu_id = -1;

    thread->thread_sp = kmalloc(PAGE_SIZE_BYTES);
    if (thread->thread_sp == NULL) {
        return NULL;
    }
    
    thread->context = kmalloc(sizeof(struct context));
    if (thread->context == NULL) {
        return NULL;
    }

    thread->state = READY;

    return thread;
}
