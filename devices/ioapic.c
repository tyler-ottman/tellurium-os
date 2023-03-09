#include <acpi/acpi.h>
#include <arch/terminal.h>
#include <devices/ioapic.h>

static uint32_t *ioapic_addr = NULL;

void init_ioapic() {
    ioapic_addr = get_ioapic_addr();
    kprintf("ioapic: %x\n", ioapic_addr);
}