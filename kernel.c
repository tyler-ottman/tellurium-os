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
    init_acpi();
    init_pmm();
    init_vmm();
    init_serial();
    init_slab();
}

int a = 0;

void _start(void) {
    init_system();
    init_kernel_process();

    init_cpu();

    kprintf(LIGHT_GREEN"Init kernel: complete\n");

    done();
}
