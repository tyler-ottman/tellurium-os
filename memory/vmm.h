#include <arch/terminal.h>
#include <memory/pmm.h>
#include <libc/string.h>

#define VMM_KERNEL_HIGHER_HALF  0xffffffff80000000

// 4 KB page frames
#define PAGE_SIZE 4096

// Page Map Level Entry Flags
#define PML_PRESENT             0x000
#define PML_WRITE               0x002
#define PML_USER                0x004
#define PML_PWT                 0x008
#define PML_PCD                 0x010
#define PML_ACCESSED            0x020
#define PML_PHYSICAL_ADDRESS    0x0000fffffffff000
#define PML_NOT_EXECUTABLE      0x8000000000000000

void init_vmm(void);
uint64_t align_address(uint64_t addr, bool round_up);
uint64_t* allocate_map(uint64_t* map_base, uint64_t map_entry, uint64_t flags);
bool get_next_page_map(uint64_t** new_map_base, uint64_t* map_base, uint64_t map_entry);
void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);
void unmap_page(uint64_t vaddr);