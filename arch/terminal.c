#include <arch/cpu.h>
#include <arch/framebuffer.h>
#include <arch/terminal.h>
#include <arch/lock.h>
#include <devices/serial.h>
#include <libc/print.h>
#include <stdarg.h>

#define NUM_SET_TEXT_ATTRIBUTES     9
#define NUM_RESET_TEXT_ATTRIBUTES   8

#define FG_COLOR_DEFAULT            0xff00aaaa
#define BG_COLOR_DEFAULT            0
#define GRAYSCALE_FACTOR            (0xffffffff / 24)

enum SET_TEXT_ATTRIBUTE {
    SET_RESET,
    SET_STRIKETHROUGH       = 9,
    SET_DOUBLE_UNDERLINE    = 21
};

enum RESET_TEXT_ATTRIBUTE {
    RESET_BOLD              = 22,
    RESET_STRIKETHROUGH     = 29
};

enum FG_NORMAL_CODE {
    FG_BLACK                = 30,
    FG_WHITE                = 37,
    FG_PSEUDO_MODE,
    FG_DEFAULT
};

enum BG_NORMAL_CODE {
    BG_BLACK                = 40,
    BG_WHITE                = 47,
    BG_PSEUDO_MODE,
    BG_DEFAULT,
};

enum FG_BRIGHT_CODE {
    FG_BRIGHT_BLACK         = 90,
    FG_BRIGHT_WHITE         = 97
};

enum BG_BRIGHT_CODE {
    BG_BRIGHT_BLACK         = 100,
    BG_BRIGHT_WHITE         = 107
};

enum ANSI_COLOR_MODE {
    ANSI4,                  // 16 (4-bit) color mode
    ANSI8           = 5,    // 256 (8-bit) color mode
    ANSI24          = 2     // 24-bit color
};

enum ANSI_COLOR_STATE {
    ANSI_COLOR_NORMAL,
    ANSI_COLOR_FG,
    ANSI_COLOR_BG,
    PROCESS_ANSI8,
    PROCESS_ANSI24
};

static uint32_t vga_to_rgb[] = {
	0xff000000,
	0xffaa0000,
	0xff00aa00,
	0xffaaaa00,
	0xff0000aa,
	0xffaa00aa,
	0xff00aaaa,
	0xffaaaaaa,
	0xff555555,
	0xffff5555,
	0xff55ff55,
	0xffffff55,
	0xff5555ff,
	0xffff55ff,
	0xff55ffff,
	0xffffffff,
};

static spinlock_t kprint_lock = 0;
static terminal kterminal;

void (*apply_set_attribute[NUM_SET_TEXT_ATTRIBUTES]) (terminal*);
void (*apply_reset_attribute[NUM_RESET_TEXT_ATTRIBUTES]) (terminal*);

void kerror(const char* err) {
    kprintf(LIGHT_RED"%s", err);
    done();
}

void no_change_text_attribute(terminal* terminal) {}

void reset_text_attribute(terminal* terminal) {
    terminal->ansi_state = ANSI_COLOR_NORMAL;
    terminal->fg_color = FG_COLOR_DEFAULT;
    terminal->bg_color = BG_COLOR_DEFAULT;
    __memset(&terminal->attributes, 0, sizeof(ansi_attributes));
}

static inline bool num_in_byte_bounds(int n) {
    return (n >= 0 && n <= 255);
}

static inline bool is_valid_color_format(const int type) {
    return (type == ANSI8 || type == ANSI24);
}

static void parse_sgr(terminal* terminal, char* sequence) {
    if (__strlen(sequence) == 0) {
        // Reset ANSI attributes
        reset_text_attribute(terminal);
        return;
    }

    char *tok = __strtok(sequence, ";");
    while (tok != NULL) {
        int n = __atoi((const char**)&tok);

        switch (terminal->ansi_state) {
        
        case ANSI_COLOR_NORMAL:

            // Set text attributes
            if (n >= SET_RESET && n <= SET_STRIKETHROUGH) {
                (*apply_set_attribute[n])(terminal);
            }
            
            // Reset text attributes
            else if (n >= RESET_BOLD && n <= RESET_STRIKETHROUGH) {
                (*apply_reset_attribute[n - RESET_BOLD])(terminal);
            }
            
            // Process 8-bit or 24-bit color foreground
            else if (n == FG_PSEUDO_MODE) {
                terminal->ansi_state = ANSI_COLOR_FG;
            }

            // Process 8-bit or 24-bit color background
            else if (n == BG_PSEUDO_MODE) {
                terminal->ansi_state = ANSI_COLOR_FG;
            }

            // Apply 16 color mode normal foreground
            else if (n >= FG_BLACK && n <= FG_WHITE) {
                terminal->fg_color = vga_to_rgb[n - FG_BLACK];
            }

            // Apply 16 color mode bright foreground
            else if (n >= FG_BRIGHT_BLACK && n <= FG_BRIGHT_WHITE) {
                terminal->fg_color = vga_to_rgb[n - FG_BRIGHT_BLACK + 8];
            }

            // Apply 16 color mode background
            else if (n >= BG_BLACK && n <= BG_WHITE) {
                terminal->bg_color = vga_to_rgb[n - BG_BLACK];
            }

            else if (n >= BG_BRIGHT_BLACK && n <= BG_BRIGHT_WHITE) {
                terminal->bg_color = vga_to_rgb[n - BG_BRIGHT_BLACK + 8];
            }

            break;
        
        // Process FG/BG logic
        case ANSI_COLOR_FG:
        case ANSI_COLOR_BG:
            if (!is_valid_color_format(n)) {
                terminal->ansi_state = ANSI_COLOR_NORMAL;
                break;
            }
            
            terminal->apply_to_fg = terminal->ansi_state == ANSI_COLOR_FG;
            terminal->ansi_state = n == ANSI8 ? PROCESS_ANSI8 : PROCESS_ANSI24;
            break;

        case PROCESS_ANSI8: {
            uint32_t color = 0xff000000;
            if (!num_in_byte_bounds(n)) {
                terminal->ansi_state = ANSI_COLOR_NORMAL;
                break;
            }

            uint8_t color_code = (uint8_t)n;

            // Standard 16 VGA colors
            if (color_code <= 15) {
                color = vga_to_rgb[color_code];
            } 
            
            // 8-bit color
            else if (color_code > 15 && color_code <= 231) {
                color_code -= 16;
                uint8_t red = color_code / 36;
                color_code -= 36 * red;
                uint8_t green = color_code / 6;
                color_code -= 6 * green;
                uint8_t blue = color_code;
                
                // Apply scale from 3-bit to 8-bit
                red *= 51, green *= 51, blue *= 51;
                color |= (red << 16) | (green << 8) | blue;
            }
            
            // grayscale
            else {
                color_code -= 232;
                color = GRAYSCALE_FACTOR * color_code;
            }
            
            // Apply color to FG/BG
            if (terminal->apply_to_fg) {
                terminal->fg_color = color;
            } else {
                terminal->bg_color = color;
            }

            terminal->ansi_state = ANSI_COLOR_NORMAL;
            break;
        }

        case PROCESS_ANSI24: {
            // do {
            //     if (!num_in_byte_bounds(n)) {
            //         terminal->ansi_state = ANSI_COLOR_NORMAL;
            //         apply_color_to_fg = 
            //     }
            // }
            terminal->ansi_state = ANSI_COLOR_NORMAL;
            break;
        }
        
        }

        tok = __strtok(NULL, ";");
    }
}

static void terminal_parse_ansi(terminal* terminal) {
    char* ansi_sequence = terminal->ansi_sequence;

    if (ansi_sequence[0] != '[') {
        return;
    }

    ansi_sequence++;
    size_t code_idx = __strlen(ansi_sequence) - 1;
    char ansi_code = ansi_sequence[__strlen(ansi_sequence) - 1];
    ansi_sequence[code_idx] = '\0';

    switch (ansi_code) {
    case 'm': // Parse color code
        parse_sgr(terminal, ansi_sequence);
        break;
    }
}

static void terminal_printf(terminal* terminal, const char* buf) {
    const char* start = buf;
    
    while (*buf) {
        if (terminal->is_ansi_state) {
            char next_ansi[2] = {*buf, '\0'};
            __strcat(terminal->ansi_sequence, next_ansi);

            if (terminal->ansi_sequence[ANSI_SEQ_LEN-1] != '\0') {
                terminal->is_ansi_state = false;
                buf = ++start;
            } else if (__strchr("[;01234567890", *buf) == NULL) {
                terminal->is_ansi_state = false;
                terminal_parse_ansi(terminal);
                terminal->ansi_sequence[0] = '\0';
            }

        } else {
            switch (*buf) {
            case '\b':
                break;
            case '\033':
                terminal->is_ansi_state = true;
                break;
            case '\n':
                newline();
                break;
            case '\r':
                break;
            case '\t':
                break;
            case '\v':
                break;
            default:
                drawchar(terminal, *buf);
                break;
            }
        }

        buf++;
    }
}

int kprintf(const char* format, ...) {
    char buf[BUF_MAX];
    va_list valist;
    
    spinlock_acquire(&kprint_lock);

    va_start(valist, format);
    int err = __vsnprintf(buf, BUF_MAX, format, valist);
    va_end(valist);
    ASSERT(err != -1);

    reset_text_attribute(&kterminal);
    terminal_printf(&kterminal, buf);

    spinlock_release(&kprint_lock);

    return err;
}

void init_kterminal() {
    for (int i = 0; i < NUM_SET_TEXT_ATTRIBUTES; i++) {
        apply_set_attribute[i] = no_change_text_attribute;
    }

    for (int i = 0; i < NUM_RESET_TEXT_ATTRIBUTES; i++) {
        apply_reset_attribute[i] = no_change_text_attribute;
    }

    apply_set_attribute[SET_RESET] = reset_text_attribute;

    kterminal.fg_color = FG_COLOR_DEFAULT;
    kterminal.bg_color = BG_COLOR_DEFAULT;
    kterminal.apply_to_fg = false;
    kterminal.is_ansi_state = 0;
    kterminal.ansi_state = ANSI_COLOR_NORMAL;
}