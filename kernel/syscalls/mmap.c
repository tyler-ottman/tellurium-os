#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/syscalls.h>
#include <libc/kmalloc.h>
#include <memory/vmm.h>

void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset) {
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