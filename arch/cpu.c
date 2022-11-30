#include <arch/cpu.h>

static volatile struct limine_smp_request kernel_smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0
};

void init_cpu(void) {
    struct limine_smp_response* smp_response = kernel_smp_request.response;
    size_t core_count = get_core_count();
    kprintf("CPU: %x available cores\n", core_count);

    for (size_t i = 0; i < core_count; i++) {
        struct limine_smp_info* core = smp_response->cpus[i];
        
        if (core->lapic_id == smp_response->bsp_lapic_id) continue;

        core->goto_address = core_init;
    }
    kprintf("BSP: AP startup complete\n");
}

void core_init(struct limine_smp_info* core) {
    load_gdt();
    idt_load();

    load_pagemap(get_kernel_pagemap());

    
    while(1) {}
}