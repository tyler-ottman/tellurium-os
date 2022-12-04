#include <stdint.h>

#define COM1        0x3F8

// PIC Ports
#define PIC0        0x020
#define PIC1        0x021
#define PIC2        0x0a0
#define PIC3        0x0a1

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t data); 
void write_serial(char ch, uint16_t port);

int init_port(uint16_t port);
void init_serial(void);