#include <devices/pit.h>
#include <devices/serial.h>

typedef struct cwf {
    uint8_t bcd: 1;
    uint8_t m_mode: 3;
    uint8_t read_write: 2;
    uint8_t select_counter: 2; 
} cwf_t;

static cwf_t default_command = {
    .bcd = 0,
    .m_mode = 0,
    .read_write = 0,
    .select_counter = 0,
};

void pit_set_reload(uint8_t channel, uint16_t value) {
    cwf_t command = default_command;
    command.select_counter = channel;
    command.read_write = 3;
    command.m_mode = 4;

    outb(PIT_CONTROL_WORD, *((uint8_t*)&command));
}

uint16_t pit_get_count(uint8_t channel) {
    cwf_t command = default_command;
    command.select_counter = channel;

    outb(PIT_CONTROL_WORD, *((uint8_t*)&command));

    uint8_t low = inb(command.select_counter);
    uint8_t high = inb(command.select_counter);

    return ((uint16_t)high << 8) | low;
}

void init_pit() {

}