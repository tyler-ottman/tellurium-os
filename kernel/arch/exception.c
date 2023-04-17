#include <arch/cpu.h>
#include <arch/exception.h>

__attribute__((noreturn))
void exception_handler(uint8_t vector) { // Stop everything
    kprintf("%s", exception_name[vector]);

    if (vector == 14) {
        uint64_t fault_register = 0;
        __asm__ volatile ("mov %%cr2, %0" : "=r" (fault_register));
        kprintf(": %x", fault_register);
    }

    kprintf("\n");

    done();

    while(1) {} // Never reaches here
}