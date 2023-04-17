#include <stdint.h>

#define PIT_COUNTER0        0x40
#define PIT_COUNTER1        0x41
#define PIT_COUNTER2        0x42
#define PIT_CONTROL_WORD    0x43
#define PS2_DATA_REG        0x60
#define PS2_CMD_STATUS_REG  0x64
#define COM1        0x3F8

#define WRITE_SERIAL(str) {                 \
    size_t len = __strlen(str);             \
    for (size_t i = 0; i < len; i++) {      \
        write_serial(str[i], COM1);         \
    }                                       \
    write_serial('\n', COM1);               \
}                                           \

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t data); 
void write_serial(char ch, uint16_t port);

int init_port(uint16_t port);
void init_serial(void);