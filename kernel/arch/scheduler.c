#include <arch/cpu.h>
#include <arch/lock.h>
#include <arch/gdt.h>
#include <arch/scheduler.h>
#include <devices/lapic.h>
#include <libc/doubly_linked_list.h>
#include <libc/kmalloc.h>
#include <stddef.h>

static spinlock_t queue_lock = 0;

static thread_t* head = NULL;
static thread_t* tail = NULL;

typedef struct thread_queue {
    thread_t *head;
    thread_t *tail;
} thread_queue_t;

void schedule_next_thread() {
    thread_t* next_thread = pop_thread_from_queue();

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

void schedule_add_thread(thread_t* thread) {
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

void thread_entry(thread_t* thread) {
    struct core_local_info* cpu_info = get_core_local_info();
    cpu_info->current_thread = thread;
    thread->state = THREAD_RUNNING;
    spinlock_release(&thread->yield_lock);
    set_thread_local(cpu_info->current_thread);
    cpu_info->tss.ist1 = (uint64_t)thread->kernel_sp;

    thread_switch(cpu_info);
}

void thread_switch(struct core_local_info* cpu_info) {
    struct pagemap* map = cpu_info->current_thread->parent->pmap;
    uint64_t* pml4 = map->pml4_base;
    pml4 = (uint64_t*)((uint64_t)pml4 - KERNEL_HHDM_OFFSET);
    // print_context(&cpu_info->current_thread->context);

    cpu_info->kernel_stack = cpu_info->current_thread->kernel_sp;
    cpu_info->kernel_scratch = cpu_info->current_thread->thread_scratch;

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
        "r" (&cpu_info->current_thread->context),
        "r" (pml4) :
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
    thread->state = THREAD_WAITING;

    schedule_thread_yield(false);
}

void schedule_notify(event_t *event, thread_t *thread) {
    int state = thread->state;
    if (state != THREAD_WAITING && state != THREAD_BLOCKED) {
        return;
    }

    if (state == THREAD_WAITING) {
        thread->received_event = event;
    }
    
    thread->state = THREAD_RUNNABLE;

    schedule_add_thread(thread);
}

void schedule_thread_yield(bool no_return) {
    int state_if_flag = core_get_if_flag();

    // Critical Section
    disable_interrupts();

    // Disable timer interrupts
    lapic_lvt_disable(LVT_TIMER);
    lapic_schedule_time(0);

    struct core_local_info *info = get_core_local_info();
    thread_t *thread = info->current_thread;

    if (!no_return) {
        spinlock_acquire(&thread->yield_lock);
    }

    // Send self IPI
    lapic_send_ipi(info->lapic_id, info->lapic_ipi_vector);

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
    thread->state = THREAD_ZOMBIE;

    schedule_thread_yield(true);
}