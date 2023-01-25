#include <acpi/acpi.h>
#include <arch/cpu.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/lock.h>
#include <arch/scheduler.h>
#include <arch/terminal.h>
#include <devices/lapic.h>
#include <devices/msr.h>
#include <devices/serial.h>
#include <libc/kmalloc.h>
#include <memory/vmm.h>

static volatile struct limine_smp_request kernel_smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

static spinlock_t init_lock = 0;

// Halt CPU activity
void done() {
    for (;;) {
        __asm__("hlt");
    }
}

// For debugging
int a = 0;
void breakpoint() {
    a++;
}

void cpuid(uint32_t in_a, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    __asm__ volatile (
        "cpuid" : "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in_a)
    );
}

thread_t* get_thread_local() {
    uint64_t fs_base = get_msr(FS_BASE);
    return (thread_t*)((uint64_t)fs_base);
}

void set_thread_local(thread_t* thread) {
    uint64_t fs_base = (uint64_t)((uint64_t)thread);
    set_msr(FS_BASE, fs_base);
}

struct core_local_info* get_core_local_info() {
    uint64_t gs_base = get_msr(GS_BASE);
    return (struct core_local_info*)((uint64_t)gs_base);
}

void set_core_local_info(struct core_local_info* cpu_info) {
    uint64_t gs_base = (uint64_t)((uint64_t)cpu_info);
    set_msr(GS_BASE, gs_base);
}

void enable_interrupts() {
    __asm__ volatile ("sti");
}

void disable_interrupts() {
    __asm__ volatile ("cli");
}

static uint32_t cores_ready = 0;
static uint64_t bsp_id = 0;

void init_cpu(void) {
    struct limine_smp_response* smp_response = kernel_smp_request.response;
    size_t core_count = get_core_count();
    bsp_id = smp_response->bsp_lapic_id;
    kprintf(INFO GREEN "CPU: %d available cores\n", core_count);

    struct limine_smp_info* core;
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

    kprintf(INFO GREEN "SMP: All cores online\n");
}

#define NUM_REGISTERS (sizeof(ctx_t) / sizeof(uint64_t))
static char* reg_names[NUM_REGISTERS] = {
    "ds", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
    "err", "rip", "cs", "rflags", "rsp", "ss"
};

void print_context(ctx_t* context) {
    uint64_t* registers = (uint64_t*)context;
    for (size_t i = 0; i < NUM_REGISTERS; i++) {
        kprintf("%s: %016x\n", reg_names[i], registers[i]);
    }
}

void core_init(struct limine_smp_info* core) {
    load_gdt();
    idt_load();

    load_pagemap(get_kernel_pagemap());
    
    uint64_t* core_stack = palloc(1);
    ASSERT(core_stack != NULL);

    struct core_local_info* cpu_info = kmalloc(sizeof(struct core_local_info));
    ASSERT(cpu_info != NULL);
    set_core_local_info(cpu_info);
    cpu_info->abort_stack = core_stack;
    cpu_info->lapic_id = core->lapic_id;
    cpu_info->current_thread = NULL;

    cpu_info->idle_thread = alloc_idle_thread();
    cpu_info->idle_thread->state = RUNNABLE;
    
    __memset(&cpu_info->tss, 0, sizeof(struct TSS));
    load_tss_entry(&cpu_info->tss);
    
    init_lapic();

    // LAPIC Timer IDT Entry uses stack stored in IST1
    set_vector_ist(cpu_info->lapic_timer_vector, 1);

    spinlock_acquire(&init_lock);
    cores_ready++;
    spinlock_release(&init_lock);

    // lapic_send_ipi(cpu_info->lapic_id, cpu_info->lapic_ipi_vector);

    if (core->lapic_id != bsp_id) {
        schedule_next_thread();
        while(1) {}
    }
}