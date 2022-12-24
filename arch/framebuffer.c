#include <arch/bitmap_font.h>
#include <arch/framebuffer.h>
#include <arch/terminal.h>
#include <libc/string.h>
#include <limine.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

struct frame_state {
    uint64_t cur_line_pixel;
    uint64_t width;
    uint64_t height;
    uint32_t h_cursor;
    uint32_t v_cursor;
    uint32_t color;
};

static struct limine_framebuffer_response* framebuffer_response;
static struct limine_framebuffer* framebuffer;
static uint32_t* pixel_buffer;
static struct frame_state state;

static inline bool is_scroll_needed() {
    return state.v_cursor + FONT_HEIGHT >= state.height;
}

static inline void newline() {
    if (is_scroll_needed()) {
        scroll_screen();
    } else {
        state.cur_line_pixel += FONT_HEIGHT * state.width;
        state.v_cursor += FONT_HEIGHT;
    }

    state.h_cursor = 0;
}

static bool state_changed(char c) {
    switch (c) {
    case '\0':
        return false; // todo, colors
    case '\n':
        newline();
        return true;
    }

    return false;
}

void clear_screen() {
    for (size_t i = 0; i < state.width; i++) {
        for (size_t j = 0; j < state.width; j++) {
            pixel_buffer[state.width * i + j] = 0;
        }
    }
}

void scroll_screen() {
    for (size_t i = 0; i < state.height - FONT_HEIGHT; i += FONT_HEIGHT) {
        for (size_t j = 0; j < FONT_HEIGHT; j++) {
            for (size_t k = 0; k < state.width; k++) {
                pixel_buffer[(i + j) * state.width + k] = pixel_buffer[(i + j + FONT_HEIGHT) * state.width + k];
            }
        }
    }

    uint32_t* last_line = &pixel_buffer[state.v_cursor * state.width];
    __memset(last_line, 0x00, 4 * FONT_HEIGHT * state.width);
}

void putchar(char c) {
    if ((c < 0x20) && state_changed(c)) { // Escape character
        return;
    }

    uint8_t* cur_char = font[(uint8_t)c];
    if (state.h_cursor >= state.width) {
        newline();
    }

    for (int i = 0; i < FONT_HEIGHT; i++) {
        size_t start = state.cur_line_pixel + state.h_cursor + i * state.width;
        for (int j = 0; j < FONT_WIDTH; j++) {
            pixel_buffer[start + j] = ((cur_char[i] >> j) & 1) ? state.color : 0;
        }
    }

    state.h_cursor += FONT_WIDTH;
}

void init_framebuffer() {
    framebuffer_response = framebuffer_request.response;
    framebuffer = framebuffer_response->framebuffers[0];
    pixel_buffer = (uint32_t*)framebuffer->address;

    state.cur_line_pixel = 0;
    state.width = framebuffer->width;
    state.height = framebuffer->height;
    state.h_cursor = 0;
    state.v_cursor = 0;
    state.color = 0xffffffff;

    clear_screen();
}