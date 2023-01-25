#include <acpi/acpi.h>
#include <arch/cpu.h>
#include <arch/framebuffer.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/process.h>
#include <arch/terminal.h>
#include <devices/lapic.h>
#include <devices/serial.h>
#include <limine.h>
#include <memory/pmm.h>
#include <memory/slab.h>
#include <memory/vmm.h>
#include <stdint.h>
#include <stddef.h>

#include <libc/string.h>

void init_system() {
    init_kterminal();
    init_gdt();
    init_idt();
    init_pmm();
    init_slab();
    init_kterminal_doublebuffer();
    init_acpi();
    init_vmm();
    init_serial();
}

void _start(void) {
    init_system();
    init_kernel_process();

    init_cpu();

    kprintf(INFO GREEN "Kernel: Initialized\n");

    done();
}
