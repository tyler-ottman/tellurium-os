#include <arch/framebuffer.h>
#include <arch/process.h>
#include <arch/syscalls.h>
#include <memory/vmm.h>

int syscall_get_fb_context(fb_context_t *context) {
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