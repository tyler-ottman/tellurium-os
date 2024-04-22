#ifndef TERMINAL_H
#define TERMINAL_H

#include <flibc/ctype.h>
#include <flibc/print.h>
#include <flibc/stdlib.h>
#include <flibc/string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BUF_MAX                             512
#define ANSI_SEQ_LEN                        64

// Resets state of colors
#define RESET                               "\033[m"

// Terminal text colors
#define BLACK                               RESET"\033[30m"
#define RED                                 RESET"\033[31m"
#define GREEN                               RESET"\033[32m"
#define YELLOW                              RESET"\033[33m"
#define BLUE                                RESET"\033[34m"
#define MAGENTA                             RESET"\033[35m"
#define CYAN                                RESET"\033[36m"
#define LIGHT_GRAY                          RESET"\033[37m"
#define GRAY                                RESET"\033[90m"
#define LIGHT_RED                           RESET"\033[91m"
#define LIGHT_GREEN                         RESET"\033[92m"
#define LIGHT_YELLOW                        RESET"\033[93m"
#define LIGHT_BLUE                          RESET"\033[94m"
#define LIGHT_MAGENTA                       RESET"\033[95m"
#define LIGHT_CYAN                          RESET"\033[96m"
#define WHITE                               RESET"\033[97m"

#define INFO                                GREEN "[" LIGHT_GREEN "INFO" GREEN "] "
#define ERROR                               RED "[" LIGHT_RED "ERROR" RED "] "

#define NUM_SET_TEXT_ATTRIBUTES             9
#define NUM_RESET_TEXT_ATTRIBUTES           8

#define CURSOR_COLOR                        0xff808080
#define RESET_COLOR                         0xff333333
#define FG_COLOR_DEFAULT                    0xff0096aa
#define BG_COLOR_DEFAULT                    0x7f333333

typedef struct terminal {
    // User provided memory management
    void *(*__malloc)(size_t size);
    void (*__free)(void *addr);

    // Print to off-screen buffer
    void (*clear)(struct terminal *term);
    void (*print)(struct terminal *term, const char *buf);
    void (*refresh)(struct terminal *term);

    // Font info
    uint32_t font_h_px;                     // Font height
    uint32_t font_w_px;                     // Font width
    uint8_t *bitmap;                        // Font bitmap

    // Terminal buffer info
    uint64_t term_h_px;                     // Terminal height
    uint64_t term_w_px;                     // Terminal width
    uint32_t *buf1;                         // Off-screen buffer

    // Cursor info
    uint32_t cursor_h;                      // Horizontal cursor
    uint32_t cursor_v;                      // Vertical cursor
    uint32_t cursor_h_max;
    uint32_t cursor_v_max;
    uint32_t cursor_color;
    bool cursor_enabled;

    // Scroll Info
    bool scroll_enabled;

    // ANSI state info
    uint32_t fg_color_default;
    uint32_t bg_color_default;
    uint32_t fg_color;                      // Foreground color
    uint32_t bg_color;                      // Background color
    uint64_t apply_to_fg;
    uint64_t is_ansi_state;
    uint64_t ansi_state;
    char ansi_sequence[ANSI_SEQ_LEN];
    void (*apply_set_attribute[NUM_SET_TEXT_ATTRIBUTES])(struct terminal *);
    void (*apply_reset_attribute[NUM_RESET_TEXT_ATTRIBUTES])(struct terminal *);

    // Framebuffer info
    uint32_t *buf2;                     // Framebuffer
    uint64_t fb_w_px;                   // Framebuffer width
    uint64_t fb_h_px;                   // Framebuffer height
} terminal_t;

// void terminal_print(terminal_t *terminal, const char *buf);
// void terminal_refresh(terminal_t *terminal);
terminal_t *terminal_alloc(uint32_t h_font, uint32_t w_font, uint64_t term_h_px,
                           uint64_t term_w_px, uint32_t fb_h_px,
                           uint32_t fb_w_px, uint32_t fb_pitch, uint32_t fb_bpp,
                           uint64_t fg_color_default, uint64_t bg_color_default,
                           uint8_t *bitmap, uint32_t *buffer1,
                           uint32_t *buffer2, void *(__malloc)(size_t),
                           void (*__free)(void *));

#endif // TERMINAL_H