#include <arch/lock.h>
#include <arch/gdt.h>
#include <arch/scheduler.h>
#include <devices/lapic.h>
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
        next_thread = get_idle_thread();
    }

    thread_entry(next_thread);
}

struct tcb* pop_thread_from_queue() {
    spinlock_acquire(&queue_lock);

    if (is_queue_empty()) {
        spinlock_release(&queue_lock);
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

void thread_entry(struct tcb* thread) {
    struct core_local_info* cpu_info = get_core_local_info();
    
    cpu_info->current_thread = thread;
    set_thread_local(cpu_info->current_thread);
    cpu_info->tss.ist1 = (uint64_t)thread->kernel_base_sp;

    lapic_write(LVT_INITIAL_COUNT, 0x30000000);
    enable_interrupts();

    struct pagemap* map = cpu_info->current_thread->parent->pmap;
    uint64_t* pml4 = map->pml4_base;
    pml4 = (uint64_t*)((uint64_t)pml4 - KERNEL_HHDM_OFFSET);
    
    __asm__ volatile(
        "mov %0, %%rsp\n\t"
        "mov %1, %%rax\n\t"
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
        "pop %%r15\n\t"
        "mov %%rax, %%cr3\n\t"
        "pop %%rax\n\t"
        "addq $8, %%rsp\n\t"
        "iretq\n\t" ::
        "r" (&cpu_info->current_thread->context),
        "r" (pml4)
    );
}