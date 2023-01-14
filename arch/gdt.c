#include <arch/gdt.h>
#include <arch/lock.h>
#include <arch/terminal.h>

GDT_Descriptor gdtr;
GDT gdt;

size_t gdt_index = 0;
static spinlock_t tss_lock = 0;

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

    GDT_Entry user_code64 = {
        .limit_low = 0,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0xfa,
        .flags_limit = 0x20,
        .base_high = 0
    };

    GDT_Entry user_data64 = {
        .limit_low = 0,
        .base_low = 0,
        .base_middle = 0,
        .access_byte = 0xf2,
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
    add_gdt_entry(user_code64);
    add_gdt_entry(user_data64);

    gdtr.size = sizeof(GDT);
    gdtr.base = (uint64_t)&gdt;

    load_gdt();

    kprintf(LIGHT_GREEN "GDT: initialized at: %16x\n", gdtr.base);
}

void add_gdt_entry(GDT_Entry entry) {
    if (gdt_index == GDT_ENTRIES - 1) {
        kerror("GDT: Exceeded max gdt entries\n");
    }

    gdt.gdt_entry[gdt_index++] = entry;
}

void load_tss_entry(struct TSS* tss) {
    spinlock_acquire(&tss_lock);

    uint64_t addr = (uint64_t)tss;
    TSS_Entry tss_entry = {
        .limit = sizeof(struct TSS),
        .base_min = (uint16_t)(addr & 0xffff),
        .base_low = (uint8_t)((addr >> 16) & 0xff),
        .access = 0x89,
        .flags = 0,
        .base_high = (uint8_t)((addr >> 24) & 0xff),
        .base_max = (uint32_t)((addr >> 32) & 0xffffffff),
        .reserved = 0
    };

    gdt.tss_entry = tss_entry;
    uint16_t tss_byte_offset = GDT_ENTRY_SIZE_BYTES * GDT_ENTRIES;

    __asm__ volatile("ltr %%ax" ::"a"(tss_byte_offset));

    spinlock_release(&tss_lock);
}

void load_gdt(void) {
    __asm__ volatile ("lgdt %0" :: "m" (gdtr));
}
