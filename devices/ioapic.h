#ifndef IOAPIC_H
#define IOAPIC_H

#include <stdint.h>

void ioapic_write(uint32_t *ioapic_addr, uint8_t offset, uint32_t val);
uint32_t ioapic_read(uint32_t *ioapic_addr, uint8_t offset);
void ioapic_map_irq(uint32_t irq, uint8_t apic_id, bool mask, uint8_t delivery, uint8_t vector);

#endif // IOAPIC_H