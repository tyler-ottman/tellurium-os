#include <acpi/acpi.h>
#include <arch/lock.h>
#include <arch/terminal.h>
#include <devices/hpet.h>
#include <memory/vmm.h>
#include <stdbool.h>
#include <stddef.h>

spinlock_t hpet_lock = 0;
static uint64_t *hpet_addr = NULL;

static bool is_hpet_aligned(uint32_t offset) {
    return offset < 0x400 && (offset % 8 == 0);
}

void hpet_write(uint32_t offset, uint64_t value) {
    if (!is_hpet_aligned(offset)) {
        return;
    }
    
    spinlock_acquire(&hpet_lock);
    *((uint64_t*)((uint64_t)hpet_addr + offset + KERNEL_HHDM_OFFSET)) = value;
    spinlock_release(&hpet_lock);
}

uint64_t hpet_read(uint32_t offset) {
    uint64_t reg;
    if (!is_hpet_aligned(offset)) {
        return 0;
    }

    spinlock_acquire(&hpet_lock);
    reg = *((uint64_t*)((uint64_t)hpet_addr + offset + KERNEL_HHDM_OFFSET));
    spinlock_release(&hpet_lock);

    return reg;
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

    uint64_t main_counter = hpet_read(HPET_MAIN_COUNTER);
}