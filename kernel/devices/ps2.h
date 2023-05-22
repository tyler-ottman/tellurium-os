#ifndef PS2_H
#define PS2_H

#include <stdint.h>

void keyboard_handler(ctx_t *ctx);
void mouse_handler(ctx_t *ctx);

void ps2_write(uint8_t port, uint8_t value);
uint8_t ps2_read_config(void);
void ps2_write_config(uint8_t value);
uint8_t ps2_read(uint8_t port);
void init_ps2(void);

#endif // PS2_H