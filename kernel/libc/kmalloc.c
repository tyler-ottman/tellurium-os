#include <libc/kmalloc.h>
#include <arch/terminal.h>

void *kmalloc(size_t size) {
    void *addr = slab_alloc(size);
    // kprintf(" %x ", addr);
    return addr;
    
    // return slab_alloc(size);
}

void *krealloc(void *addr, size_t size) {
    return slab_realloc(addr, size);
}

void kfree(void *addr) {
    slab_free(addr);
}