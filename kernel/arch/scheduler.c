#include <arch/cpu.h>
#include <arch/lock.h>
#include <arch/gdt.h>
#include <arch/scheduler.h>
#include <devices/lapic.h>
#include <libc/doubly_linked_list.h>
#include <libc/kmalloc.h>
#include <stddef.h>

#define QUEUE_MAX                           100

spinlock_t joined_queue_lock = 0;
spinlock_t ready_queue_lock = 0;

// Threads waiting for other threads to terminate
static thread_t *joined_threads[QUEUE_MAX] = {0};

// Scheduler's ready threads linked list
static thread_t *head = NULL;
static thread_t *tail = NULL;

typedef struct thread_queue {
    thread_t *head;
    thread_t *tail;
} thread_queue_t;

void schedule_next_thread() {
    thread_t *next_thread = pop_thread_from_queue();

    if (!next_thread) {
        next_thread = get_core_local_info()->idle_thread;
    }

    thread_entry(next_thread);
}

// Pop next thread from front of linked list
thread_t *pop_thread_from_queue() {
    spinlock_acquire(&ready_queue_lock);
    if (head == NULL) {
        spinlock_release(&ready_queue_lock);
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

    spinlock_release(&ready_queue_lock);

    return thread;
}

void schedule_add_thread(thread_t *thread) {
    bool i_flag = core_get_if_flag();
    if (i_flag) {
        disable_interrupts();
    }

    spinlock_acquire(&ready_queue_lock);

    if (head == NULL) {
        head = thread;
        thread->prev = NULL;
    } else {
        tail->next = thread;
        thread->prev = tail;
    }

    tail = thread;
    thread->next = NULL;

    spinlock_release(&ready_queue_lock);

    if (i_flag) {
        enable_interrupts();
    }
}

void thread_entry(thread_t *thread) {
    core_t *core = get_core_local_info();
    
    core->kernel_stack = thread->kernel_sp;
    core->kernel_scratch = thread->thread_scratch;
    core->tss.ist1 = (uint64_t)core->irq_stack;
    core->current_thread = thread;

    thread->state = THREAD_RUNNING;

    spinlock_release(&thread->yield_lock);

    thread_switch();
}

void thread_switch() {
    thread_t *thread = get_thread_local();
    struct pagemap *map = thread->parent->pmap;

    uint64_t cr3 = (uint64_t)(map->pml4_base) - KERNEL_HHDM_OFFSET;

    lapic_schedule_time(thread->quantum);
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

    __builtin_unreachable();
}

void thread_wrapper(void *entry, void *param) {
    // Jump to actual thread function
    void (*func)(void *) = (void (*)(void *))((uintptr_t)entry);
    (*func)(param);

    // Terminate thread if not already terminated
    schedule_thread_terminate();
}

void schedule_thread_wait() {
    thread_t *thread = get_thread_local();
    if (thread->state != THREAD_RUNNING) {
        return;
    }

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

    thread_t *thread = get_core_local_info()->current_thread;

    spinlock_acquire(&joined_queue_lock);

    // Wake up joined threads waiting for thread to terminate 
    for (size_t i = 0; i < QUEUE_MAX; i++) {
        thread_t *joined_thread = joined_threads[i];
        if (joined_thread && joined_thread->join_thread == thread) {
            joined_thread->state = THREAD_RUNNABLE;

            schedule_add_thread(joined_thread);
            
            joined_threads[i] = NULL;
        }
    }

    spinlock_release(&joined_queue_lock);

    schedule_thread_yield(true, YIELD_TERMINATE);
}

void schedule_thread_join(thread_t *thread_to_wait_on) {
    int i_flag = core_get_if_flag();
    if (i_flag) {
        disable_interrupts();
    }

    thread_t *thread = get_core_local_info()->current_thread;
    
    thread->join_thread = thread_to_wait_on;
    
    spinlock_acquire(&joined_queue_lock);

    if (thread_to_wait_on->state == THREAD_ZOMBIE) {
        spinlock_release(&joined_queue_lock);
        if (i_flag) {
            enable_interrupts();
        }
        return;
    }

    for (size_t i = 0; i < QUEUE_MAX; i++) {
        if (!joined_threads[i]) {
            joined_threads[i] = thread;
            break;
        }
    }

    spinlock_release(&joined_queue_lock);

    schedule_thread_yield(false, YIELD_JOIN);

    if (i_flag) {
        enable_interrupts();
    }    
}