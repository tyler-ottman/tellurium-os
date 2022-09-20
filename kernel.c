#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <arch/GDT.h>
#include <arch/idt.h>
#include <arch/terminal.h>
 
static volatile int a;

// The following will be our kernel's entry point.
void _start(void) {
    __asm__ volatile ("cli"); // disable interrupts
    init_terminal();
    init_gdt();
    init_idt();
 
    terminal_print("Kernel Initialized\n", 20);

    a = 2;
    a /= 0;

    // We're done, just hang...
    done();
}