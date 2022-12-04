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

void enable_interrupts() {
    __asm__ volatile ("sti");
}

extern void* ISR_Timer_Interrupt[];
void init_lapic() {
    lapic_addr = get_lapic_addr();

    // Software enable local APIC
    uint32_t spurious_reg = read_lapic_reg(SPURIOUS_INTERRUPT_VECTOR);
    uint8_t lapic_vector = allocate_vector();
    spurious_reg |= (1 << 8);
    spurious_reg &= ((0xffffff00) | lapic_vector);
    write_lapic_reg(SPURIOUS_INTERRUPT_VECTOR, spurious_reg);
    
    kprintf("IST handler addr: %016x\n", ISR_Timer_Interrupt);
    add_descriptor(lapic_vector, ISR_Timer_Interrupt, 0x8e);
    write_lapic_reg(LVT_TIMER, read_lapic_reg(LVT_TIMER) | lapic_vector);
    write_lapic_reg(LVT_TIMER, read_lapic_reg(LVT_TIMER) & ~(LVT_TIMER_MASK_BIT));
    write_lapic_reg(LVT_INITIAL_COUNT, 0x1fffffff);
    
    enable_interrupts();
    
    // kprintf("Reg: %x\n", read_lapic_reg(SPURIOUS_INTERRUPT_VECTOR));
    // kprintf(LIGHT_GREEN "LAPIC: Initialized\n");
}
