#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <arch/GDT.h>
#include <arch/idt.h>
#include <arch/terminal.h>
#include <memory/pmm.h>
#include <memory/vmm.h>

// The following will be our kernel's entry point.
void _start(void) {
    __asm__ volatile ("cli"); // disable interrupts
    init_terminal();
    init_gdt();
    init_idt();
    init_pmm();
    // init_vmm();

    terminal_printf(LIGHT_GREEN"Init kernel: complete\n");
    
    uint64_t* temp = (uint64_t*)(0xffffffff70000000);
    uint64_t a = *temp;
    terminal_printf("Here it is: %x\n", a);

    // We're done, just hang...
    done();
}