#include <arch/gdt.h>
#include <arch/lock.h>
#include <arch/thread.h>
#include <libc/kmalloc.h>
#include <libc/vector.h>

static spinlock_t tid_lock = 0;
static uint32_t cur_tid = 1;

static void idle_thread_spin(void) {
    kprintf("Idle Thread reached\n");
    for(;;) {}
}

struct tcb* alloc_idle_thread(void) {
    void* entry = (void*)((uint64_t)idle_thread_spin);
    return create_kernel_thread(entry, NULL);
}

struct tcb* get_idle_thread(void) {
    struct core_local_info* cpu_info = get_core_local_info();
    return cpu_info->idle_thread;
}

struct tcb* create_kernel_thread(void* entry, void* param) {
    ASSERT(entry != NULL);

    struct tcb* thread = kmalloc(sizeof(struct tcb));
    ASSERT(thread != NULL);

    thread->tid = get_new_tid();

    thread->cpu_id = -1;

    thread->parent = get_kernel_process();

    uint64_t stack_top;
    size_t stack_size = PAGE_SIZE_BYTES;
    thread->thread_base_sp = kmalloc(stack_size);
    ASSERT(thread->thread_base_sp != NULL);
    stack_top = (uint64_t)thread->thread_base_sp + stack_size;
    thread->thread_sp = (uint64_t*)stack_top;
    
    thread->kernel_base_sp = kmalloc(stack_size);
    ASSERT(thread->kernel_base_sp != NULL);
    stack_top = (uint64_t)thread->kernel_base_sp + stack_size;
    thread->kernel_base_sp = (uint64_t*)(stack_top);
    kprintf("Kernel base: %x\n", thread->kernel_base_sp);

    struct context* context = &thread->context;
    __memset(context, 0, sizeof(struct context));
    // context->ds = GDT_KERNEL_DATA;
    context->rdi = (uint64_t)param;
    context->rip = (uint64_t)entry;
    context->cs = GDT_KERNEL_CODE;
    context->rflags = RFLAGS_RESERVED_MASK | RFLAGS_INTERRUPT_MASK;
    context->rsp = (uint64_t)thread->thread_sp;
    context->ss = GDT_KERNEL_DATA;

    thread->state = CREATED;

    thread->next = NULL;
    thread->prev = NULL;

    return thread;
}

uint32_t get_new_tid(void) {
    spinlock_acquire(&tid_lock);

    uint32_t new_tid = cur_tid++;

    spinlock_release(&tid_lock);
    return new_tid;
}