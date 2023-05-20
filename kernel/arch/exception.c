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

void exception_handler(uint8_t vector) { // Stop everything
    disable_interrupts();
    kprintf(ERROR "%s\n", exception_name[vector]);

    done();

    __builtin_unreachable();
}

extern spinlock_t kprint_lock;
extern terminal_t kterminal;
extern void terminal_printf(terminal_t* terminal, const char* buf);

void exception_handler_err(uint8_t vector, ctx_t *ctx) {
    struct core_local_info *cpu_info = get_core_local_info();
    int core_id = cpu_info->lapic_id;

    uint64_t fault_register = 0;
    __asm__ volatile ("mov %%cr2, %0" : "=r" (fault_register));

    uint64_t err = ctx->err;

    // char buf[512];
    // __snprintf(buf, 512, ERROR "[CPU%d] %s, err: %x, cr2: %x\n", core_id,
    //         exception_name[vector], err, fault_register);
    // // __snprintf(buf, 512, ERROR "stack: %x\n", ctx);
    // spinlock_acquire(&kprint_lock);
    // terminal_printf(&kterminal, buf);
    // spinlock_release(&kprint_lock);

    kprintf(ERROR "[CPU%d] %s, err: %x, cr2: %x\n", core_id,
            exception_name[vector], err, fault_register);
    // kprintf(ERROR "[CPU%d] %x, err: %x, cr2: %x\n", core_id,
    //         exception_name, err, fault_register);

    // breakpoint();
    for (;;) {
        __asm__ volatile ("hlt");
    }

    // done();

    // __builtin_unreachable();
}