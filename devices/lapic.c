#include <devices/lapic.h>

uint32_t* lapic_addr;

bool is_lapic_aligned(size_t offset) {
    return offset < 0x400 || (offset % 16 == 0);
}

uint32_t lapic_read(size_t offset) {
    if (!is_lapic_aligned(offset)) {
        kerror(UNALIGNED_LAPIC);
    }

    return *((uint32_t*)((uint64_t)lapic_addr + offset));
}

void lapic_write(size_t offset, uint32_t val) {
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

void disable_interrupts() {
    __asm__ volatile ("cli");
}

extern void* ISR_Timer_Interrupt[];
void init_lapic() {
    lapic_addr = get_lapic_addr();

    // Software enable local APIC
    lapic_enable();
    
    // Add IDT entry for timer interrupts
    uint8_t lapic_vector = allocate_vector();
    add_descriptor(lapic_vector, ISR_Timer_Interrupt, 0x8e);
    lapic_lvt_set_vector(LVT_TIMER, lapic_vector);

    // Enable reception of timer interrupt
    lapic_lvt_enable(LVT_TIMER);
    
    lapic_write(LVT_INITIAL_COUNT, 0x30000000);
    enable_interrupts();
}

void lapic_lvt_set_vector(uint32_t lvt, uint8_t vector) {
    lapic_write(lvt, (lapic_read(lvt) & ~(LVT_VECTOR)) | vector);
}

void lapic_lvt_enable(uint32_t lvt) {
    lapic_write(lvt, lapic_read(lvt) & ~(LVT_MASK));
}

void lapic_lvt_disable(uint32_t lvt) {
    lapic_write(lvt, lapic_read(lvt) | LVT_MASK);
}

void lapic_enable() {
    lapic_write(SIV, lapic_read(SIV) | SIV_ENABLE);
}

void lapic_disable() {
    lapic_write(SIV, lapic_read(SIV) & ~(SIV_ENABLE));
}
