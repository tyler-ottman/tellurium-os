#include <arch/cpu.h>
#include <arch/framebuffer.h>
#include <arch/gdt.h>
#include <arch/process.h>
#include <arch/syscalls.h>
#include <arch/terminal.h>
#include <devices/msr.h>
#include <libc/kmalloc.h>
#include <memory/vmm.h>

#define SYS_ERROR                                   0
#define SYS_SUCCESS                                 1

#define SYSCALL_GET_FB_CONTEXT                      1
#define SYSCALL_MMAP                                2

extern void* ISR_syscall[];

static int syscall_get_fb_context(fb_context_t *context) {
    if (!context) {
        return SYS_ERROR;
    }

    context->fb_width = fb_get_width();
    context->fb_height = fb_get_height();
    context->fb_pitch = fb_get_pitch();

    // Map framebuffer to user process
    uint64_t addr = (uint64_t)(fb_get_framebuffer());
    addr -= KERNEL_HHDM_OFFSET;
    uint64_t vaddr = addr + 0x900000000; // The offset is arbitrary
    size_t fb_size_bytes = fb_get_pitch() * fb_get_height();
    struct core_local_info *cpu_info = get_core_local_info();
    pcb_t *proc = cpu_info->current_thread->parent;
    uint64_t vmm_flags = PML_PRESENT | PML_NOT_EXECUTABLE | PML_USER | PML_WRITE;
    map_section(proc->pmap, vaddr, addr, fb_size_bytes, vmm_flags);
    context->fb_buff = (void *)vaddr;

    return SYS_SUCCESS;
}

static void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset) {
    size_t num_pages = len / PAGE_SIZE_BYTES;
    if (len % PAGE_SIZE_BYTES != 0) {
        num_pages++;
    }

    size_t num_bytes = num_pages * PAGE_SIZE_BYTES;
    void *section = kmalloc(num_bytes);
    if (!section) {
        return (void *)-1;
    }

    uintptr_t vaddr = (uintptr_t)section - KERNEL_HHDM_OFFSET;
    struct core_local_info *cpu_info = get_core_local_info();
    pcb_t *proc = cpu_info->current_thread->parent;    
    uint64_t vmm_flags = PML_NOT_EXECUTABLE | PML_USER | PML_WRITE | PML_PRESENT;
    map_section(proc->pmap, vaddr, vaddr, num_bytes, vmm_flags);

    return (void *)vaddr;
}

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
    default:
        kprintf("Unknown syscall\n");
        break;
    }

    // Place return value in RAX register
    ctx->rax = ret;
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