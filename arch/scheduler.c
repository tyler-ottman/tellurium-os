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

void create_kernel_thread(void* entry, void* param) {
    ASSERT(entry != NULL);

    struct tcb* thread = kmalloc(sizeof(struct tcb));
    ASSERT(thread != NULL);

    thread->tid = get_new_tid();

    thread->cpu_id = -1;

    thread->parent = get_kernel_process();

    size_t stack_size = PAGE_SIZE_BYTES;
    thread->thread_base_sp = kmalloc(PAGE_SIZE_BYTES);
    ASSERT(thread->thread_base_sp != NULL);
    thread->thread_sp = thread->thread_base_sp + stack_size;
    
    thread->kernel_base_sp = NULL;

    thread->context = kmalloc(sizeof(struct context));
    ASSERT(thread->context != NULL);
    
    __memset(thread->context, 0, sizeof(struct context));
    struct context* context = thread->context;
    context->ds = GDT_KERNEL_DATA;
    context->rdi = (uint64_t)param;
    context->rip = (uint64_t)entry;
    context->cs = GDT_KERNEL_CODE;
    context->rflags = RFLAGS_RESERVED_MASK | RFLAGS_INTERRUPT_MASK;
    context->rsp = (uint64_t)thread->thread_sp;
    context->ss = GDT_KERNEL_DATA;

    thread->state = READY;

    thread->next = NULL;
    thread->prev = NULL;

    add_thread_to_queue(thread);
}

void schedule_next_thread() {
    struct tcb* next_thread = pop_thread_from_queue();
    if (next_thread == NULL) {
        schedule_idle_thread();
    }

    
}

void schedule_idle_thread() {

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