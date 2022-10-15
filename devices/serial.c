#include <devices/serial.h>

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %1, %0" : : "dN"(port), "a"(data));
}

void write_serial(char ch, uint16_t port) {
    while ((inb(port + 5) & 0x20) == 0) {}
    outb(port, ch);
}

// https://wiki.osdev.org/Serial_Ports
int init_port(uint16_t port) {
    outb(port + 1, 0x00);
    outb(port + 3, 0x80);
    outb(port + 0, 0x03);
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);
    outb(port + 2, 0xC7);
    outb(port + 4, 0x0B);
    outb(port + 4, 0x1E);
    outb(port + 0, 0xAE);

    if (inb(port) != 0xAE) {
        return 1;
    }

    outb(port + 4, 0x0F);
    return 0;
}

void init_serial(void) {
    init_port(COM1);
}