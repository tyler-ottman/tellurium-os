#include <acpi/acpi.h>
#include <arch/cpu.h>
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

void init_system() {
    disable_interrupts();
    init_terminal();
    init_gdt();
    init_idt();
    init_acpi();
    init_pmm();
    init_vmm();
    init_serial();
    init_slab();
}

// The following will be our kernel's entry point.
void _start(void) {
    init_system();
    init_kernel_process();
    
    init_cpu();

    kprintf(LIGHT_GREEN"Init kernel: complete\n");

    // We're done, just hang...
    done();
}
