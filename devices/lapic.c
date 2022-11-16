#include <devices/lapic.h>

uint32_t* lapic_addr;

bool is_lapic_aligned(size_t offset) {
    return offset < 0x400 || (offset % 16 == 0);
}

uint32_t read_lapic_reg(size_t offset) {
    if (!is_lapic_aligned(offset)) {
        kerror(UNALIGNED_LAPIC);
    }

    return *((uint32_t*)((uint64_t)lapic_addr + offset));
}

void write_lapic_reg(size_t offset, uint32_t val) {
    if  (!is_lapic_aligned(offset)) {
        kerror(UNALIGNED_LAPIC);
    }

    *((uint32_t*)((uint64_t)lapic_addr + offset)) = val;
}

void init_lapic() {
    lapic_addr = get_lapic_addr();
    
    kprintf("LAPIC: Addr: %x\n", lapic_addr);
    kprintf(LIGHT_GREEN "LAPIC: Initialized\n");
}