#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdbool.h>
#include <stdint.h>
#include <terminal.h>

typedef struct fb_context {
    void *fb_buff;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
} fb_context_t;

void clear_screen(terminal_t *term);
void fb_load_buffer(terminal_t *term);
void newline(terminal_t *term);
void backspace(terminal_t *term);
void scroll_screen(terminal_t *term);
void drawchar(terminal_t *terminal, char c);
void draw_cursor(terminal_t *term, uint32_t color);

#endif // FRAMEBUFFER_H