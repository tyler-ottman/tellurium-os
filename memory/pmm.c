#include <memory/pmm.h>

static volatile struct limine_memmap_request memory_map_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

uint64_t bitmap_len; // Number of page frame entries in bitmap
uint64_t cached_index = 0; // List index in bitmap accessed
uint8_t* bitmap; // Raw bitmap

void init_pmm(void) {
    // Get memory map
    struct limine_memmap_response* memory_map_response = memory_map_request.response;
    struct limine_memmap_entry** entries = memory_map_response->entries;
    struct limine_memmap_entry* entry;

    // Print number of memory map entries
    size_t mmap_entries = memory_map_response->entry_count;
    terminal_printf("Limine -> MMap entries: %u\n", mmap_entries);  
    
    // Print MMap entries: base, size, type
    for (size_t idx = 0; idx < mmap_entries; idx++) {
        entry = entries[idx];
        terminal_printf("Base: %016x, Len: %16x, Type: %u\n", entry->base, entry->length, entry->type);
    }

    // Calculate max physical address page frames can be allocated
    // Calculate number of available bytes in all usable memory section
    uint64_t available_bytes = 0;
    uint64_t max_address = 0;
    for (size_t idx = 0; idx < mmap_entries; idx++) {
        entry = entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            max_address = entry->base + entry->length;
            available_bytes += entry->length;
        }
    }

    // Calculate frame statistics
    uint64_t frames = max_address / PAGE_SIZE_BYTES;
    uint64_t free_frames = available_bytes / PAGE_SIZE_BYTES;
    uint64_t reserved_frames = frames - free_frames;
    terminal_printf(LIGHT_RED"PMM: Page frames -> Total: %u, Reserved: %u, Free: %u\n", frames, reserved_frames, free_frames);
    
    // Calculate number of bitmap entries
    bitmap_len = frames;
    uint64_t bitmap_size_bytes = frames / 8;
    if (frames % PAGE_SIZE_BYTES) bitmap_size_bytes++;
    terminal_printf("PMM: Bitmap size in bytes: %u\n", bitmap_size_bytes);

    // Find spot in memory for bitmap data structure
    bool flag_bitmap_allocated = false;
    for (size_t idx = 0; idx < mmap_entries; idx++) {
        entry = entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->length > bitmap_size_bytes) {
                flag_bitmap_allocated = true;
                bitmap = (uint8_t*)entry->base;
                terminal_printf("PMM: Insert bitmap in mmap section %u\n", idx);
            }
        }
    }
}