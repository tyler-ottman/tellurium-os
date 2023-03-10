#include <acpi/acpi.h>
#include <arch/idt.h>
#include <arch/lock.h>
#include <arch/terminal.h>
#include <devices/ioapic.h>
#include <libc/string.h>

#define IOAPIC_REGSEL               0x00
#define IOAPIC_IOWIN                0x10

#define IOAPIC_ID                   0x00
#define IOAPIC_VER                  0x01
#define IOAPIC_ARB                  0x02
#define IOAPIC_REDTAB               0x10

#define IOAPIC_REDTAB_ENTRY(i)      (IOAPIC_REDTAB + 2 * i)

// Keyboard Interrupts Redirection Table
#define IOAPIC_KEYBOARD_ENTRY       0

typedef struct redtab {
    uint8_t vector: 8;
    uint8_t delivery_mode: 3;
    uint8_t destination_mode: 1;
    uint8_t delivery_status: 1;
    uint8_t interrupt_polarity: 1;
    uint8_t remote_irr: 1;
    uint8_t trigger_mode: 1;
    uint8_t interrupt_mask: 1;
    uint64_t reserved: 39;
    uint8_t destination_field: 8;
}__attribute__ ((__packed__)) redtab_t;

static spinlock_t ioapic_lock = 0;
static uint32_t *ioapic_addr = NULL;

static void ioapic_select_reg(uint8_t offset) {
    *((uint32_t *)((uint64_t)ioapic_addr + IOAPIC_REGSEL)) = offset;
}

void ioapic_write(uint8_t offset, uint32_t val) {
    spinlock_acquire(&ioapic_lock);
    ioapic_select_reg(offset);
    *((uint32_t *)((uint64_t)ioapic_addr + IOAPIC_IOWIN)) = val;
    spinlock_release(&ioapic_lock);
}

uint32_t ioapic_read(uint8_t offset) {
    spinlock_acquire(&ioapic_lock);
    ioapic_select_reg(offset);
    uint64_t reg = *((uint32_t *)((uint64_t)ioapic_addr + IOAPIC_IOWIN));
    spinlock_release(&ioapic_lock);
    return reg;
}

static bool ioapic_read_redtab(int entry, redtab_t *redtab) {
    if (entry >= 24 || entry < 0) {
        return false;
    }

    uint32_t offset = IOAPIC_REDTAB_ENTRY(entry);
    uint64_t reg = ((uint64_t)ioapic_read(offset + 1) << 32) | ioapic_read(offset);

    __memcpy(redtab, &reg, 8);

    return true;
}

static bool ioapic_write_redtab(int entry, redtab_t *redtab) {
    if (entry >= 24 || entry < 0) {
        return false;
    }
    
    uint32_t offset = IOAPIC_REDTAB_ENTRY(entry);
    uint64_t reg = *((uint64_t *)redtab);
    ioapic_write(offset + 1, reg >> 32);
    ioapic_write(offset, (uint32_t)reg);

    return true;
}

void ioapic_map_irq(uint8_t apic_id, bool mask, uint8_t delivery, uint8_t vector) {
    redtab_t redtab_keyboard;

    ioapic_read_redtab(IOAPIC_KEYBOARD_ENTRY, &redtab_keyboard);
    redtab_keyboard.destination_field = apic_id;
    redtab_keyboard.interrupt_mask = mask;
    redtab_keyboard.destination_mode = 0;
    redtab_keyboard.delivery_mode = delivery;
    redtab_keyboard.vector = vector;
    ioapic_write_redtab(IOAPIC_KEYBOARD_ENTRY, &redtab_keyboard);
}

void init_ioapic() {
    ioapic_addr = get_ioapic_addr();
    if (!ioapic_addr) {
        kerror(INFO "IOAPIC: ioapic address undefined\n");
    }

    // Arbitrarily choose last APIC ID to send keyboard IRQs
    uint32_t *apic_ids = get_lapic_ids();
    ioapic_map_irq(apic_ids[get_core_count() - 1], true, 0, allocate_vector());
}