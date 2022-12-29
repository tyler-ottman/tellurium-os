#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdbool.h>
#include <stdint.h>

void clear_screen(void);
void newline(void);
void putchar(char c);
void scroll_screen(void);
void init_framebuffer(void);

#endif // FRAMEBUFFER_H