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

void lapic_time_handler() {
    kprintf(MAGENTA"Interrupt detected\n");
    done();
}

extern void* ISR_Timer_Interrupt;
void init_lapic() {
    lapic_addr = get_lapic_addr();
    kprintf("LAPIC: Addr: %x\n", lapic_addr);

    uint64_t map_addr = (uint64_t)lapic_addr;
    map_section(map_addr, map_addr, 4096, PML_NOT_EXECUTABLE | PML_WRITE | PML_PRESENT);

    // Software enable local APIC
    uint32_t spurious_reg = read_lapic_reg(SPURIOUS_INTERRUPT_VECTOR);
    uint8_t lapic_vector = allocate_vector();
    spurious_reg |= (1 << 8);
    spurious_reg &= ((0xffffff00) | lapic_vector);
    write_lapic_reg(SPURIOUS_INTERRUPT_VECTOR, spurious_reg);
    add_descriptor(lapic_vector, ISR_Timer_Interrupt, 0x8e);

    kprintf("Reg: %x\n", read_lapic_reg(SPURIOUS_INTERRUPT_VECTOR));
    kprintf(LIGHT_GREEN "LAPIC: Initialized\n");
}