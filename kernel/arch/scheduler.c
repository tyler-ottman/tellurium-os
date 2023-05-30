#include <arch/cpu.h>
#include <arch/lock.h>
#include <arch/gdt.h>
#include <arch/scheduler.h>
#include <devices/lapic.h>
#include <libc/doubly_linked_list.h>
#include <libc/kmalloc.h>
#include <stddef.h>

static spinlock_t queue_lock = 0;

static thread_t *head = NULL;
static thread_t *tail = NULL;

typedef struct thread_queue {
    thread_t *head;
    thread_t *tail;
} thread_queue_t;

void schedule_next_thread() {
    thread_t *next_thread = pop_thread_from_queue();

    if (next_thread == NULL) {
        next_thread = get_idle_thread();
    }

    thread_entry(next_thread);
}

// Pop next thread from front of linked list
thread_t *pop_thread_from_queue() {
    spinlock_acquire(&queue_lock);
    if (head == NULL) {
        spinlock_release(&queue_lock);
        return NULL;
    }

    thread_t *thread = head;
    if (head->next == NULL) {
        tail = NULL;
    } else {
        head->next->prev = NULL;
    }

    head = head->next;

    thread->prev = NULL;
    thread->next = NULL;

    spinlock_release(&queue_lock);

    return thread;
}

void schedule_add_thread(thread_t *thread) {
    spinlock_acquire(&queue_lock);

    if (head == NULL) {
        head = thread;
        thread->prev = NULL;
    } else {
        tail->next = thread;
        thread->prev = tail;
    }

    tail = thread;
    thread->next = NULL;

    spinlock_release(&queue_lock);
}

void thread_entry(thread_t *thread) {
    core_t *core = get_core_local_info();
    core->current_thread = thread;
    core->tss.ist1 = (uint64_t)thread->kernel_sp;

    set_thread_local(thread);
    thread->state = THREAD_RUNNING;

    spinlock_release(&thread->yield_lock);

    thread_switch(core);
}

void thread_switch(core_t *core) {
    thread_t *thread = core->current_thread;
    struct pagemap *map = core->current_thread->parent->pmap;

    uint64_t cr3 = (uint64_t)(map->pml4_base) - KERNEL_HHDM_OFFSET;

    core->kernel_stack = thread->kernel_sp;
    core->kernel_scratch = thread->thread_scratch;
    
    lapic_schedule_time(1000);
    lapic_lvt_enable(LVT_TIMER);

    __asm__ volatile(
        "mov %0, %%rsp\n\t"
        "mov %1, %%r15\n\t"
        "pop %%rax\n\t"
        "pop %%rbx\n\t"
        "pop %%rcx\n\t"
        "pop %%rdx\n\t"
        "pop %%rsi\n\t"
        "pop %%rdi\n\t"
        "pop %%rbp\n\t"
        "pop %%r8\n\t"
        "pop %%r9\n\t"
        "pop %%r10\n\t"
        "pop %%r11\n\t"
        "pop %%r12\n\t"
        "pop %%r13\n\t"
        "pop %%r14\n\t"
        "mov %%r15, %%cr3\n\t"
        "pop %%r15\n\t"
        "addq $8, %%rsp\n\t"
        "iretq\n\t" ::
        "r" (&thread->context),
        "r" ((uint64_t *)cr3) :
        "memory"
    );
}

void thread_wrapper(void *entry, void *param) {
    // Jump to actual thread function
    void (*func)(void *) = (void (*)(void *))((uintptr_t)entry);
    (*func)(param);

    // Terminate thread if not already terminated
    schedule_thread_terminate();
}

void schedule_thread_wait(event_t *event) {
    thread_t *thread = get_core_local_info()->current_thread;
    if (thread->state != THREAD_RUNNING) {
        return;
    }

    thread->waiting_for = event;

    schedule_thread_yield(false, YIELD_WAIT);
}

int schedule_notify(event_t *event, thread_t *thread) {
    int state = thread->state;
    if (state != THREAD_WAITING && state != THREAD_BLOCKED) {
        return SCHEDULE_ERR;
    }

    if (state == THREAD_WAITING) {
        thread->received_event = event;
    }
    
    thread->state = THREAD_RUNNABLE;

    schedule_add_thread(thread);

    return SCHEDULE_OK;
}

void schedule_thread_yield(bool no_return, int cause) {
    int state_if_flag = core_get_if_flag();

    // Critical Section
    disable_interrupts();

    // Disable timer interrupts
    lapic_lvt_disable(LVT_TIMER);
    lapic_schedule_time(0);

    core_t *core = get_core_local_info();
    thread_t *thread = core->current_thread;

    thread->yield_cause = cause;

    if (!no_return) {
        spinlock_acquire(&thread->yield_lock);
    }

    // Send self IPI
    lapic_send_ipi(core->lapic_id, core->lapic_ipi_vector);

    enable_interrupts();

    if (no_return) { // Hault here until IPI serviced (for terminated threads)
        core_hlt();
    }

    // Thread waits here until IPI serviced, then context switching back here
    // releases the lock
    spinlock_acquire(&thread->yield_lock);
    spinlock_release(&thread->yield_lock);

    if (!state_if_flag) {
        disable_interrupts();
    }
}

void schedule_thread_terminate() {
    disable_interrupts();

    schedule_thread_yield(true, YIELD_TERMINATE);
}