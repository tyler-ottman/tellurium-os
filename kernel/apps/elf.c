#include <apps/elf.h>
#include <arch/terminal.h>
#include <fs/vfs.h>
#include <libc/kmalloc.h>

int elf_load(pcb_t *proc, const char *path) {
    vnode_t *elf_node;
    vfs_open(&elf_node, vfs_get_root(), path);
    if (!elf_node) {
        return ELF_ERROR;
    }

    // Read file as contiguous chunk of bytes
    int elf_size = elf_node->stat.st_size;
    uint8_t *data = kmalloc(elf_size);
    if (!data) {
        return ELF_ERROR;
    }
    vfs_read(data, elf_node, elf_size, 0);
    
    for (int i = 0; i < elf_size; i++) {
        write_serial(data[i], COM1);
    }

    return ELF_SUCCESS;
}