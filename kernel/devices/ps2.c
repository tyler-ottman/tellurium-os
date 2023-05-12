#include <acpi/acpi.h>
#include <arch/idt.h>
#include <arch/process.h>
#include <arch/scheduler.h>
#include <devices/ioapic.h>
#include <devices/lapic.h>
#include <devices/ps2.h>
#include <devices/serial.h>
#include <stdbool.h>

extern void *ISR_ps2[];

typedef struct keyboard_state {
    bool is_capslock;
    bool is_shift;
}__attribute__((packed)) keyboard_state_t;

// static char char_capslock[] = {
//     '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
//     '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
//     '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 
//     '\0', '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0',
//     '*', '\0', ' '
// };

// static char char_no_capslock[] = {
//     '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
//     '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
//     '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
//     '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0',
//     '*', '\0', ' '
// };

// static char char_shift[] = {
//     '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
//     '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
//     '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 
//     '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0',
//     '*', '\0', ' '
// };

// static char char_shift_capslock[] = {
//     '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
//     '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',
//     '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~', 
//     '\0', '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '\0',
//     '*', '\0', ' '
// };

static keyboard_state_t keyboard_state;

void ps2_handler(ctx_t *ctx) {
    uint8_t c = inb(PS2_DATA_REG);

    switch (c) {
    case 0x3A:
        keyboard_state.is_capslock = !keyboard_state.is_capslock;
        break;
    case 0x2A:
        keyboard_state.is_shift = true;
        break;
    case 0xAA:
        keyboard_state.is_shift = false;
        break;
    }

    // testing print
    // if (c <= 0x39) {
    //     char *char_format = char_no_capslock;
    //     if (keyboard_state.is_capslock && keyboard_state.is_shift) {
    //         char_format = char_shift_capslock;
    //     } else if (keyboard_state.is_capslock) {
    //         char_format = char_capslock;
    //     } else if (keyboard_state.is_shift) {
    //         char_format = char_shift;
    //     }
        
    //     kprintf("%c", char_format[c]);
    // }

    struct core_local_info* cpu_info = get_core_local_info();
    save_context(cpu_info, ctx);
    
    thread_t *current_thread = cpu_info->current_thread;
    if (cpu_info->idle_thread != current_thread) {
        schedule_add_thread(current_thread);
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

static void init_keyboard_state() {
    keyboard_state.is_capslock = false;
    keyboard_state.is_shift = false;
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
    ioapic_map_irq(1, apic_ids[0], false, 0, allocate_vector(), ISR_ps2, 0x8e);
    inb(0x60);

    init_keyboard_state();
}