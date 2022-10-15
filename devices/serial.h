#include <stdint.h>

#define COM1 0x3F8


uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t data); 
void write_serial(char ch, uint16_t port);

int init_port(uint16_t port);
void init_serial(void);