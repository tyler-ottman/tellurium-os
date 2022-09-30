#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <arch/GDT.h>
#include <arch/idt.h>
#include <arch/terminal.h>
#include <memory/pmm.h>
 
static volatile int a;
extern void overflow(void);

// The following will be our kernel's entry point.
void _start(void) {
    __asm__ volatile ("cli"); // disable interrupts
    init_terminal();
    init_gdt();
    init_idt();
    init_pmm();
 
    terminal_printf(LIGHT_GREEN"Init kernel: complete\n");

    a = 2;
    a /= 0;

    // We're done, just hang...
    done();
}