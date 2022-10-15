#include <memory/vmm.h>

// Using linker script to find kernel sections
extern uint8_t _stext[];
extern uint8_t _etext[];
extern uint8_t _srodata[];
extern uint8_t _erodata[];
extern uint8_t _sdata[];
extern uint8_t _edata[];

static volatile struct limine_kernel_address_request kernel_addr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request kernel_hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static uint64_t* pml4_base = NULL;

void init_vmm() {
    struct limine_kernel_address_response* kernel_response = kernel_addr_request.response;
    struct limine_memmap_response* limine_memory_map = memory_map_request.response;
    struct limine_hhdm_response* limine_hhdm = kernel_hhdm_request.response;

    uint64_t* limine_cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r"(limine_cr3));

    // Allocate frame for pml4
    pml4_base = pmm_alloc(1);
    if (!pml4_base) {
        // Todo: throw error if allocation fails
    }

    terminal_printf("VMM: Kernel: Physical Base: %016x, Virtual Base: %016x\n", kernel_response->physical_base, kernel_response->virtual_base);
    terminal_printf("text: 0x%x - 0x%x, rodata: 0x%x - 0x%x\ndata: 0x%x - 0x%x\n", _stext, _etext, _srodata, _erodata, _sdata, _edata);
    
    uint64_t _stext_align = align_address((uint64_t)_stext, false);
    uint64_t _srodata_align = align_address((uint64_t)_srodata, false);
    uint64_t _sdata_align = align_address((uint64_t)_sdata, false);
    uint64_t _etext_align = align_address((uint64_t)_etext, true);
    uint64_t _erodata_align = align_address((uint64_t)_erodata, true);
    uint64_t _edata_align = align_address((uint64_t)_edata, true);

    terminal_printf("Aligned: text: %x - %x, rodata: %x - %x, data: %x - %x\n", _stext_align, _etext_align, _srodata_align, _erodata_align, _sdata_align, _edata_align);

    // Map physical kernel to virtual upper 2 GB of high half
    uint64_t kernel_virtual_base = kernel_response->virtual_base;
    uint64_t kernel_physical_base = kernel_response->physical_base;

    // text section
    for (uint64_t addr = _stext_align; addr < _etext_align; addr += PAGE_SIZE) {
        uint64_t paddr = addr - kernel_virtual_base + kernel_physical_base;
        // terminal_printf("mapping %x to %x\n", addr, paddr);
        map_page(addr, paddr, PML_PRESENT);
    }

    // rodata section
    for (uint64_t addr = _srodata_align; addr < _erodata_align; addr += PAGE_SIZE) {
        uint64_t paddr = addr - kernel_virtual_base + kernel_physical_base;
        map_page(addr, paddr, PML_NOT_EXECUTABLE | PML_PRESENT);
    }

    // data section
    for (uint64_t addr = _sdata_align; addr < _edata_align; addr += PAGE_SIZE) {
        uint64_t paddr = addr - kernel_virtual_base + kernel_physical_base;
        map_page(addr, paddr, PML_NOT_EXECUTABLE | PML_WRITE | PML_PRESENT);
    }

    // identity map of lower 4 GBs
    struct limine_memmap_entry** entries = limine_memory_map->entries;
    uint64_t hhdm = limine_hhdm->offset;
    for (size_t idx = 0; idx < limine_memory_map->entry_count; idx++) {
        struct limine_memmap_entry* entry = entries[idx];

        uint64_t start_paddr = align_address(entry->base, false);
        uint64_t end_paddr = align_address(entry->base + entry->length, true);

        if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            continue; // Already mapped
        }

        for (uint64_t addr = start_paddr; addr < end_paddr; addr += PAGE_SIZE) {
            map_page(addr + hhdm, addr, PML_NOT_EXECUTABLE | PML_WRITE | PML_PRESENT);
            map_page(addr, addr, PML_NOT_EXECUTABLE | PML_WRITE | PML_PRESENT);
        }
    }

    // Hand CR3 to OS
    terminal_printf("Change to OS paging\n");
    // uint64_t cr3_write = (uint64_t)pml4_base;

    print_levels();

    // __asm__ volatile ("movq %0, %%cr3" : "=r"(cr3_write));

    terminal_printf(LIGHT_GREEN "VMM: Initialized\n");
}

uint64_t align_address(uint64_t addr, bool round_up) {
    // Round address up/down to nearest 4096 bytes
    uint64_t rounded_addr = addr / PAGE_SIZE;
    rounded_addr *= PAGE_SIZE;
    if (round_up && (rounded_addr != addr)) {
        rounded_addr += PAGE_SIZE;
    }
    return rounded_addr;
}

uint64_t* allocate_map(uint64_t* map_base, uint64_t map_entry, uint64_t flags) {
    uint64_t* next_level_map = pmm_alloc(1);
    
    if (!next_level_map) {
        return NULL;
    }

    __memset(next_level_map, 0, PAGE_SIZE);
    map_base[map_entry] = ((uint64_t)next_level_map) | flags;
    return (uint64_t*)((uint64_t)next_level_map & PML_PHYSICAL_ADDRESS);
}

bool get_next_page_map(uint64_t** new_map_base, uint64_t* map_base, uint64_t map_entry) {
    uint64_t next_map_entry = map_base[map_entry];

    if (next_map_entry & PML_PRESENT) {
        *new_map_base = (uint64_t*)(next_map_entry & PML_PHYSICAL_ADDRESS);
        return true;
    }

    return false;
}

void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint64_t pml4e = (vaddr >> 39) & 0x1ff;
    uint64_t pdpte = (vaddr >> 30) & 0x1ff;
    uint64_t pde = (vaddr >> 21) & 0x1ff;
    uint64_t pte = (vaddr >> 12) & 0x1ff;
    // terminal_printf("NEW MAPPING: %x -> %x\n", vaddr, paddr);

    uint64_t* pdpt_base = NULL;
    if (!get_next_page_map(&pdpt_base, pml4_base, pml4e)) {
        pdpt_base = allocate_map(pml4_base, pml4e, PML_PRESENT | PML_WRITE | PML_USER);
        // terminal_printf("%x -> %x, pdpt_base: %x\n", vaddr, paddr, pdpt_base);
        if (!pdpt_base) {
            // Out of memory, handle later
        }
    }

    uint64_t* pd_base = NULL;
    if (!get_next_page_map(&pd_base, pdpt_base, pdpte)) {
        pd_base = allocate_map(pdpt_base, pdpte, PML_PRESENT | PML_WRITE | PML_USER);
        // terminal_printf("%x -> %x, pd_base: %x\n", vaddr, paddr, pd_base);
        if (!pd_base) {
            // Out of memory, handle later
        }
    }
    
    uint64_t* pt_base = NULL;
    if (!get_next_page_map(&pt_base, pd_base, pde)) {
        pt_base = allocate_map(pd_base, pde, PML_PRESENT | PML_WRITE | PML_USER);
        // terminal_printf("%x -> %x, pt_base: %x\n", vaddr, paddr, pt_base);
        if (!pt_base) {
            // Out of memory, handle later
        }
    }

    pt_base[pte] = paddr | flags;
}

void unmap_page(uint64_t vaddr) {
    uint64_t pml4e = (vaddr >> 39) & 0x1ff;
    uint64_t pdpte = (vaddr >> 30) & 0x1ff;
    uint64_t pde = (vaddr >> 21) & 0x1ff;
    uint64_t pte = (vaddr >> 12) & 0x1ff;

    uint64_t* pdpt_base = NULL;
    if (!get_next_page_map(&pdpt_base, pml4_base, pml4e)) {
        return;
    }

    uint64_t* pd_base = NULL;
    if (!get_next_page_map(&pd_base, pdpt_base, pdpte)) {
        return;
    }
    
    uint64_t* pt_base = NULL;
    if (!get_next_page_map(&pt_base, pd_base, pde)) {
        return;
    }

    pt_base[pte] = 0;

    // Clear TLB
    __asm__ volatile ("invlpg (%0)" : : "r"(vaddr));
}

void print_levels(void) {
    terminal_printf("pml4_base: %x\n", pml4_base);
    
    // Traverse PML4
    for (size_t idx = 0; idx < 512; idx++) {
        if (pml4_base[idx] & PML_PRESENT) {
            terminal_printf("pml4_base[%d]: %x\n", idx, pml4_base[idx]);

            // Traverse PDPT
            uint64_t* pdpt_base = (uint64_t*)(pml4_base[idx] & PML_PHYSICAL_ADDRESS);
            for (size_t jdx = 0; jdx < 512; jdx++) {
                if (pdpt_base[jdx] & PML_PRESENT) {
                    terminal_printf("  pdpt_base[%d]: %x\n", jdx, pdpt_base[jdx]);

                    uint64_t* pd_base = (uint64_t*)(pdpt_base[jdx] & PML_PHYSICAL_ADDRESS);
                    for (size_t kdx = 0; kdx < 512; kdx++) {
                        if (pd_base[kdx] & PML_PRESENT) {
                            terminal_printf("    pd_base[%d]: %x\n", kdx, pd_base[kdx]);

                            uint64_t* pt_base = (uint64_t*)(pd_base[kdx] & PML_PHYSICAL_ADDRESS);
                            for (size_t ldx = 0; ldx < 512; ldx++) {
                                if (pt_base[ldx] & PML_PRESENT) {
                                    terminal_printf("      pt_base[%d]: %x\n", ldx, pt_base[ldx]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}