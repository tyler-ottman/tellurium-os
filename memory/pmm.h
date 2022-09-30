#include <stdbool.h>
#include <arch/terminal.h>
#include <libc/string.h>

#define PAGE_SIZE_BYTES     4096

// https://wiki.osdev.org/Page_Frame_Allocation
void init_pmm(void);

// Alocated n contiguous pages
void* palloc(size_t n);

// Free allocated pages
void pfree(void* base, size_t n);