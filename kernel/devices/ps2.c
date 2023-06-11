#include <acpi/acpi.h>
#include <arch/idt.h>
#include <arch/process.h>
#include <arch/scheduler.h>
#include <devices/ioapic.h>
#include <devices/lapic.h>
#include <devices/ps2.h>
#include <devices/serial.h>
#include <fs/devfs.h>
#include <libc/ringbuffer.h>
#include <stdbool.h>
#include <sys/misc.h>

#define PS2_STATUS_OUTPUT_BUF           0x1
#define PS2_STATUS_INPUT_BUF            0x2

#define QUEUE_SIZE                      100

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

typedef struct keyboard_data {
    uint8_t data;
} keyboard_data_t;

// Keyboard device-specific data
vnode_t *dev_keyboard;
spinlock_t dev_keyboard_lock = 0;
static keyboard_data_t keyboard_packets[QUEUE_SIZE];
static size_t keyboard_head = 0;
static size_t keyboard_tail = 0;
static size_t num_keyboard_packets = 0;

// Keyboard Handler specific data
static keyboard_state_t keyboard_state;
static keyboard_data_t keyboard_data;

void keyboard_handler(ctx_t *ctx) {
    (void)char_capslock;
    (void)char_no_capslock;
    (void)char_shift;
    (void)char_shift_capslock;

    char *char_format = char_no_capslock; // Default
    bool discard_keyboard_packet = false;
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
    bool is_printable = c <= 0x39;
    if (is_printable) {
        if (keyboard_state.is_capslock && keyboard_state.is_shift) {
            char_format = char_shift_capslock;
        } else if (keyboard_state.is_capslock) {
            char_format = char_capslock;
        } else if (keyboard_state.is_shift) {
            char_format = char_shift;
        }
        
        // kprintf("%c", char_format[c]);
    }

    if (!is_printable) { // Change later when keyboard has better support
        discard_keyboard_packet = true;
    }

    // Add keyboard packet to queue
    if (!discard_keyboard_packet && (num_keyboard_packets < QUEUE_SIZE)) {
        spinlock_acquire(&dev_keyboard_lock);

        keyboard_data.data = char_format[c];
        __memcpy(&keyboard_packets[keyboard_tail++], &keyboard_data,
                 sizeof(keyboard_data));

        if (keyboard_tail >= QUEUE_SIZE) {
            keyboard_tail = 0;
        }

        num_keyboard_packets++;

        spinlock_release(&dev_keyboard_lock);
    }

    save_context(ctx);
    
    core_t *core = get_core_local_info();
    thread_t *thread = get_thread_local();

    if (core->idle_thread != thread) {
        schedule_add_thread(core->current_thread);
    }

    lapic_eoi();
    schedule_next_thread();
}

static int dev_keyboard_read(void *buff, vnode_t *node, size_t size,
                             size_t offset) {
    (void)node;
    (void)offset;
    
    ASSERT_RET((size % sizeof(keyboard_data_t) == 0) && (size < QUEUE_SIZE), 0);

    spinlock_acquire(&dev_keyboard_lock);

    uint64_t *kbuff = (uint64_t *)buff;
    size_t count;
    for (count = 0; count < size; count++) {
        if (num_keyboard_packets == 0) {
            spinlock_release(&dev_keyboard_lock);
            return count;
        }

        __memcpy(kbuff++, &keyboard_packets[keyboard_head++],
                 sizeof(keyboard_data_t));

        if (keyboard_head >= QUEUE_SIZE) {
            keyboard_head = 0;
        }

        num_keyboard_packets--;
    }

    spinlock_release(&dev_keyboard_lock);

    return count;
}

static vfsops_t keyboard_ops = {
    vfs_mount_stub,
    vfs_open_stub,
    vfs_close_stub,
    dev_keyboard_read,
    vfs_write_stub,
    vfs_create_stub
};

void dev_keyboard_init() {
    devfs_new_device(&dev_keyboard, "kb0", &keyboard_ops);
}

typedef struct mouse_data {
    uint8_t flags;
    int delta_x;
    int delta_y;
} mouse_data_t;

// Mouse device-specific data
vnode_t *dev_mouse;
spinlock_t dev_mouse_lock = 0;
static mouse_data_t mouse_packets[QUEUE_SIZE];
static size_t mouse_head = 0;
static size_t mouse_tail = 0;
static size_t num_mouse_packets = 0;

// Mouse Handler specific data
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

        // Discard mouse packet if buffer full
        if (num_mouse_packets >= QUEUE_SIZE) {
            break;
        }

        // Add mouse packet to queue
        spinlock_acquire(&dev_mouse_lock);

        __memcpy(&mouse_packets[mouse_tail++], &mouse_data,
                 sizeof(mouse_data_t));

        if (mouse_tail >= QUEUE_SIZE) {
            mouse_tail = 0;
        }

        num_mouse_packets++;

        spinlock_release(&dev_mouse_lock);

        break;
    default:
        mouse_cycle = 0;
        break;
    }

    core_t *core = get_core_local_info();
    thread_t *thread = core->current_thread;
    if (thread != core->idle_thread) {
        save_context(ctx);
        schedule_add_thread(thread);
    }

    lapic_eoi();

    schedule_next_thread();
}

static int dev_mouse_read(void *buff, vnode_t *node, size_t size,
                          size_t offset) {
    (void)node;
    (void)offset;

    ASSERT_RET (size == sizeof(mouse_data_t), 0);

    ASSERT_RET(num_mouse_packets != 0, 0);

    spinlock_acquire(&dev_mouse_lock);

    mouse_data_t *packet = (mouse_data_t *)buff;
    *packet = mouse_packets[mouse_head++];

    if (mouse_head >= QUEUE_SIZE) {
        mouse_head = 0;
    }

    num_mouse_packets--;

    spinlock_release(&dev_mouse_lock);

    return size;
}

static vfsops_t mouse_ops = {
    vfs_mount_stub,
    vfs_open_stub,
    vfs_close_stub,
    dev_mouse_read,
    vfs_write_stub,
    vfs_create_stub
};

void dev_mouse_init() {
    devfs_new_device(&dev_mouse, "ms0", &mouse_ops);
}

void mouse_write(uint8_t byte) {
    ps2_write(PS2_CMD_STATUS_REG, 0xd4);
    ps2_write(PS2_DATA_REG, byte);
}

uint8_t mouse_read() {
    return ps2_read(PS2_DATA_REG);
}

void ps2_wait(uint8_t status_mask) {
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