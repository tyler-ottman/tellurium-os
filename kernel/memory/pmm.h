#ifndef PMM_H
#define PMM_H

#include <stdbool.h>
#include <arch/terminal.h>
#include <libc/string.h>

#define KERNEL_HHDM_OFFSET  (kernel_hhdm_request.response->offset)
#define PAGE_SIZE_BYTES     4096

extern volatile struct limine_hhdm_request kernel_hhdm_request;
extern volatile struct limine_memmap_request memory_map_request;

uint8_t* get_bitmap_addr(void);
size_t get_bitmap_size(void);

uint8_t bitmap_read(uint64_t addr);
bool bitmap_available(size_t bitmap_index, size_t pages);
void bitmap_set(uint64_t addr);
void bitmap_reset(uint64_t addr);
uint64_t palign(size_t frames);
void init_pmm(void);
void* palloc_internal(size_t pages);

void* palloc(size_t pages);
void pfree(void* base, size_t pages);

#endif // PMM_H