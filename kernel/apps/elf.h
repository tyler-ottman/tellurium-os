#ifndef ELF_H
#define ELF_H

#include <arch/process.h>

#define ELF_ERROR                   0
#define ELF_SUCCESS                 1

int elf_load(pcb_t *proc, const char *path, uint64_t *entry);

#endif // ELF_H