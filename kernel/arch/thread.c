#include <arch/gdt.h>
#include <arch/lock.h>
#include <arch/process.h>
#include <libc/kmalloc.h>
#include <libc/vector.h>
#include <memory/pmm.h>
#include <memory/vmm.h>

extern void thread_wrapper(struct core_local_info *cpu_info);

static spinlock_t tid_lock = 0;
static uint32_t cur_tid = 1;

static void idle_thread_spin(void) {
    // kprintf("Idle Thread reached\n");

    for(;;) {
        __asm__ volatile ("pause");
    }
}

thread_t* alloc_idle_thread(void) {
    void* entry = (void*)((uint64_t)idle_thread_spin);
    return create_kernel_thread(entry, NULL);
}

thread_t* get_idle_thread(void) {
    struct core_local_info* cpu_info = get_core_local_info();
    return cpu_info->idle_thread;
}

void thread_destroy(thread_t* thread) {
    if (!thread) {
        return;
    }
    
    if (!thread->thread_base_sp) {
        kfree(thread->thread_base_sp);
    }

    if (!thread->kernel_base_sp) {
        kfree(thread->thread_base_sp);
    }

    // Remove thread from pare
    struct pcb* parent = thread->parent;
    for (int i = 0; i < parent->threads.cur_elements; i++) {
        thread_t* cur_thread = VECTOR_GET(parent->threads, i);
        if (cur_thread == thread) {
            VECTOR_REMOVE(parent->threads, i);
            break;
        }
    }

    kfree(thread);
}

thread_t* create_kernel_thread(void* entry, void* param) {
    if (!entry) {
        return NULL;
    }

    thread_t* thread = kmalloc(sizeof(thread_t));
    if (!thread) {
        return NULL;
    }

    thread->tid = get_new_tid();

    thread->cpu_id = -1;

    thread->parent = get_kernel_process();

    uint64_t stack_top;
    size_t stack_size = PAGE_SIZE_BYTES;
    thread->thread_base_sp = kmalloc(stack_size);
    if (!thread->thread_base_sp) {
        thread_destroy(thread);
        return NULL;
    }
    stack_top = (uint64_t)thread->thread_base_sp + stack_size;
    thread->thread_sp = (uint64_t*)(stack_top - 8);
    
    thread->kernel_base_sp = kmalloc(stack_size);
    if (!thread->kernel_base_sp) {
        thread_destroy(thread);
        return NULL;
    }
    stack_top = (uint64_t)thread->kernel_base_sp + stack_size;
    thread->kernel_sp = (uint64_t*)(stack_top);

    ctx_t* context = &thread->context;
    __memset(context, 0, sizeof(ctx_t));
    context->rsi = (uint64_t)param;
    context->rdi = (uint64_t)entry;
    context->rip = (uint64_t)thread_wrapper;

    context->cs = GDT_KERNEL_CODE;
    context->rflags = RFLAGS_RESERVED_MASK | RFLAGS_INTERRUPT_MASK;
    context->rsp = (uint64_t)thread->thread_sp;
    context->ss = GDT_KERNEL_DATA;

    thread->state = CREATED;
    thread->isKernel = true;
    thread->received_event = NULL;

    thread->prev = NULL;
    thread->next = NULL;

    return thread;
}

thread_t *create_user_thread(struct pcb *proc, void *entry, void *param) {
    if (!entry) {
        return NULL;
    }

    thread_t* thread = kmalloc(sizeof(thread_t));
    if (!thread) {
        return NULL;
    }

    thread->tid = get_new_tid();

    thread->cpu_id = -1;

    thread->parent = proc;

    uint64_t stack_top;
    size_t stack_size = PAGE_SIZE_BYTES;
    thread->thread_base_sp = kmalloc(stack_size);
    if (!thread->thread_base_sp) {
        thread_destroy(thread);
        return NULL;
    }
    uint64_t addr = (uint64_t)thread->thread_base_sp - KERNEL_HHDM_OFFSET;
    thread->thread_base_sp = (uint64_t *)addr;
    stack_top = (uint64_t)thread->thread_base_sp + stack_size;
    thread->thread_sp = (uint64_t*)(stack_top - 8);

    // Now map stack to userspace
    uint64_t vaddr = (uint64_t)thread->thread_base_sp;
    map_section(proc->pmap, vaddr, vaddr, stack_size, PML_PRESENT | PML_NOT_EXECUTABLE | PML_USER | PML_WRITE);

    thread->kernel_base_sp = kmalloc(stack_size);
    if (!thread->kernel_base_sp) {
        thread_destroy(thread);
        return NULL;
    }
    stack_top = (uint64_t)thread->kernel_base_sp + stack_size;
    thread->kernel_sp = (uint64_t*)(stack_top);

    ctx_t* context = &thread->context;
    __memset(context, 0, sizeof(ctx_t));
    context->rdi = (uint64_t)param;
    context->rip = (uint64_t)entry;

    context->cs = GDT_USER_CODE | 3;
    context->rflags = RFLAGS_RESERVED_MASK | RFLAGS_INTERRUPT_MASK;
    context->rsp = (uint64_t)thread->thread_sp;
    context->ss = GDT_USER_DATA | 3;

    thread->state = CREATED;
    thread->isKernel = false;
    thread->received_event = NULL;

    thread->prev = NULL;
    thread->next = NULL;

    return thread;
}

uint32_t get_new_tid(void) {
    spinlock_acquire(&tid_lock);

    uint32_t new_tid = cur_tid++;

    spinlock_release(&tid_lock);
    return new_tid;
}
