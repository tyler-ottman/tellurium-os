#include <memory/pmm.h>

static volatile struct limine_memmap_request memory_map_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

void init_pmm(void) {
    // Get memory map
    struct limine_memmap_response* memory_map_response = memory_map_request.response;
    
    uint64_t mmap_entries = memory_map_response->entry_count;
    
    terminal_printf("Limine -> MMap entries: %u\n", mmap_entries);

    struct limine_memmap_entry** entries = memory_map_response->entries;
    struct limine_memmap_entry* mmap_entry;
    for (uint64_t idx = 0; idx < mmap_entries; idx++) {
        mmap_entry = entries[idx];
        uint64_t base = mmap_entry->base;
        uint64_t length = mmap_entry->length;
        uint64_t type = mmap_entry->type;

        terminal_printf("Base: %016x, Len: %16x, Type: %u\n", base, length, type);
    }
}