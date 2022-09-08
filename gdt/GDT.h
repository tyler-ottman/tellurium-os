#include <stdint.h>

#ifndef GDT_H
#define GDT_H

#define GDT_KERNEL_CODE 0x28

// https://wiki.osdev.org/Global_Descriptor_Table

// GDT Descriptor (GDTR)
typedef struct {
    uint16_t size; // Size of GDT
    uint64_t base; // Linear address of GDT
}__attribute__((packed)) GDT_Descriptor;

// GDT Entry
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t flags_limit;
    uint8_t base_high;
}__attribute__((packed)) GDT_Entry;

typedef struct {
    GDT_Entry gdt_entry[7];
} GDT;

void init_gdt(void);
void setGdt(void);

#endif // GDT_H