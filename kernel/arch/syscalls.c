#include <arch/cpu.h>
#include <arch/gdt.h>
#include <arch/syscalls.h>
#include <arch/terminal.h>
#include <devices/msr.h>

extern void* ISR_syscall[];

void syscall_handler(ctx_t *ctx) {
    kprintf(INFO "SYSCALL id: %d\n", ctx->rax);
}

void init_syscall() {
    // Store address of syscall isr
    set_msr(IA32_LSTAR, (uint64_t)ISR_syscall);

    // Clear rflags mask register
    set_msr(IA32_FMASK, 0);

    // Set CS/SS selector field for syscall/sysret
    set_msr(IA32_STAR, ((uint64_t)GDT_KERNEL_CODE << 32));
    uint16_t selector = GDT_USER_DATA - 0x8;
    uint64_t msr = get_msr(IA32_STAR) | ((uint64_t)selector << 48);
    set_msr(IA32_STAR, msr);

    // Enable syscalls
    set_msr(IA32_EFER, get_msr(IA32_EFER) | 1);
}