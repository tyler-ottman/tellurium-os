#include <acpi/acpi.h>
#include <arch/idt.h>
#include <arch/process.h>
#include <arch/scheduler.h>
#include <devices/ioapic.h>
#include <devices/lapic.h>
#include <devices/ps2.h>
#include <devices/serial.h>

extern void *ISR_ps2[];

void ps2_handler(ctx_t *ctx) {
    // disable_interrupts();
    kprintf("%c", inb(PS2_DATA_REG));

    struct core_local_info* cpu_info = get_core_local_info();
    thread_t *current_thread = cpu_info->current_thread;
    // kprintf("thread: %x\n", current_thread);
    if (current_thread) {
        __memcpy(&current_thread->context, ctx, sizeof(ctx_t));
    }
    
    lapic_eoi();
    schedule_next_thread();
}

static inline void ps2_set_command(uint8_t command) {
    ps2_write(PS2_CMD_STATUS_REG, command);
}

void ps2_write(uint8_t port, uint8_t value) {
    while (inb(PS2_CMD_STATUS_REG) & 2) {}
    outb(port, value);
}

uint8_t ps2_read(uint8_t port) {
    while (!(inb(PS2_CMD_STATUS_REG) & 1)) {}
    return inb(port);
}

void ps2_write_config(uint8_t value) {
    ps2_set_command(0x60); // write configuration command
    ps2_write(PS2_DATA_REG, value);
}

uint8_t ps2_read_config() {
    ps2_set_command(0x20); // read configuration command
    return ps2_read(PS2_DATA_REG);
}

void init_ps2() {
    // Disable PIC
    outb(0xa1, 0xff);
    outb(0x21, 0xff);

    ps2_set_command(0xad);
    ps2_set_command(0xa7);

    uint8_t config = ps2_read_config();
    config |= (1 << 0); // Enable keyboard interrupts
    ps2_write_config(config);

    config = ps2_read_config();

    ps2_set_command(0xae);

    // Arbitrarily choose first APIC ID to send keyboard IRQs
    uint32_t *apic_ids = get_lapic_ids();
    ioapic_map_irq(1, apic_ids[get_core_count() - 1], false, 0, allocate_vector(), ISR_ps2, 0x8e);
    enable_interrupts();
}