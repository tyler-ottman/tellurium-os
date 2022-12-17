#ifndef LAPIC_H
#define LAPIC_H

#include <acpi/acpi.h>
#include <arch/idt.h>
#include <arch/terminal.h>
#include <devices/msr.h>
#include <memory/vmm.h>

#define LOCAL_APIC_ID               0x020
#define LOCAL_APIC_VERSION          0x030
#define LAPIC_EOI                   0x0B0
#define SIV                         0x0F0
#define LVT_TIMER                   0x320
#define DIVIDE_CONFIG               0x3e0
#define LVT_INITIAL_COUNT           0x380
#define LVT_CURRENT_COUNT           0x390

// Spurious Interrupt Vector Registers Masks
#define SIV_ENABLE                  0x00000100

// LVT register masks
#define LVT_MASK                    0x00010000
#define LVT_VECTOR                  0x000000ff

void lapic_time_handler(void);
bool is_lapic_aligned(size_t offset);
uint32_t lapic_read(size_t offset);
void lapic_write(size_t offset, uint32_t val);
void init_lapic(void);
void lapic_lvt_set_vector(uint32_t lvt, uint8_t vector);
void lapic_lvt_enable(uint32_t lvt);
void lapic_lvt_disable(uint32_t lvt);
void lapic_enable(void);
void lapic_disable(void);
void lapic_eoi(void);

#endif // LAPIC_H