#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <arch/terminal.h>
#include <limine.h>
#include <stdbool.h>
#include <stdint.h>

#define CURSOR_COLOR    0xff808080
#define RESET_COLOR     0x00000000

typedef struct fb_context {
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
} fb_context_t;

void clear_screen(terminal_t *term);
uint32_t fb_get_width(void);
uint32_t fb_get_height(void);
uint32_t *fb_get_framebuffer(void);
uint16_t fb_get_bpp(void);
uint64_t fb_get_pitch(void);
void fb_load_buffer(terminal_t *term);
void newline(terminal_t* term);
void backspace(terminal_t *term);
void scroll_screen(terminal_t* term);
void drawchar(terminal_t* terminal, char c);
void draw_cursor(terminal_t* term, uint32_t color);
void init_framebuffer(void);

#endif // FRAMEBUFFER_H