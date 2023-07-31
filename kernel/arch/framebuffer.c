#include <arch/framebuffer.h>
#include <arch/kterminal_font.h>
#include <arch/terminal.h>
#include <libc/string.h>

#define ANSI_ESC        '\033'

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static struct limine_framebuffer_response *framebuffer_response;
static struct limine_framebuffer *framebuffer;
static uint32_t *px_buffer;

static inline bool is_scroll_needed(terminal_t *term) {
    return term->v_cursor + 1 >= term->v_cursor_max;
}

static inline uint32_t get_px_base(terminal_t *term) {
    return term->h_font_px * term->w_fb_px * term->v_cursor + term->w_font_px * term->h_cursor;
}

uint32_t fb_get_width() {
    return framebuffer->width;
}

uint32_t fb_get_height() {
    return framebuffer->height;
}

uint32_t *fb_get_framebuffer() {
    return (uint32_t *)framebuffer->address;
}

uint16_t fb_get_bpp() {
    return framebuffer->bpp;
}

uint64_t fb_get_pitch() {
    return framebuffer->pitch;
}

void fb_load_buffer(terminal_t *term) {
    uint32_t px_base = 0;
    for (size_t i = 0; i < term->h_term_px; i++) {
        for (size_t j = 0; j < term->w_term_px; j++) {
            px_buffer[px_base + j] = term->buffer1[px_base + j];
        }
        px_base += term->w_fb_px;
    }
}

void newline(terminal_t *term) {
    if (is_scroll_needed(term)) {
        scroll_screen(term);
    } else {
        term->v_cursor++;
    }

    term->h_cursor = 0;
}

void backspace(terminal_t *term) {
    if (term->h_cursor == 0 && term->v_cursor != 0) {
        term->v_cursor--;
    } else if (term->h_cursor != 0) {
        term->h_cursor--;
    }
}

void clear_screen(terminal_t *term) {
    for (size_t i = 0; i < framebuffer->height; i++) {
        for (size_t j = 0; j < framebuffer->width; j++) {
            term->buffer1[framebuffer->width * i + j] = term->bg_color_default;
        }
    }

    // fb_load_buffer(term);
}

void scroll_screen(terminal_t *term) {
    uint32_t v_px_offset = term->h_font_px * term->w_term_px;
    uint32_t px_base = 0; // Change later with windowing
    for (size_t i = 0; i < term->h_term_px - term->h_font_px; i++) {
        for (size_t j = 0; j < term->w_term_px; j++) {
            term->buffer1[px_base + j] = term->buffer1[px_base + v_px_offset + j];
        }
        px_base += term->w_fb_px;
    }

    // Clear last line
    for (size_t i = 0; i < term->h_font_px; i++) {
        for (size_t j = 0; j < term->w_term_px; j++) {
            term->buffer1[px_base + j] = term->bg_color_default;
        }
        px_base += term->w_fb_px;
    }
}

void drawchar(terminal_t *term, char c) {
    uint8_t* cur_char = kterminal_font[(uint8_t)c];
    uint32_t px_base = get_px_base(term);
    for (size_t i = 0; i < term->h_font_px; i++) {
        for (size_t j = 0; j < term->w_font_px; j++) {
            term->buffer1[px_base + j] = ((cur_char[i] >> j) & 1) ? term->fg_color : term->bg_color;
        }
        px_base += term->w_fb_px;
    }

    if (term->h_cursor + 1 >= term->h_cursor_max) {
        newline(term);
    } else {
        term->h_cursor++;
    }
}

void draw_cursor(terminal_t *term, uint32_t color) {
    uint64_t px_base = get_px_base(term);
    for (size_t i = 0; i < term->h_font_px; i++) {
        for (size_t j = 0; j < term->w_font_px; j++) {
            term->buffer1[px_base + j] = color;
        }
        px_base += term->w_fb_px;
    }
}

void init_framebuffer() {
    framebuffer_response = framebuffer_request.response;
    framebuffer = framebuffer_response->framebuffers[0];
    px_buffer = (uint32_t *)framebuffer->address;
}