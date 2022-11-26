#include <arch/gdt.h>

GDT_Descriptor gdtr;
GDT gdt;

size_t gdt_index = 0;

// https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md
void init_gdt() {
    GDT_Entry null_descriptor = {
        .limit_low = 0,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0,
        .flags_limit = 0,
        .base_high = 0
    };

    GDT_Entry kernel_code16 = {
        .limit_low = 0xffff,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0x9a,
        .flags_limit = 0,
        .base_high = 0
    };

    GDT_Entry kernel_data16 = {
        .limit_low = 0xffff,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0x92,
        .flags_limit = 0,
        .base_high = 0
    };

    GDT_Entry kernel_code32 = {
        .limit_low = 0xffff,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0x9a,
        .flags_limit = 0xcf,
        .base_high = 0
    };

    GDT_Entry kernel_data32 = {
        .limit_low = 0xffff,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0x92,
        .flags_limit = 0xcf,
        .base_high = 0
    };

    GDT_Entry kernel_code64 = {
        .limit_low = 0,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0x9a,
        .flags_limit = 0x20,
        .base_high = 0
    };

    GDT_Entry kernel_data64 = {
        .limit_low = 0,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0x92,
        .flags_limit = 0,
        .base_high = 0
    };

    add_gdt_entry(null_descriptor);
    add_gdt_entry(kernel_code16);
    add_gdt_entry(kernel_data16);
    add_gdt_entry(kernel_code32);
    add_gdt_entry(kernel_data32);
    add_gdt_entry(kernel_code64);
    add_gdt_entry(kernel_data64);

    gdtr.size = sizeof(GDT);
    gdtr.base = (uint64_t)&gdt;

    load_gdt();

    kprintf(LIGHT_GREEN"GDT: initialized at: %16x\n", gdtr.base);
}

void add_gdt_entry(GDT_Entry entry) {
    gdt.gdt_entry[gdt_index++] = entry;
}

void load_gdt(void) {
    __asm__ volatile ("lgdt %0" :: "m" (gdtr));
}