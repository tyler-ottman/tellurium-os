#include <arch/exception.h>

__attribute__((noreturn))
void exception_handler(uint8_t vector) { // Stop everything
    terminal_println(exception_name[vector]);

    __asm__ volatile ("cli");
    __asm__ volatile ("hlt");

    while(1) {} // Never reaches here
}