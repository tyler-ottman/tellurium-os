#include <memory/pmm.h>

static volatile struct limine_memmap_request memory_map_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

void init_pmm(void) {
    // Get memory map
    struct limine_memmap_response* memory_map_response = memory_map_request.response;
    
    uint64_t mmap_entries = memory_map_response->entry_count;
    
    terminal_print("Limine -> MMap entries: ");
    char num_entries[16];
    __utoa(mmap_entries, num_entries, 10);
    terminal_println(num_entries);

    struct limine_memmap_entry** entries = memory_map_response->entries;
    struct limine_memmap_entry* mmap_entry;
    char temp_str[32];
    for (uint64_t idx = 0; idx < mmap_entries; idx++) {
        mmap_entry = entries[idx];
        uint64_t base = mmap_entry->base;
        uint64_t length = mmap_entry->length;
        uint64_t type = mmap_entry->type;

        // Todo, add print with formatting
        __utoa(base, temp_str, 16);
        terminal_print("Base: ");
        terminal_print(temp_str);

        __utoa(length, temp_str, 16);
        terminal_print(" Length: ");
        terminal_print(temp_str);

        __utoa(type, temp_str, 16);
        terminal_print(" Type: ");
        terminal_println(temp_str);
    }
}