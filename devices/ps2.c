#include <acpi/acpi.h>
#include <arch/idt.h>
#include <devices/ioapic.h>
#include <devices/ps2.h>

void init_ps2() {
    // Arbitrarily choose last APIC ID to send keyboard IRQs
    uint32_t *apic_ids = get_lapic_ids();
    ioapic_map_irq(1, apic_ids[get_core_count() - 1], true, 0, allocate_vector());
}