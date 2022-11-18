#ifndef LAPIC_H
#define LAPIC_H

#include <acpi/acpi.h>
#include <arch/idt.h>
#include <arch/terminal.h>
#include <devices/msr.h>
#include <memory/vmm.h>

#define LOCAL_APIC_ID               0x020
#define LOCAL_APIC_VERSION          0x030
#define EOI                         0x0B0
#define SPURIOUS_INTERRUPT_VECTOR   0x0F0
#define LVT_TIMER                   0x320

bool is_lapic_aligned(size_t offset);
uint32_t read_lapic_reg(size_t offset);
void write_lapic_reg(size_t offset, uint32_t val);
void lapic_time_handler(void);
void init_lapic(void);

#endif // LAPIC_H