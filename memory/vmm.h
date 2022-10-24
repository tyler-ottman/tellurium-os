#include <arch/terminal.h>
#include <memory/pmm.h>
#include <libc/string.h>

#define VMM_KERNEL_HIGHER_HALF  0xffffffff80000000

// 4 KB page frames
#define PAGE_SIZE 4096
#define PAGE_ENTRIES 512

// Page Map Level Entry Flags
#define PML_PRESENT             0x001
#define PML_WRITE               0x002
#define PML_USER                0x004
#define PML_PWT                 0x008
#define PML_PCD                 0x010
#define PML_ACCESSED            0x020
#define PML_PHYSICAL_ADDRESS    0x0000fffffffff000
#define PML_NOT_EXECUTABLE      0x8000000000000000

void map_section(uint64_t vaddr_base, uint64_t paddr_base, uint64_t len, uint64_t flags);
void init_vmm(void);
uint64_t align_address(uint64_t addr, bool round_up);
uint64_t* allocate_map(uint64_t* map_base, uint64_t map_entry, uint64_t flags);
bool get_next_page_map(uint64_t** new_map_base, uint64_t* map_base, uint64_t map_entry);
void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);
void unmap_page(uint64_t vaddr);

void recursive_level_print(uint64_t* base, size_t lvls_remaining, size_t num_lvls);
void print_levels(uint64_t* base, uint64_t num_lvls);