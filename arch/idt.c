#include <arch/lock.h>
#include <arch/idt.h>

extern void* isr_table[];

static IDT_Descriptor idtr;
static spinlock_t idt_lock = 0;
size_t cur_vector_idt;

__attribute__((aligned(0x10))) static IDT_Entry idt_entry[256];

uint8_t allocate_vector() {
    if (cur_vector_idt >= 256) {
        kerror("IDT: Exceeded available IDT entries\n");
    }

    return cur_vector_idt++;
}

void add_descriptor(uint8_t vector, void* gate_entry, uint8_t flags) {
    spinlock_acquire(&idt_lock);
    
    IDT_Entry* descriptor_ptr = &idt_entry[vector];

    uint64_t entry_addr = (uint64_t)gate_entry;

    descriptor_ptr->offset_low16 = entry_addr & 0xffff;
    descriptor_ptr->segment_selector = GDT_KERNEL_CODE;
    descriptor_ptr->ist = 0;
    descriptor_ptr->gate_type = flags;
    descriptor_ptr->offset_middle16 = (entry_addr >> 16) & 0xffff;
    descriptor_ptr->offset_upper32 = (entry_addr >> 32) & 0xffffffff;
    descriptor_ptr->reserved = 0;

    idt_load();

    spinlock_release(&idt_lock);
}

void set_vector_ist(uint8_t vector, int ist) {
    spinlock_acquire(&idt_lock);

    IDT_Entry* idt_descriptor = &idt_entry[vector];

    idt_descriptor->ist = ist;

    idt_load();

    spinlock_release(&idt_lock);
}

void idt_load() {
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

void init_idt(void) {
    idtr.offset = (uint64_t)&idt_entry[0];
    idtr.size = 256 * sizeof(IDT_Entry) - 1;

    // Load IDT entries
    for (uint8_t idx = 0; idx < 21; idx++) {
        if (idx <= 20) { // Add Exception entries in IDT
            add_descriptor(idx, isr_table[idx], 0x8e);
        }
    }

    cur_vector_idt = 32;

    kprintf(LIGHT_GREEN"IDT: initialized at: %16x\n", idtr.offset);
}