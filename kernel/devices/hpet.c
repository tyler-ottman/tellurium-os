#include <acpi/acpi.h>
#include <arch/lock.h>
#include <arch/terminal.h>
#include <devices/hpet.h>
#include <memory/vmm.h>
#include <stdbool.h>
#include <stddef.h>

spinlock_t hpet_lock = 0;
static uint64_t *hpet_addr = NULL;

static void hpet_write(uint32_t offset, uint64_t value);
static uint64_t hpet_read(uint32_t offset);

static bool is_hpet_aligned(uint32_t offset) {
    return offset < 0x400 && (offset % 8 == 0);
}

void hpet_write(uint32_t offset, uint64_t value) {
    if (!is_hpet_aligned(offset)) {
        return;
    }
    
    spinlock_acquire(&hpet_lock);
    *((volatile uint64_t *)((uint64_t)hpet_addr + offset + KERNEL_HHDM_OFFSET)) = value;
    spinlock_release(&hpet_lock);
}

uint64_t hpet_read(uint32_t offset) {
    uint64_t reg;
    if (!is_hpet_aligned(offset)) {
        return 0;
    }

    spinlock_acquire(&hpet_lock);
    reg = *((volatile uint64_t *)((uint64_t)hpet_addr + offset + KERNEL_HHDM_OFFSET));
    spinlock_release(&hpet_lock);

    return reg;
}

uint32_t hpet_get_period() {
    return (hpet_read(HPET_GENERAL_CAP_AND_ID) >> 32);
}

uint32_t hpet_get_timer() {
    return hpet_read(HPET_MAIN_COUNTER);
}

void hpet_set_timer(uint64_t val) {
    bool timer_enabled = hpet_read(HPET_GENERAL_CONFIG) & 1;
    if (timer_enabled) {
        hpet_timer_disable();
    }

    hpet_write(HPET_MAIN_COUNTER, val);

    if (timer_enabled) {
        hpet_timer_enable();
    }
}

void hpet_timer_enable() {
    hpet_write(HPET_GENERAL_CONFIG, hpet_read(HPET_GENERAL_CONFIG) | 1);
}

void hpet_timer_disable() {
    hpet_write(HPET_GENERAL_CONFIG, hpet_read(HPET_GENERAL_CONFIG) & ~(1));
}

void init_hpet() {
    if (!acpi_hpet_present()) {
        return;
    }

    hpet_addr = (uint64_t *)get_hpet_addr();

    kprintf(INFO GREEN "HPET_CLK_PERIOD: %d fs\n", hpet_get_period());    
}