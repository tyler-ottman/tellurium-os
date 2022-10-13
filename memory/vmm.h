#include <arch/terminal.h>
#include <memory/pmm.h>
#include <libc/string.h>

#define VMM_DEBUG

// 4 KB page frames
#define PAGE_SIZE 4096

// Page Map Level Entry Flags
#define PML_PRESENT             0x000
#define PML_READ_WRITE          0x001
#define PML_USER_SUPERVISOR     0x002
#define PML_PWT                 0x003
#define PML_PCD                 0x004
#define PML_ACCESSED            0x005
#define PML_PHYSICAL_ADDRESS    0x0000fffffffff000
#define PML_EXECUTE             0x8000000000000000

void init_vmm(void);

uint64_t* get_next_page_map(uint64_t* map_base, uint64_t map_entry, bool read_only);
void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);