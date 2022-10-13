#include <memory/pmm.h>

static volatile struct limine_memmap_request memory_map_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

uint64_t bitmap_max_entries; // Number of page frame entries in bitmap
uint64_t cached_index = 0; // List index in bitmap accessed
uint8_t* bitmap; // Raw bitmap

bool bitmap_available(size_t bitmap_index, size_t pages) {
    for (size_t idx = 0; idx < pages; idx++) {
        uint64_t addr = (bitmap_index + idx) * PAGE_SIZE_BYTES;
        if (bitmap_read(addr)) {
            // Cannot allocate at given address
            return false;
        }
    }
    return true;
}

uint8_t bitmap_read(uint64_t addr) {
    size_t nth_bit = addr / PAGE_SIZE_BYTES;
    return (bitmap[nth_bit / 8] >> (nth_bit % 8)) & 1;
}

void bitmap_set(uint64_t addr) {
    size_t nth_bit = addr / PAGE_SIZE_BYTES;
    bitmap[nth_bit / 8] |= (1 << (nth_bit % 8));
}

void bitmap_reset(uint64_t addr) {
    size_t nth_bit = addr / PAGE_SIZE_BYTES;
    bitmap[nth_bit / 8] &= ~(1 << (nth_bit % 8));
}

uint64_t palign(size_t frames) {
    // frames / 8 bytes needed for bitmap
    size_t bytes = frames / 8;
    if (bytes * 8 < frames) {
        // If frames not divisible by 8, add extra byte
        bytes++;
    }

    // Now align to 4kb
    frames =  bytes / PAGE_SIZE_BYTES;
    if (frames * PAGE_SIZE_BYTES < bytes) {
        frames++;
    }
    return frames * PAGE_SIZE_BYTES;
}

void init_pmm(void) {
    // Get memory map
    struct limine_memmap_response* memory_map_response = memory_map_request.response;
    struct limine_memmap_entry** entries = memory_map_response->entries;
    struct limine_memmap_entry* entry;

    // Print number of memory map entries
    size_t mmap_entries = memory_map_response->entry_count;
    terminal_printf("Limine -> MMap entries: %u\n", mmap_entries);  

    // Calculate max physical address page frames can be allocated
    uint64_t max_address = 0;
    for (size_t idx = 0; idx < mmap_entries; idx++) {
        entry = entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            max_address = entry->base + entry->length;
        }
    }

    uint64_t frames = max_address / PAGE_SIZE_BYTES;
    bitmap_max_entries = frames;
    // Align bitmap size to neareset 4kb
    uint64_t bitmap_size_aligned = palign(frames);

    // Find spot in memory for bitmap data structure
    for (size_t idx = 0; idx < mmap_entries; idx++) {
        entry = entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->length > bitmap_size_aligned) {
                bitmap = (uint8_t*)entry->base;
                entry->base += bitmap_size_aligned;
                entry->length -= bitmap_size_aligned;
                break;
            }
        }
    }

    // Mark sections as allocated/free
    __memset((void*)bitmap, 0xff, bitmap_size_aligned);
    terminal_printf("PMM: bitmap addr: %016x: %d\n", bitmap, *bitmap);
    uint64_t* test = (uint64_t*)(0xffff800000000000 + bitmap);
    *test = 0x4f;
    terminal_printf("AFTER: %016x\n", *bitmap);
    for (size_t idx = 0; idx < mmap_entries; idx++) {
        entry = entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (size_t jdx = 0; jdx < entry->length; jdx += PAGE_SIZE_BYTES) {
                bitmap_reset(entry->base + jdx);
            }
        }
    }

    // PMM: Debug info
    uint64_t available_bytes = 0;
    for (size_t idx = 0; idx < mmap_entries; idx++) {
        entry = entries[idx];
        terminal_printf("Base: %016x, Len: %16x, Type: %u\n", entry->base, entry->length, entry->type);
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            available_bytes += entry->length;
        }
    }

    uint64_t free_frames = available_bytes / PAGE_SIZE_BYTES;
    uint64_t reserved_frames = frames - free_frames;
    terminal_printf("PMM: Page frames -> Total: %u, Reserved: %u, Free: %u\n", frames, reserved_frames, free_frames);
    // terminal_printf("PMM: Bitmap size aligned: %u\n", bitmap_size_aligned);

    terminal_printf(LIGHT_GREEN "PMM: Initialized\n");
}

void* palloc_internal(size_t pages) {
    while (cached_index + pages < bitmap_max_entries) {
        // Try to allocates pages at current index
        if (bitmap_available(cached_index, pages)) {
            terminal_printf("Allocate %u frames at %16x\n", pages, cached_index * PAGE_SIZE_BYTES);
            for (size_t idx = 0; idx < pages; idx++) {
                uint64_t addr = (cached_index + idx) * PAGE_SIZE_BYTES;
                bitmap_set(addr);
            }
            void* ret = (void*)(cached_index * PAGE_SIZE_BYTES);
            cached_index += pages;
            return ret;
        }
        cached_index++;
    }
    return NULL;
}

void* pmm_alloc(size_t pages) {
    void* ret = (void*)palloc_internal(pages);

    if (!ret) {
        // https://wiki.osdev.org/Page_Frame_Allocation
        // Information about caching index
        cached_index = 0;
        ret = (void*)palloc_internal(pages);
    }

    return ret;
}

void pmm_free(void* base, size_t pages) {
    uint64_t addr = (uint64_t)base;
    for (size_t idx = 0; idx < pages; idx++) {
        bitmap_reset(addr);
        addr += PAGE_SIZE_BYTES;
    }
}