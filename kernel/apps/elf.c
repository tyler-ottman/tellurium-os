#include <apps/elf.h>
#include <arch/kterminal.h>
#include <fs/vfs.h>
#include <klib/kmalloc.h>
#include <memory/vmm.h>
#include <sys/misc.h>

// e_ident
#define EI_MAG                          0
#define EI_CLASS                        4
#define EI_DATA                         5
#define EI_VERSION                      6
#define EI_OSABI                        7
#define EI_ABIVERSION                   8

// File Identification (EI_MAG)
#define EI_MAG0                         0x7f
#define EI_MAG1                         'E'
#define EI_MAG2                         'L'
#define EI_MAG3                         'F'

// File class (EI_CLASS)
#define ELFCLASS64                      2

// Data Encoding (EI_DATA)
#define ELFDATA2LSB                     1

// Operating System and ABI Identifiers
#define ELFOSABI_SYSV                   0

// OSABI version (0 for System V ABI)
#define ABIVERSION_SYSV                 0

#define ELF_ASSERT(cond)                \
        {                               \
            if (!(cond)) {              \
                return ELF_ERROR;       \
            }                           \
        }

// e_type
#define ET_EXEC                         2

// e_machine
#define EM_X86_64                       62

// e_version
#define EV_CURRENT                      1

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;

// p_type
#define PT_LOAD                         1

// p_flags
#define PF_X                            0x1
#define PF_W                            0x2
#define PF_R                            0x4

// ELF Header
typedef struct Elf64_Ehdr {
    unsigned char e_ident[16];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
} Elf64_Ehdr_t;

// ELF Program Header
typedef struct Elf64_Phdr {
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr_t;

static int elf_verify_header(Elf64_Ehdr_t *header) {
    const char magic[] = {EI_MAG0, EI_MAG1, EI_MAG2, EI_MAG3};
    ASSERT_RET(!__strncmp((const char *)&header->e_ident[EI_MAG], magic, 4), ELF_ERR);
    ASSERT_RET(header->e_ident[EI_CLASS] == ELFCLASS64, ELF_ERR);
    ASSERT_RET(header->e_ident[EI_DATA] == ELFDATA2LSB, ELF_ERR);
    ASSERT_RET(header->e_ident[EI_OSABI] == ELFOSABI_SYSV, ELF_ERR);
    ASSERT_RET(header->e_ident[EI_ABIVERSION] == ABIVERSION_SYSV, ELF_ERR);
    ASSERT_RET(header->e_type == ET_EXEC, ELF_ERR);
    ASSERT_RET(header->e_machine == EM_X86_64, ELF_ERR);
    ASSERT_RET(header->e_version == EV_CURRENT, ELF_ERR);
    return ELF_SUCCESS;
}

static const Elf64_Phdr_t *elf_get_phdr(void *elf_raw, int n) {
    Elf64_Ehdr_t *header = (Elf64_Ehdr_t *)elf_raw;
    if (!header || (n < 0) || (n >= header->e_phnum)) {
        return NULL;
    }

    uint64_t addr = (uint64_t)elf_raw + header->e_phoff + n * header->e_phentsize;
    return (const Elf64_Phdr_t *)addr;
}

int elf_load(pcb_t *proc, const char *path, uint64_t *entry) {
    vnode_t *elf_node;
    int err = vfs_open(&elf_node, vfs_get_root(), path);
    ASSERT(!err, err, "elf load fail");

    if (!elf_node) {
        return ELF_ERR;
    }

    // Read file as contiguous chunk of bytes
    uint8_t *data = kmalloc(elf_node->stat.st_size);
    if (!data) {
        return ELF_ERR;
    }

    size_t bytes_read;
    int ret = vfs_read(data, elf_node, elf_node->stat.st_size, 0, &bytes_read);
    if (ret) {
        return ELF_ERR;
    }
    
    Elf64_Ehdr_t *header = (Elf64_Ehdr_t *)data;
    if (!elf_verify_header(header)) {
        return ELF_ERR;
    }

    if (!entry) {
        return ELF_ERR;
    }
    
    *entry = header->e_entry;

    // Map program sections to user process vspace
    for (int i = 0; i < header->e_phnum; i++) {
        const Elf64_Phdr_t *p_hdr = elf_get_phdr(data, i);

        if (p_hdr->p_type != PT_LOAD) {
            continue;
        }

        uint64_t vmm_flags = PML_NOT_EXECUTABLE | PML_USER | PML_PRESENT;
        if (p_hdr->p_flags & PF_X) {
            vmm_flags &= ~(PML_NOT_EXECUTABLE);
        }

        if (p_hdr->p_flags & PF_W) {
            vmm_flags |= PML_WRITE;
        }

        uint64_t offset = p_hdr->p_vaddr & 0xfff;
        uint64_t vaddr = p_hdr->p_vaddr - offset;
        uint64_t size_bytes = offset + p_hdr->p_memsz;
        int num_pages = size_bytes / PAGE_SIZE_BYTES;
        if (size_bytes % PAGE_SIZE_BYTES != 0) {
            num_pages++;
        }

        // Load program section to memory
        void *p_section = kmalloc(num_pages * PAGE_SIZE_BYTES);
        if (!p_section) { // very bad
            return ELF_ERR;
        }
        // Address where program section will be stored in memory
        __memset(p_section, '\0', num_pages * PAGE_SIZE_BYTES);

        // Location of program section in ELF file
        const void *p_section_data = (const void *)((uint64_t)data + p_hdr->p_offset);

        // If program section start is not page aligned
        p_section = (void *)((uint64_t)p_section + offset);
        __memcpy(p_section, p_section_data, p_hdr->p_memsz);

        // Map section to process's userland
        uint64_t paddr = (uint64_t)p_section - KERNEL_HHDM_OFFSET; // Physical address of page frame
        uint64_t size = num_pages * PAGE_SIZE_BYTES;
        // map_section(get_kernel_pagemap(), vaddr, paddr, size, vmm_flags);
        map_section(proc->pmap, vaddr, paddr, size, vmm_flags);
    }

    return ELF_SUCCESS;
}