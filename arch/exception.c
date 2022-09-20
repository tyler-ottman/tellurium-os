#include <arch/exception.h>

// Temporary use
unsigned int my_strlen(const char *s)
{
    const char* start = s;
    while(*s!='\0')
        s++;
    return (unsigned int)(s - start);
}

__attribute__((noreturn))
void exception_handler(uint8_t vector) { // Stop everything
    terminal_print(exception_name[vector], my_strlen(exception_name[vector]));

    __asm__ volatile ("cli");
    __asm__ volatile ("hlt");

    while(1) {} // Never reaches here
}