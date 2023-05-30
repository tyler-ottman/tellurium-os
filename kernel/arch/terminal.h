#ifndef TERMINAL_H
#define TERMINAL_H

#include <limine.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <libc/string.h>
#include <libc/ctype.h>
#include <libc/stdlib.h>
#include <devices/serial.h>

#define BUF_MAX     512
#define ANSI_SEQ_LEN        64

// Resets state of colors
#define RESET           "\033[m"

// Terminal text colors
#define BLACK           RESET"\033[30m"
#define RED             RESET"\033[31m"
#define GREEN           RESET"\033[32m"
#define YELLOW          RESET"\033[33m"
#define BLUE            RESET"\033[34m"
#define MAGENTA         RESET"\033[35m"
#define CYAN            RESET"\033[36m"
#define LIGHT_GRAY      RESET"\033[37m"
#define GRAY            RESET"\033[90m"
#define LIGHT_RED       RESET"\033[91m"
#define LIGHT_GREEN     RESET"\033[92m"
#define LIGHT_YELLOW    RESET"\033[93m"
#define LIGHT_BLUE      RESET"\033[94m"
#define LIGHT_MAGENTA   RESET"\033[95m"
#define LIGHT_CYAN      RESET"\033[96m"
#define WHITE           RESET"\033[97m"

#define INFO            GREEN"["LIGHT_GREEN"INFO"GREEN"] "
#define ERROR           RED "[" LIGHT_RED "ERROR" RED "] "

typedef struct terminal {
    uint32_t *buffer;
    bool is_double_buffer;
    uint32_t h_cursor;
    uint32_t v_cursor;
    uint32_t h_cursor_max;
    uint32_t v_cursor_max;
    uint32_t h_font_px;
    uint32_t w_font_px;
    uint64_t h_term_px;
    uint64_t w_term_px;
    uint64_t w_fb_px;
    uint32_t *line_last_char;

    uint32_t fg_color_default;
    uint32_t bg_color_default;
    uint32_t fg_color;
    uint32_t bg_color;
    uint32_t cursor_color;
    uint64_t apply_to_fg;               // Internal sgr flag
    uint64_t is_ansi_state;
    uint64_t ansi_state;
    char ansi_sequence[ANSI_SEQ_LEN];
} terminal_t;

void kerror(const char *msg, int err);
int kprintf(const char *format, ...);
int kprintj(const char *ptr);

void init_kterminal_doublebuffer(void);
void init_kterminal(void);

#endif // TERMINAL_H