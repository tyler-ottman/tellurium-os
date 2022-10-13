#include <memory/vmm.h>

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

void init_vmm() {
    struct limine_kernel_address_response* kernel_response = kernel_addr_request.response;
    uint64_t* limine_cr3;
    __asm__ volatile ("movq %%cr3, %0" : "=r"(limine_cr3));

#ifdef VMM_DEBUG // print vmm debug info
    terminal_printf("VMM: Kernel: Physical Base: %016x, Virtual Base: %016x\n", kernel_response->physical_base, kernel_response->virtual_base);
    terminal_printf("text: 0x%x - 0x%x, rodata: 0x%x - 0x%x\ndata: 0x%x - 0x%x\n", _stext, _etext, _srodata, _erodata, _sdata, _edata);

    // Print limine PML4
    terminal_printf("Address of Limine PML4: %016x\n", limine_cr3);
    // Traverse PML4
    
#endif // VMM_DEBUG

    // Allocate PML4
    // uint8_t* cr3_pml4 = (uint8_t*)page_alloc(1);
    // __memset(cr3_pml4, 0x00, PAGE_SIZE);

    // terminal_printf("VMM: cr3 base: %016x\n", cr3_pml4);

    // uint64_t cr3_write = 0;
    // __asm__ volatile ("movq %0, %%cr3" : "=r"(cr3_write));
    terminal_printf(LIGHT_GREEN "VMM: Initialized\n");
}

uint64_t* get_next_page_map(uint64_t* map_base, uint64_t map_entry, bool read_only) {
    uint64_t* next_map_addr = map_base[map_entry];

    if (next_map_addr != NULL) {
        return (uint64_t*)((uint64_t)next_map_addr & PML_PHYSICAL_ADDRESS);
    }

    if (read_only) {
        return NULL;
    }

    void* map = pmm_alloc(1);
}

void map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags) {

    // Gets address for every level
    uint64_t pml4e = (vaddr >> 39) & 0x1ff;
    uint64_t pdpte = (vaddr >> 30) & 0x1ff;
    uint64_t pde = (vaddr >> 21) & 0x1ff;
    uint64_t pte = (vaddr >> 12) & 0x1ff;


}