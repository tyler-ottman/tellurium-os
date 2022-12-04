#include <acpi/acpi.h>
#include <arch/cpu.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/terminal.h>
#include <devices/lapic.h>
#include <devices/serial.h>
#include <limine.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <stdint.h>
#include <stddef.h>

// The following will be our kernel's entry point.
void _start(void) {
    // __asm__ volatile ("cli"); // disable interrupts
    init_terminal();
    init_gdt();
    init_idt();
    init_acpi();
    init_pmm();
    init_vmm();
    init_serial();
    
    init_cpu();

    // kprintf(LIGHT_GREEN"Init kernel: complete\n");

    // We're done, just hang...
    done();
}