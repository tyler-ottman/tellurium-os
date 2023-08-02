#include <acpi/acpi.h>
#include <arch/cpu.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/kterminal.h>
#include <arch/lock.h>
#include <arch/scheduler.h>
#include <arch/syscalls.h>
#include <devices/lapic.h>
#include <devices/serial.h>
#include <klib/kmalloc.h>
#include <memory/vmm.h>
#include <sys/misc.h>

static volatile struct limine_smp_request kernel_smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

extern void kmain(void);

static spinlock_t init_lock = 0;

static inline core_t *core_get_local_unsafe() {
    core_t *core;

    __asm__ (
        "cli\n\t"
        "swapgs\n\t"
        "movq %%gs:0, %0\n\t"
        "swapgs\n\t" :
        "=r"(core)
    );

    return core;
}

void cpuid(uint32_t in_a, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    __asm__ volatile (
        "cpuid" : "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in_a)
    );
}

inline void enable_interrupts() {
    __asm__ ("sti");
}

inline void disable_interrupts() {
    __asm__ ("cli");
}

inline int core_get_if_flag() {
    uint64_t rflags;

    __asm__ volatile (
        "pushfq\n\t"
        "pop %0" :
        "=rm" (rflags) : :
        "memory"
    );

    return rflags & (1 << 9);
}

inline void core_hlt() {
    for (;;) {
        __asm__ ("hlt");
    }
}

inline thread_t *get_thread_local() {
    int i_flag = core_get_if_flag();
    
    core_t *core = core_get_local_unsafe();
    thread_t *thread = core->current_thread;

    if (i_flag) {
        __asm__ ("sti");
    }

    return thread;
}

inline core_t *get_core_local_info() {
    int i_flag = core_get_if_flag();

    core_t *core = core_get_local_unsafe();

    if (i_flag) {
        __asm__ ("sti");
    }
    
    return core;
}

void set_core_local_info(core_t *core) {
    core->self = core;
    
    set_msr(IA32_KERNEL_GS_BASE, (uint64_t)core);
    set_msr(GS_BASE, (uint64_t)core);
}

void save_context(ctx_t *ctx) {
    core_t *core = get_core_local_info();
    thread_t *thread = core->current_thread;
    
    __memcpy(&thread->context, ctx, sizeof(ctx_t));

    // Save scratch register
    thread->thread_scratch = core->kernel_scratch;
}

static uint32_t cores_ready = 0;
static uint64_t bsp_id = 0;

void init_cpu(void) {
    struct limine_smp_response *smp_response = kernel_smp_request.response;
    size_t core_count = get_core_count();
    bsp_id = smp_response->bsp_lapic_id;
    kprintf(INFO GREEN "CPU: %d available cores\n", core_count);

    struct limine_smp_info *core;
    for (size_t i = 0; i < core_count; i++) {
        core = smp_response->cpus[i];
        
        if (core->lapic_id == smp_response->bsp_lapic_id) {
            // core_init(core);
            continue;
        }
        
        core->goto_address = core_init;
    }

    while (cores_ready != smp_response->cpu_count - 1) {
        __asm__ volatile ("pause");
    }

    core_init(smp_response->cpus[bsp_id]);

    kprintf(INFO GREEN "SMP: All cores online\n");

    void *kmain_entry = (void *)((uint64_t)kmain);
    thread_t *thread_kmain = (thread_t *)create_kernel_thread(kmain_entry, NULL);
    schedule_add_thread(thread_kmain);

    schedule_next_thread();
}

#define NUM_REGISTERS (sizeof(ctx_t) / sizeof(uint64_t))
static char* reg_names[NUM_REGISTERS] = {
    "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
    "err", "rip", "cs", "rflags", "rsp", "ss"
};

void print_context(ctx_t *context) {
    uint64_t *registers = (uint64_t *)context;
    for (size_t i = 0; i < NUM_REGISTERS; i++) {
        kprintf("%s: %016x\n", reg_names[i], registers[i]);
    }
}

void core_init(struct limine_smp_info *limine_core_info) {
    load_gdt();
    idt_load();

    load_pagemap(get_kernel_pagemap());

    core_t* core = kmalloc(sizeof(core_t));
    ASSERT(core != NULL, ERR_NO_MEM, NULL);
    set_core_local_info(core);

    core->kernel_stack = NULL;
    core->irq_stack = kmalloc(4 * PAGE_SIZE_BYTES);    
    ASSERT(core->irq_stack, ERR_NO_MEM, NULL);
    
    core->lapic_id = limine_core_info->lapic_id;

    core->current_thread = NULL;
    
    core->idle_thread = alloc_idle_thread();
    ASSERT(core->idle_thread, ERR_NO_MEM, NULL);
    core->idle_thread->state = THREAD_RUNNABLE;
   
    __memset(&core->tss, 0, sizeof(struct TSS));
    load_tss_entry(&core->tss);

    init_lapic();
    
    set_vector_ist(core->lapic_timer_vector, 1);
    set_vector_ist(core->lapic_ipi_vector, 1);
    for (int i = 0; i < 20; i++) {
        set_vector_ist(i, 1);
    }

    init_syscall();

    spinlock_acquire(&init_lock);
    cores_ready++;
    spinlock_release(&init_lock);

    if (limine_core_info->lapic_id != bsp_id) {
        schedule_next_thread();
        while(1) {}
    }
}
