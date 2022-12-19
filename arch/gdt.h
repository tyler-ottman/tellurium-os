#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include <arch/terminal.h>
#include <libc/string.h>

#define GDT_ENTRIES             8
#define GDT_KERNEL_CODE         0x28
#define GDT_KERNEL_DATA         0x30

#define GDT_ENTRY_SIZE_BYTES    8

typedef struct {
    uint16_t size; // Size of GDT
    uint64_t base; // Linear address of GDT
}__attribute__((packed)) GDT_Descriptor;

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t flags_limit;
    uint8_t base_high;
}__attribute__((packed)) GDT_Entry;

typedef struct {
    uint16_t limit;
    uint16_t base_min;
    uint8_t base_low;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
    uint32_t base_max;
    uint32_t reserved;
}__attribute__((packed)) TSS_Entry;

struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base_addr;
}__attribute__((packed));

typedef struct {
    GDT_Entry gdt_entry[GDT_ENTRIES];
    TSS_Entry tss_entry;
} GDT;

void init_gdt(void);
void add_gdt_entry(GDT_Entry entry);
void load_tss_entry(struct TSS* tss);
void load_gdt(void);

#endif // GDT_H
