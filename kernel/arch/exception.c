#include <arch/exception.h>
#include <arch/lock.h>
#include <libc/print.h>

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

__attribute__((noreturn))
void exception_handler(uint8_t vector) {
    kprintf(ERROR "%s\n", exception_name[vector]);

    core_hlt();

    __builtin_unreachable();
}

__attribute__((noreturn))
void exception_handler_err(uint8_t vector, ctx_t *ctx) {
    struct core_local_info *cpu_info = get_core_local_info();
    int core_id = cpu_info->lapic_id;

    uint64_t cr2;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (cr2));

    kprintf(ERROR "[CPU%d] %s, err: %x, cr2: %x\n", core_id,
            exception_name[vector], ctx->err, cr2);

    core_hlt();

    __builtin_unreachable();
}