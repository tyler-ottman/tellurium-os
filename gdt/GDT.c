#include "GDT.h"

// https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md
void load_gdt() {
    gdtr.base = (uint64_t)&gdtr;
    gdtr.size = sizeof(GDT) - 1;

    // Null Descriptor
    gdt.gdt_entry[0].limit_low = 0;
    gdt.gdt_entry[0].base_low = 0;
    gdt.gdt_entry[0].base_middle = 0;
    gdt.gdt_entry[0].access_byte = 0;
    gdt.gdt_entry[0].flags_limit = 0;
    gdt.gdt_entry[0].base_high = 0;

    // 16-bit kernel code
    gdt.gdt_entry[1].limit_low = 0xffff;
    gdt.gdt_entry[1].base_low = 0;
    gdt.gdt_entry[1].base_middle = 0;
    gdt.gdt_entry[1].access_byte = 154;
    gdt.gdt_entry[1].flags_limit = 0;
    gdt.gdt_entry[1].base_high = 0;

    // 16-bit kernel data
    gdt.gdt_entry[2].limit_low = 0xffff;
    gdt.gdt_entry[2].base_low = 0;
    gdt.gdt_entry[2].base_middle = 0;
    gdt.gdt_entry[2].access_byte = 146;
    gdt.gdt_entry[2].flags_limit = 0;
    gdt.gdt_entry[2].base_high = 0;

    // 32-bit kernel code
    gdt.gdt_entry[3].limit_low = 0xffff;
    gdt.gdt_entry[3].base_low = 0;
    gdt.gdt_entry[3].base_middle = 0;
    gdt.gdt_entry[3].access_byte = 154;
    gdt.gdt_entry[3].flags_limit = 207;
    gdt.gdt_entry[3].base_high = 0;

    // 32-bit kernel data
    gdt.gdt_entry[4].limit_low = 0xffff;
    gdt.gdt_entry[4].base_low = 0;
    gdt.gdt_entry[4].base_middle = 0;
    gdt.gdt_entry[4].access_byte = 146;
    gdt.gdt_entry[4].flags_limit = 207;
    gdt.gdt_entry[4].base_high = 0;

    // 64-bit kernel code
    gdt.gdt_entry[5].limit_low = 0;
    gdt.gdt_entry[5].base_low = 0;
    gdt.gdt_entry[5].base_middle = 0;
    gdt.gdt_entry[5].access_byte = 154;
    gdt.gdt_entry[5].flags_limit = 32;
    gdt.gdt_entry[5].base_high = 0;

    // 64-bit kernel data
    gdt.gdt_entry[6].limit_low = 0;
    gdt.gdt_entry[6].base_low = 0;
    gdt.gdt_entry[6].base_middle = 0;
    gdt.gdt_entry[6].access_byte = 146;
    gdt.gdt_entry[6].flags_limit = 0;
    gdt.gdt_entry[6].base_high = 0;

    // Load gdt
    setGdt();
}

void setGdt(void) {
    gdtr.size = sizeof(GDT);
    gdtr.base = (uint64_t)&gdt;
    __asm__ volatile ("lgdt %0" :: "m" (gdtr));
}