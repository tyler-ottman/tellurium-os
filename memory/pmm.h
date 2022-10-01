#include <stdbool.h>
#include <arch/terminal.h>
#include <libc/string.h>
#include <string.h>

#define PAGE_SIZE_BYTES     4096

uint8_t bitmap_read(uint64_t addr);

// set bit at corresponding address to 0/1
void bitmap_set(uint64_t addr);
void bitmap_reset(uint64_t addr);

// Align page to 4kb
uint64_t palign(size_t frames);

// https://wiki.osdev.org/Page_Frame_Allocation
void init_pmm(void);

// Alocated n contiguous pages
void* palloc(size_t n);

// Free allocated pages
void pfree(void* base, size_t n);