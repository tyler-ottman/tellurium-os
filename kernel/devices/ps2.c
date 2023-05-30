#include <acpi/acpi.h>
#include <arch/idt.h>
#include <arch/process.h>
#include <arch/scheduler.h>
#include <devices/ioapic.h>
#include <devices/lapic.h>
#include <devices/ps2.h>
#include <devices/serial.h>
#include <libc/ringbuffer.h>
#include <stdbool.h>
#include <sys/misc.h>

#define PS2_STATUS_OUTPUT_BUF           0x1
#define PS2_STATUS_INPUT_BUF            0x2

extern void *ISR_keyboard[];
extern void *ISR_mouse[];

typedef struct keyboard_state {
    bool is_capslock;
    bool is_shift;
}__attribute__((packed)) keyboard_state_t;

static char char_capslock[] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 
    '\0', '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0',
    '*', '\0', ' '
};

static char char_no_capslock[] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
    '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0',
    '*', '\0', ' '
};

static char char_shift[] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 
    '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0',
    '*', '\0', ' '
};

static char char_shift_capslock[] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',
    '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~', 
    '\0', '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '\0',
    '*', '\0', ' '
};

static keyboard_state_t keyboard_state;

void keyboard_handler(ctx_t *ctx) {
    (void)char_capslock;
    (void)char_no_capslock;
    (void)char_shift;
    (void)char_shift_capslock;

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
    if (c <= 0x39) {
        char *char_format = char_no_capslock;
        if (keyboard_state.is_capslock && keyboard_state.is_shift) {
            char_format = char_shift_capslock;
        } else if (keyboard_state.is_capslock) {
            char_format = char_capslock;
        } else if (keyboard_state.is_shift) {
            char_format = char_shift;
        }
        
        kprintf("%c", char_format[c]);
    }

    save_context(ctx);
    
    core_t *core = get_core_local_info();
    thread_t *thread = get_thread_local();

    if (core->idle_thread != thread) {
        schedule_add_thread(thread);
    }

    lapic_eoi();
    schedule_next_thread();
}

typedef struct mouse_data {
    uint8_t flags;
    int delta_x;
    int delta_y;
} mouse_data_t;

static mouse_data_t mouse_data;
static size_t mouse_cycle = 0;
static bool mouse_discard = false;

void mouse_handler(ctx_t *ctx) {
    uint8_t data = inb(PS2_DATA_REG);

    switch (mouse_cycle) {
    case 0:
        mouse_data.flags = data;

        // Bad packet inbound
        if (!(data & (1 << 3)) || (data & (1 << 6)) || (data & (1 << 7))) {
            mouse_discard = true;
            break;
        }

        mouse_cycle++;
        break;
    case 1:
        mouse_data.delta_x = (int8_t)data;
        mouse_cycle++;
        break;
    case 2:
        mouse_data.delta_y = (int8_t)data;
        mouse_cycle = 0;
        
        if (mouse_discard) {
            mouse_discard = false;
            break;
        }

        // Add mouse packet to queue

        break;
    default:
        mouse_cycle = 0;
        break;
    }

    save_context(ctx);

    core_t *core = get_core_local_info();
    thread_t *thread = get_thread_local();
    if (core->idle_thread != thread) {
        schedule_add_thread(thread);
    }

    lapic_eoi();

    schedule_next_thread();
}

static inline void ps2_wait(uint8_t status_mask) {
    ASSERT_RET(status_mask == PS2_STATUS_OUTPUT_BUF ||
               status_mask == PS2_STATUS_INPUT_BUF,);

    bool compare = status_mask == PS2_STATUS_OUTPUT_BUF;

    size_t timeout = 0x20000;
    while (timeout--) {
        if ((inb(PS2_CMD_STATUS_REG) & status_mask) == compare) {
            return;
        }
    }
}

void ps2_write(uint8_t port, uint8_t value) {
    ps2_wait(PS2_STATUS_INPUT_BUF);

    outb(port, value);
}

uint8_t ps2_read(uint8_t port) {
    ps2_wait(PS2_STATUS_OUTPUT_BUF);

    return inb(port);
}

static inline uint8_t mouse_read() {
    return ps2_read(PS2_DATA_REG);
}

static inline void mouse_write(uint8_t byte) {
    ps2_write(PS2_CMD_STATUS_REG, 0xd4);
    ps2_write(PS2_DATA_REG, byte);
}

static inline void mouse_send_command(uint8_t byte) {
    mouse_write(byte);
    mouse_read();
}

static inline void ps2_set_command(uint8_t command) {
    ps2_write(PS2_CMD_STATUS_REG, command);
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

    // Disable ps/2 ports
    ps2_set_command(0xad);
    ps2_set_command(0xa7);

    uint8_t config = ps2_read_config();
    bool port2_enabled = config & (1 << 5);

    // Enable keyboard interrupts
    config |= (1 << 0);
    
    // Enabled mouse interrupts
    if (port2_enabled) {
        config |= (1 << 1);
    }

    ps2_write_config(config);

    // Enable first ps/2 port
    ps2_set_command(0xae);

    // Enable second ps/2 port
    if (port2_enabled) {
        ps2_set_command(0xa8);
    }

    // Configure mouse
    __memset(&mouse_data, 0x0, sizeof(mouse_data_t));

    mouse_send_command(0xf6);
    mouse_send_command(0xf4);

    // Arbitrarily choose first APIC ID to send IRQs
    uint32_t *apic_ids = get_lapic_ids();

    // Route keyboard/mouse IRQ to bsp core
    ioapic_map_irq(1, apic_ids[0], false, 0, allocate_vector(), ISR_keyboard, 0x8e);
    ioapic_map_irq(12, apic_ids[0], false, 0, allocate_vector(), ISR_mouse, 0x8e);

    inb(0x60);

    init_keyboard_state();
}