#ifndef PS2_H
#define PS2_H

#include <stdint.h>

void keyboard_handler(ctx_t *ctx);
void mouse_handler(ctx_t *ctx);

void dev_keyboard_init(void);
void dev_mouse_init(void);

void mouse_write(uint8_t byte);
uint8_t mouse_read(void);

void ps2_wait(uint8_t status_mask);
void ps2_write(uint8_t port, uint8_t value);
uint8_t ps2_read(uint8_t port);
uint8_t ps2_read_config(void);
void ps2_write_config(uint8_t value);
void init_ps2(void);

#endif // PS2_H