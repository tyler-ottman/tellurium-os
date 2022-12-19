#include <arch/lock.h>
#include <arch/gdt.h>
#include <arch/scheduler.h>
#include <libc/kmalloc.h>
#include <stddef.h>

static spinlock_t queue_lock = 0;

static struct tcb* head = NULL;
static struct tcb* tail = NULL;

static bool is_queue_empty() {
    return head == NULL;
}

void schedule_next_thread() {
    struct tcb* next_thread = pop_thread_from_queue();
    if (next_thread == NULL) {
        kprintf("No threads to schedule\n");
        next_thread = get_idle_thread();
    }

    struct core_local_info* cpu_info = get_core_local_info();
    set_thread_local(cpu_info->current_thread);
    

    kprintf(GREEN "Scheduling thread\n");
}

struct tcb* pop_thread_from_queue() {
    spinlock_acquire(&queue_lock);
    
    if (is_queue_empty()) {
        return NULL;
    }

    struct tcb* thread = head;
    if (thread->next == NULL) {
        tail = NULL;
    } else {
        thread->next->prev = NULL;
    }

    head = head->next;
    thread->next = NULL;

    spinlock_release(&queue_lock);

    return thread;
}

void add_thread_to_queue(struct tcb* thread) {
    spinlock_acquire(&queue_lock);

    if (is_queue_empty()) {
        tail = thread;
    } else {
        tail->next = thread;
        thread->prev = tail;
    }

    tail = thread;
    thread->next = NULL;

    spinlock_release(&queue_lock);
}