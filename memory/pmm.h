#include <stdbool.h>
#include <arch/terminal.h>
#include <libc/string.h>
#include <string.h>

#define PAGE_SIZE_BYTES     4096

extern volatile struct limine_memmap_request memory_map_request;

uint8_t* get_bitmap_addr(void);
size_t get_bitmap_size(void);

// Read status of bitmap index
uint8_t bitmap_read(uint64_t addr);

// Check the status of bitmap at 'index' for 'pages' entries
bool bitmap_available(size_t bitmap_index, size_t pages);

// set bit at corresponding address to 0/1
void bitmap_set(uint64_t addr);
void bitmap_reset(uint64_t addr);

// Align page to 4kb
uint64_t palign(size_t frames);

// https://wiki.osdev.org/Page_Frame_Allocation
void init_pmm(void);

// Internal page frame allocator
void* palloc_internal(size_t pages);

// Alocated n contiguous pages
void* pmm_alloc(size_t pages);

// Free allocated pages
void pmm_free(void* base, size_t pages);