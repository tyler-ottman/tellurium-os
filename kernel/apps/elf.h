#ifndef ELF_H
#define ELF_H

#include <arch/process.h>

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Word;

#define ELF_ERROR                   0
#define ELF_SUCCESS                 1

int elf_load(pcb_t *proc, const char *path);

#endif // ELF_H