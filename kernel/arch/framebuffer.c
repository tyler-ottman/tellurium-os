#include <arch/framebuffer.h>
#include <libc/string.h>

#define ANSI_ESC        '\033'

static inline bool is_scroll_needed(terminal_t *term) {
    return term->cursor_v + 1 >= term->cursor_v_max;
}

static inline uint32_t get_px_base(terminal_t *term) {
    return term->font_h_px * term->term_w_px * term->cursor_v +
           term->font_w_px * term->cursor_h;
}

void fb_load_buffer(terminal_t *term) {
    for (size_t i = 0; i < term->term_h_px; i++) {
        for (size_t j = 0; j < term->term_w_px; j++) {
            term->buf2[term->fb_w_px * i + j] =
                term->buf1[term->term_w_px * i + j];
        }
    }
}

void newline(terminal_t *term) {
    if (is_scroll_needed(term)) {
        scroll_screen(term);
    } else {
        term->cursor_v++;
    }

    term->cursor_h = 0;
}

void backspace(terminal_t *term) {
    if (term->cursor_h == 0 && term->cursor_v != 0) {
        term->cursor_v--;
    } else if (term->cursor_h != 0) {
        term->cursor_h--;
    }
}

void clear_screen(terminal_t *term) {
    for (size_t i = 0; i < term->term_h_px; i++) {
        for (size_t j = 0; j < term->term_w_px; j++) {
            term->buf1[term->term_w_px * i + j] = term->bg_color_default;
        }
    }
}

void scroll_screen(terminal_t *term) {
    uint32_t v_px_offset = term->font_h_px * term->term_w_px;
    uint32_t px_base = 0;
    for (size_t i = 0; i < term->term_h_px - term->font_h_px; i++) {
        for (size_t j = 0; j < term->term_w_px; j++) {
            term->buf1[px_base + j] = term->buf1[px_base + v_px_offset + j];
        }
        px_base += term->term_w_px;
    }

    // Clear last line
    for (size_t i = 0; i < term->font_h_px; i++) {
        for (size_t j = 0; j < term->term_w_px; j++) {
            term->buf1[px_base + j] = term->bg_color_default;
        }
        px_base += term->term_w_px;
    }
}

void drawchar(terminal_t *term, char c) {
    uint8_t* cur_char = &term->bitmap[term->font_h_px * (uint8_t)c];
    uint32_t px_base = get_px_base(term);
    for (size_t i = 0; i < term->font_h_px; i++) {
        for (size_t j = 0; j < term->font_w_px; j++) {
            term->buf1[px_base + j] =
                ((cur_char[i] >> j) & 1) ? term->fg_color : term->bg_color;
        }
        px_base += term->term_w_px;
    }

    if (term->cursor_h + 1 >= term->cursor_h_max) {
        newline(term);
    } else {
        term->cursor_h++;
    }
}

void draw_cursor(terminal_t *term, uint32_t color) {
    uint64_t px_base = get_px_base(term);
    for (size_t i = 0; i < term->font_h_px; i++) {
        for (size_t j = 0; j < term->font_w_px; j++) {
            term->buf1[px_base + j] = color;
        }
        px_base += term->term_w_px;
    }
}