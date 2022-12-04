#include <arch/exception.h>

__attribute__((noreturn))
void exception_handler(uint8_t vector) { // Stop everything
    kprintf("%s\n", exception_name[vector]);

    done();

    while(1) {} // Never reaches here
}