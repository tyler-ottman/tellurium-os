#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <arch/gdt.h>
#include <arch/terminal.h>

// https://wiki.osdev.org/Interrupt_Descriptor_Table

// IDT Descriptor (IDTR)
typedef struct {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed)) IDT_Descriptor;

// IDT Entry
typedef struct {
    uint16_t offset_low16;
    uint16_t segment_selector;
    uint8_t ist;
    uint8_t gate_type;
    uint16_t offset_middle16;
    uint32_t offset_upper32;
    uint32_t reserved;
}__attribute__((packed)) IDT_Entry;

uint8_t allocate_vector(void);
void add_descriptor(uint8_t, void*, uint8_t);
void set_vector_ist(uint8_t vector, int ist);
void idt_load(void);
void init_idt(void);

#endif // IDT_H