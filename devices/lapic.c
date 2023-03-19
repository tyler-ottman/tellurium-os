#include <arch/lock.h>
#include <devices/hpet.h>
#include <devices/lapic.h>
#include <arch/scheduler.h>
#include <arch/cpu.h>

extern void* ISR_Timer[];
extern void* ISR_IPI[];

uint32_t* lapic_addr;
static spinlock_t calibrate_lock = 0;

bool is_lapic_aligned(size_t offset) {
    return offset < 0x400 || (offset % 16 == 0);
}

uint32_t lapic_read(size_t offset) {
    if (!is_lapic_aligned(offset)) {
        kerror(UNALIGNED_LAPIC);
    }

    return *((uint32_t*)((uint64_t)lapic_addr + offset + KERNEL_HHDM_OFFSET));
}

void lapic_write(size_t offset, uint32_t val) {
    if  (!is_lapic_aligned(offset)) {
        kerror(UNALIGNED_LAPIC);
    }

    *((uint32_t*)((uint64_t)lapic_addr + offset + KERNEL_HHDM_OFFSET)) = val;
}

void lapic_time_handler(ctx_t* ctx) {
    lapic_eoi();

    struct core_local_info* cpu_info = get_core_local_info();
    thread_t* current_thread = cpu_info->current_thread;
    if (current_thread) {
        __memcpy(&current_thread->context, ctx, sizeof(ctx_t));
    }
    
    // kprintf(MAGENTA"Timer Handle\n");

    schedule_next_thread();
}

void lapic_ipi_handler(ctx_t* ctx) {
    lapic_eoi();

    struct core_local_info* cpu_info = get_core_local_info();
    thread_t* current_thread = cpu_info->current_thread;
    if (current_thread) {
        __memcpy(&current_thread->context, ctx, sizeof(ctx_t));
    }
    
    kprintf(MAGENTA"IPI Handle\n");

    schedule_next_thread();
}

void lapic_calibrate(bool hpet_present) {
    spinlock_acquire(&calibrate_lock);

    size_t lapic_samples = 0xffffff;
    if (hpet_present) {
        hpet_timer_disable();
        hpet_set_timer(0);

        lapic_write(LVT_INITIAL_COUNT, lapic_samples);
        hpet_timer_enable();
        while (lapic_read(LVT_CURRENT_COUNT) != 0) {}

        hpet_timer_disable();

        uint64_t hpet_period_fs = hpet_get_period();
        uint64_t hpet_samples = hpet_get_timer();

        uint64_t lapic_period = (hpet_samples * hpet_period_fs) / lapic_samples;
        uint64_t lapic_freq = FEMTO / lapic_period;

        get_core_local_info()->lapic_freq = lapic_freq;
        // kprintf("lapic freq: %d\n", lapic_freq);
    } else { // PIT

    }

    spinlock_release(&calibrate_lock);
}

void init_lapic() {
    struct core_local_info* cpu_info = get_core_local_info();
    
    lapic_addr = get_lapic_addr();
    
    // Software enable local APIC
    lapic_enable();
    // enable_interrupts();
    
    // Add IDT entry for timer interrupts
    uint8_t timer_vector = allocate_vector();
    cpu_info->lapic_timer_vector = timer_vector;
    
    add_descriptor(timer_vector, ISR_Timer, 0x8e);
    lapic_lvt_set_vector(LVT_TIMER, timer_vector);

    lapic_calibrate(acpi_hpet_present());

    // Enable reception of timer interrupt
    lapic_lvt_enable(LVT_TIMER);

    uint8_t ipi_vector = allocate_vector();
    add_descriptor(ipi_vector, ISR_IPI, 0x8e);
    cpu_info->lapic_ipi_vector = ipi_vector;
}

void lapic_schedule_time(uint64_t us) {
    uint64_t ticks = us * (get_core_local_info()->lapic_freq / 1000000);
    lapic_write(LVT_INITIAL_COUNT, ticks);
    enable_interrupts();
}

void lapic_send_ipi(uint32_t lapic_id, uint32_t vector) {
    lapic_write(LAPIC_ICR1, lapic_id << 24);
    lapic_write(LAPIC_ICR0, vector); // Writing to lower half invokes IPI
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

void lapic_eoi() {
    lapic_write(LAPIC_EOI, 0);
}
