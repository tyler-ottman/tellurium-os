#include <acpi/acpi.h>
#include <arch/cpu.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/lock.h>
#include <arch/terminal.h>
#include <devices/lapic.h>
#include <devices/serial.h>
#include <libc/kmalloc.h>
#include <memory/vmm.h>

static volatile struct limine_smp_request kernel_smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

struct core_local_info* get_core_local_info() {
    struct core_local_info* cpu_info = NULL;
    __asm__ volatile ("mov %%gs:0x0, %0" : "=r" (cpu_info));
    return cpu_info;
}

static void set_core_local_info(struct core_local_info* cpu_info) {
    // __asm__ volatile ("mov %0, %%gs:0x0" : "=r" (cpu_info));
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
    kprintf("CPU: %d available cores\n", core_count);

    struct core_local_info* cpu_info = kmalloc(sizeof(struct core_local_info));
    ASSERT(cpu_info != NULL);
    kprintf("This is the address: %x\n", cpu_info);
    set_core_local_info(cpu_info);

    struct limine_smp_info* core;
    for (size_t i = 0; i < core_count; i++) {
        core = smp_response->cpus[i];
        
        if (core->lapic_id == smp_response->bsp_lapic_id) {
            core_init(core);
            continue;
        }
        
        core->goto_address = core_init;
    }

    while (cores_ready != smp_response->cpu_count) {
        __asm__ volatile ("pause");
    }

    kprintf(LIGHT_GREEN "SMP: All cores online\n");
}

void core_init(struct limine_smp_info* core) {
    load_gdt();
    idt_load();

    load_pagemap(get_kernel_pagemap());

    init_lapic();

    uint64_t* core_stack = palloc(1);
    if (core_stack == NULL) {
        kerror("cpu: could not allocate core stack\n");
    }

    cores_ready++;

    if (core->lapic_id != bsp_id) {
        while(1) {}
    }
}