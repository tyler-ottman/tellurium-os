#include <arch/terminal.h>
#include <stdint.h>

const char* exception_name[] =  {
    "Divide Error Exception",
    "Debug Exception",
    "NMI Interrupt",
    "Breakpoint Exception",
    "Overflow Exception",
    "BOUND Range Exceeded Exception",
    "Invalid Opcode Exception",
    "Device Not Available Exception",
    "Double Fault Exception",
    "Coprocessor Segment Overrun",
    "Invalid TSS Exception",
    "Segment Not Present",
    "Stack Fault Exception",
    "General Protection Exception",
    "Page-Fault Exception",
    "Reserved",
    "x87 FPU Floating-Point Error",
    "Alignment Check Exception",
    "Machine-Check Exception",
    "SIMD Floating-Point Exception",
    "Virtualization Exception"
};

void exception_handler(uint8_t vector);