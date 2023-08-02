#include <acpi/acpi.h>
#include <arch/cpu.h>
#include <arch/idt.h>
#include <arch/kterminal.h>
#include <arch/lock.h>
#include <devices/ioapic.h>
#include <flibc/string.h>
#include <sys/misc.h>

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

typedef struct ioapic_version {
    uint8_t apic_version: 8;
    uint8_t reserved0: 8;
    uint8_t max_red_entry: 8;
    uint8_t reserved1: 8;
}__attribute__ ((__packed__)) ioapic_version_t;

static spinlock_t ioapic_lock = 0;
// static uint32_t *ioapic_addr = NULL;

static uint32_t *ioapic_get_addr(size_t irq) {
    int gsi = acpi_irq_to_gsi(irq); // Get corresponding gsi from IRQ
    
    struct MADT *madt = get_madt();
    if (!madt) {
        return NULL;
    }

    io_apic_t **ioapics = acpi_get_ioapics();
    for (size_t i = 0; i < acpi_get_num_ioapics(); i++) {
        io_apic_t *ioapic = ioapics[i];
        uint32_t *ioapic_addr = (uint32_t *)(uint64_t)ioapic->io_apic_address;
        uint32_t ioapic_ver = ioapic_read(ioapic_addr, IOAPIC_VER);
        ioapic_version_t *ver = (ioapic_version_t *)&ioapic_ver;

        // Desired GSI resides in selected I/O APIC
        int gsi_b = ioapic->global_sys_interrupt_base;
        int gsi_max = ver->max_red_entry;
        if (gsi >= gsi_b && gsi <= gsi_max) {
            return ioapic_addr;
        }
    }

    return NULL;
}

static void ioapic_select_reg(uint32_t *ioapic_addr, uint8_t offset) {
    uint64_t base = (uint64_t)ioapic_addr + IOAPIC_REGSEL;
    *((volatile uint32_t *)base) = offset;
}

void ioapic_write(uint32_t *ioapic_addr, uint8_t offset, uint32_t val) {
    spinlock_acquire(&ioapic_lock);
    
    ioapic_select_reg(ioapic_addr, offset);
    uint64_t base = (uint64_t)ioapic_addr + IOAPIC_IOWIN;
    *((volatile uint32_t *)base) = val;

    spinlock_release(&ioapic_lock);
}

uint32_t ioapic_read(uint32_t *ioapic_addr, uint8_t offset) {
    spinlock_acquire(&ioapic_lock);
    
    ioapic_select_reg(ioapic_addr, offset);
    uint64_t base = (uint64_t)ioapic_addr + IOAPIC_IOWIN;
    uint32_t data = *((volatile uint32_t *)base);
    
    spinlock_release(&ioapic_lock);
    return data;
}

static bool ioapic_read_redtab(uint32_t *ioapic_addr, int entry, redtab_t *redtab) {
    if (entry >= 24 || entry < 0) {
        return false;
    }

    uint32_t offset = IOAPIC_REDTAB_ENTRY(entry);
    uint32_t low = ioapic_read(ioapic_addr, offset);
    uint32_t high = ioapic_read(ioapic_addr, offset + 1);
    uint64_t reg = ((uint64_t)high << 32) | low;

    __memcpy(redtab, &reg, 8);

    return true;
}

static bool ioapic_write_redtab(uint32_t *ioapic_addr, int entry, redtab_t *redtab) {
    if (entry >= 24 || entry < 0) {
        return false;
    }
    
    uint32_t offset = IOAPIC_REDTAB_ENTRY(entry);
    uint64_t reg = *((uint64_t *)redtab);
    ioapic_write(ioapic_addr, offset + 1, reg >> 32);
    ioapic_write(ioapic_addr, offset, (uint32_t)reg);

    return true;
}

void ioapic_map_irq(uint32_t irq, uint8_t apic_id, bool mask, uint8_t delivery, uint8_t vector, void *gate_entry, uint8_t flags) {
    // Get I/O APIC address legacy irq
    uint32_t *ioapic_addr = ioapic_get_addr(irq);
    ASSERT(ioapic_addr, 0, "IOAPIC: ioapic not found\n");

    // Redirect IRQ to IDT vector
    redtab_t redtab_keyboard;
    int irq_redtab_entry = acpi_irq_to_gsi(irq) - acpi_get_gsi_base(ioapic_addr);
    ioapic_read_redtab(ioapic_addr, irq_redtab_entry, &redtab_keyboard);
    redtab_keyboard.destination_field = apic_id;
    redtab_keyboard.interrupt_mask = mask;
    redtab_keyboard.destination_mode = 0;
    redtab_keyboard.delivery_mode = delivery;
    redtab_keyboard.vector = vector;
    ioapic_write_redtab(ioapic_addr, irq_redtab_entry, &redtab_keyboard);

    add_descriptor(vector, gate_entry, flags);
    set_vector_ist(vector, 1);
}