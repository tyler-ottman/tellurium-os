#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <arch/terminal.h>
#include <stdbool.h>
#include <stdint.h>

void clear_screen(void);
void newline(void);
void drawchar(terminal* terminal, char c);
void scroll_screen(void);
void init_framebuffer(void);

#endif // FRAMEBUFFER_H