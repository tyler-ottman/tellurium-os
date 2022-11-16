#include <memory/vmm.h>

extern int breakpoint(void);

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
size_t frames_allocated = 0;

void map_section(uint64_t vaddr_base, uint64_t paddr_base, uint64_t len, uint64_t flags) {
    uint64_t paddr = paddr_base;
    for (uint64_t vaddr = vaddr_base; vaddr < vaddr_base + len; vaddr += PAGE_SIZE) {
        map_page(vaddr, paddr, flags);
        paddr += PAGE_SIZE;
    }
}

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

    uint64_t kernel_vaddr = kernel_response->virtual_base;
    uint64_t kernel_paddr = kernel_response->physical_base;

    // map .text section
    uint64_t stext_vaddr = align_address((uint64_t)_stext, false);
    uint64_t etext_vaddr = align_address((uint64_t)_etext, true);
    uint64_t stext_paddr = stext_vaddr - kernel_vaddr + kernel_paddr;
    uint64_t text_len = etext_vaddr - stext_vaddr;
    map_section(stext_vaddr, stext_paddr, text_len, PML_PRESENT);

    // map .rodata section
    uint64_t srodata_vaddr = align_address((uint64_t)_srodata, false);
    uint64_t erodata_vaddr = align_address((uint64_t)_erodata, true);
    uint64_t srodata_paddr = srodata_vaddr - kernel_vaddr + kernel_paddr;
    uint64_t rodata_len = erodata_vaddr - srodata_vaddr;
    map_section(srodata_vaddr, srodata_paddr, rodata_len, PML_NOT_EXECUTABLE | PML_PRESENT);

    // map .data section
    uint64_t sdata_vaddr = align_address((uint64_t)_sdata, false);
    uint64_t edata_vaddr = align_address((uint64_t)_edata, true);
    uint64_t sdata_paddr = sdata_vaddr - kernel_vaddr + kernel_paddr;
    uint64_t data_len = edata_vaddr - sdata_vaddr;
    map_section(sdata_vaddr, sdata_paddr, data_len, PML_NOT_EXECUTABLE | PML_WRITE | PML_PRESENT);

    kprintf(".text: %016x - %016x, .rodata: %016x - %016x\n.data: %016x - %016x\n", stext_vaddr, etext_vaddr, srodata_vaddr, erodata_vaddr, sdata_vaddr, edata_vaddr);

    // identity map of lower 4 GBs
    struct limine_memmap_entry** entries = limine_memory_map->entries;
    uint64_t hhdm = limine_hhdm->offset;
    for (size_t idx = 0; idx < limine_memory_map->entry_count; idx++) {
        struct limine_memmap_entry* entry = entries[idx];

        uint64_t start_paddr = align_address(entry->base, false);
        uint64_t end_paddr = align_address(entry->base + entry->length, true);
        uint64_t len = end_paddr - start_paddr;

        map_section(start_paddr + hhdm, start_paddr, len, PML_NOT_EXECUTABLE | PML_WRITE | PML_PRESENT);
        map_section(start_paddr, start_paddr, len, PML_WRITE | PML_PRESENT);
    }
    
    // Hand over paging to OS
    uint64_t cr3_write = (uint64_t)pml4_base;
    __asm__ volatile ("mov %0, %%cr3" : : "r"(cr3_write));
    // print_levels(pml4_base, 2);

    // Todo: Destroy Limine's page tables

    kprintf("VMM: tellurium-os using %d frames\n", frames_allocated);
    kprintf(LIGHT_GREEN "VMM: Initialized\n");
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

    frames_allocated++;
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
    // kprintf("NEW MAPPING: %x -> %x\n", vaddr, paddr);

    uint64_t* pdpt_base = NULL;
    if (!get_next_page_map(&pdpt_base, pml4_base, pml4e)) {
        pdpt_base = allocate_map(pml4_base, pml4e, PML_PRESENT | PML_WRITE | PML_USER);
        // kprintf("%x -> %x, pdpt_base: %x\n", vaddr, paddr, pdpt_base);
        if (!pdpt_base) {
            // Out of memory, handle later
        }
    }

    uint64_t* pd_base = NULL;
    if (!get_next_page_map(&pd_base, pdpt_base, pdpte)) {
        pd_base = allocate_map(pdpt_base, pdpte, PML_PRESENT | PML_WRITE | PML_USER);
        // kprintf("%x -> %x, pd_base: %x\n", vaddr, paddr, pd_base);
        if (!pd_base) {
            // Out of memory, handle later
        }
    }
    
    uint64_t* pt_base = NULL;
    if (!get_next_page_map(&pt_base, pd_base, pde)) {
        pt_base = allocate_map(pd_base, pde, PML_PRESENT | PML_WRITE | PML_USER);
        // kprintf("%x -> %x, pt_base: %x\n", vaddr, paddr, pt_base);
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

uint64_t get_virt_addr(size_t pml4e, size_t pdpte, size_t pde, size_t pte, size_t page_offset) {
    uint64_t virt = 0;
    virt |= (pml4e << 39);
    virt |= (pdpte << 30);
    virt |= (pde << 21);
    virt |= (pte << 12);
    virt |= (page_offset);

    if ((pml4e >> 8) & 1) {
        virt |= 0xffff000000000000;
    }

    return virt;
}

const char* map_names[] = {"pml4_base", "pdpt_base", "pd_base", "pt_base"};
void recursive_level_print(uint64_t* base, size_t lvls_remaining, size_t depth) {
    if (!lvls_remaining) return;
    
    for (size_t idx = 0; idx < PAGE_ENTRIES; idx++) {
        if (base[idx] & PML_PRESENT) {
            for (size_t jdx = 0; jdx < depth; jdx++) kprintf("\t");
            kprintf("%s[%d]: %016x\n", map_names[depth], idx, base[idx]);

            uint64_t* map_base = (uint64_t*)(base[idx] & PML_PHYSICAL_ADDRESS);
            recursive_level_print(map_base, lvls_remaining - 1, depth + 1);
        }
    }
}

void print_levels(uint64_t* base, uint64_t num_levels) {
    if (num_levels > 4) return;
    recursive_level_print(base, num_levels, 0);
}