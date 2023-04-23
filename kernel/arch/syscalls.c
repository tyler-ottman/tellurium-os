#include <arch/cpu.h>
#include <arch/gdt.h>
#include <arch/syscalls.h>
#include <arch/terminal.h>
#include <devices/msr.h>

extern void* ISR_syscall[];

void syscall_handler(int sys_id, int arg1, int arg2, int arg3) {
    kprintf("Received syscall\n");
}

void init_syscall() {
    // Store address of syscall isr
    set_msr(IA32_LSTAR, (uint64_t)ISR_syscall);

    // Clear rflags mask register
    set_msr(IA32_FMASK, 0);

    // Set CS/SS selector field for syscall/sysret
    set_msr(IA32_STAR, ((uint64_t)GDT_KERNEL_CODE << 32));
    uint64_t msr = get_msr(IA32_STAR) | ((uint64_t)GDT_USER_CODE << 48); 
    set_msr(IA32_STAR, msr);

    // Enable syscalls
    set_msr(IA32_EFER, get_msr(IA32_EFER) | 1);
}