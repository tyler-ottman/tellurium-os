#include <arch/gdt.h>
#include <arch/kterminal.h>
#include <arch/syscalls.h>
#include <devices/msr.h>

#define SYSCALL_GET_FB_CONTEXT                      1
#define SYSCALL_MMAP                                2
#define SYSCALL_OPEN                                3
#define SYSCALL_READ                                4
#define SYSCALL_LSEEK                               5

extern void *ISR_syscall[];

void syscall_handler(ctx_t *ctx) {    
    // Syscall ID palced in RAX
    uint64_t syscall_id = ctx->rax;

    // x86_64 syscall arguments
    uint64_t arg1 = ctx->rdi;
    uint64_t arg2 = ctx->rsi;
    uint64_t arg3 = ctx->rdx;
    uint64_t arg4 = ctx->r10;
    uint64_t arg5 = ctx->r8;
    uint64_t arg6 = ctx->r9;

    size_t ret = 0;

    switch (syscall_id) {
    case SYSCALL_GET_FB_CONTEXT:
        ret = syscall_get_fb_context((fb_context_t *)arg1);
        break;
    case SYSCALL_MMAP:
        ret = (size_t)syscall_mmap((void *)arg1, arg2, arg3, arg4, arg5, arg6);
        break;
    case SYSCALL_OPEN:
        ret = (int)syscall_open((const char *)arg1, arg2);
        break;
    case SYSCALL_READ:
        ret = (size_t)syscall_read(arg1, (void *)arg2, arg3);
        break;
    case SYSCALL_LSEEK:
        ret = (int)syscall_lseek((int)arg1, arg2, (int)arg3);
        break;
    default:
        kprintf("Unknown syscall: %x\n", syscall_id);
        break;
    }

    // Place return value in RAX register
    ctx->rax = ret;
}

void init_syscall() {
    // Store address of syscall isr
    set_msr(IA32_LSTAR, (uint64_t)ISR_syscall);

    // Mask all fields in rflags
    set_msr(IA32_FMASK, 0x3f7fd5);

    // Set CS/SS selector field for syscall/sysret
    set_msr(IA32_STAR, ((uint64_t)(GDT_KERNEL_CODE | 3) << 32));
    uint16_t selector = (GDT_USER_DATA | 3) - 0x8;
    uint64_t msr = get_msr(IA32_STAR) | ((uint64_t)selector << 48);
    set_msr(IA32_STAR, msr);

    // Enable syscalls
    set_msr(IA32_EFER, get_msr(IA32_EFER) | 1);
}