#include <arch/cpu.h>
#include <arch/framebuffer.h>
#include <arch/terminal.h>
#include <arch/lock.h>
#include <devices/serial.h>
#include <libc/kmalloc.h>
#include <libc/print.h>
#include <stdarg.h>

#define NUM_SET_TEXT_ATTRIBUTES     9
#define NUM_RESET_TEXT_ATTRIBUTES   8

#define FG_COLOR_DEFAULT            0xff0096aa
#define BG_COLOR_DEFAULT            RESET_COLOR

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
    PROCESS_NORMAL,
    PROCESS_FG_BG,
    PROCESS_ANSI8,
    PROCESS_ANSI24
};

static uint32_t rgb256[] = {
    0x000000, 0xaa0000, 0x00aa00, 0xaaaa00, 0x0000aa, 0xaa00aa, 0x00aaaa, 0xaaaaaa,
	0x555555, 0xff5555, 0x55ff55, 0xffff55, 0x5555ff, 0xff55ff, 0x55ffff, 0xffffff,
    0x000000, 0x000033, 0x000066, 0x000099, 0x0000cc, 0x0000ff, 0x003300, 0x003333,
    0x003366, 0x003399, 0x0033cc, 0x0033ff, 0x006600, 0x006633, 0x006666, 0x006699,
    0x0066cc, 0x0066ff, 0x009900, 0x009933, 0x009966, 0x009999, 0x0099cc, 0x0099ff,
    0x00cc00, 0x00cc33, 0x00cc66, 0x00cc99, 0x00cccc, 0x00ccff, 0x00ff00, 0x00ff33,
    0x00ff66, 0x00ff99, 0x00ffcc, 0x00ffff, 0x330000, 0x330033, 0x330066, 0x330099,
    0x3300cc, 0x3300ff, 0x333300, 0x333333, 0x333366, 0x333399, 0x3333cc, 0x3333ff,
    0x336600, 0x336633, 0x336666, 0x336699, 0x3366cc, 0x3366ff, 0x339900, 0x339933,
    0x339966, 0x339999, 0x3399cc, 0x3399ff, 0x33cc00, 0x33cc33, 0x33cc66, 0x33cc99,
    0x33cccc, 0x33ccff, 0x33ff00, 0x33ff33, 0x33ff66, 0x33ff99, 0x33ffcc, 0x33ffff,
    0x660000, 0x660033, 0x660066, 0x660099, 0x6600cc, 0x6600ff, 0x663300, 0x663333,
    0x663366, 0x663399, 0x6633cc, 0x6633ff, 0x666600, 0x666633, 0x666666, 0x666699,
    0x6666cc, 0x6666ff, 0x669900, 0x669933, 0x669966, 0x669999, 0x6699cc, 0x6699ff,
    0x66cc00, 0x66cc33, 0x66cc66, 0x66cc99, 0x66cccc, 0x66ccff, 0x66ff00, 0x66ff33,
    0x66ff66, 0x66ff99, 0x66ffcc, 0x66ffff, 0x990000, 0x990033, 0x990066, 0x990099,
    0x9900cc, 0x9900ff, 0x993300, 0x993333, 0x993366, 0x993399, 0x9933cc, 0x9933ff,
    0x996600, 0x996633, 0x996666, 0x996699, 0x9966cc, 0x9966ff, 0x999900, 0x999933,
    0x999966, 0x999999, 0x9999cc, 0x9999ff, 0x99cc00, 0x99cc33, 0x99cc66, 0x99cc99,
    0x99cccc, 0x99ccff, 0x99ff00, 0x99ff33, 0x99ff66, 0x99ff99, 0x99ffcc, 0x99ffff,
    0xcc0000, 0xcc0033, 0xcc0066, 0xcc0099, 0xcc00cc, 0xcc00ff, 0xcc3300, 0xcc3333,
    0xcc3366, 0xcc3399, 0xcc33cc, 0xcc33ff, 0xcc6600, 0xcc6633, 0xcc6666, 0xcc6699,
    0xcc66cc, 0xcc66ff, 0xcc9900, 0xcc9933, 0xcc9966, 0xcc9999, 0xcc99cc, 0xcc99ff,
    0xcccc00, 0xcccc33, 0xcccc66, 0xcccc99, 0xcccccc, 0xccccff, 0xccff00, 0xccff33,
    0xccff66, 0xccff99, 0xccffcc, 0xccffff, 0xff0000, 0xff0033, 0xff0066, 0xff0099,
    0xff00cc, 0xff00ff, 0xff3300, 0xff3333, 0xff3366, 0xff3399, 0xff33cc, 0xff33ff,
    0xff6600, 0xff6633, 0xff6666, 0xff6699, 0xff66cc, 0xff66ff, 0xff9900, 0xff9933,
    0xff9966, 0xff9999, 0xff99cc, 0xff99ff, 0xffcc00, 0xffcc33, 0xffcc66, 0xffcc99,
    0xffcccc, 0xffccff, 0xffff00, 0xffff33, 0xffff66, 0xffff99, 0xffffcc, 0xffffff,
    0x000000, 0x0a0a0a, 0x141414, 0x1e1e1e, 0x282828, 0x323232, 0x3c3c3c, 0x464646,
    0x505050, 0x5a5a5a, 0x646464, 0x6e6e6e, 0x787878, 0x828282, 0x8c8c8c, 0x969696,
    0xa0a0a0, 0xaaaaaa, 0xb4b4b4, 0xbebebe, 0xc8c8c8, 0xd2d2d2, 0xdcdcdc, 0xe6e6e6,
};

static spinlock_t kprint_lock = 0;
static terminal_t kterminal;

void (*apply_set_attribute[NUM_SET_TEXT_ATTRIBUTES]) (terminal_t*);
void (*apply_reset_attribute[NUM_RESET_TEXT_ATTRIBUTES]) (terminal_t*);

void kerror(const char* err) {
    kprintf(LIGHT_RED"%s", err);
    done();
}

void no_change_text_attribute(terminal_t* terminal) {}

void reset_text_attribute(terminal_t* terminal) {
    terminal->ansi_state = PROCESS_NORMAL;
    terminal->fg_color = FG_COLOR_DEFAULT;
    terminal->bg_color = BG_COLOR_DEFAULT;
}

static inline bool num_in_byte_bounds(int n) {
    return (n >= 0 && n <= 255);
}

static inline void apply_color(terminal_t* terminal, uint32_t color) {
    if (terminal->apply_to_fg) {
        terminal->fg_color = color;
    } else {
        terminal->bg_color = color;
    }
}

static void parse_sgr(terminal_t* terminal, char* sequence) {
    uint32_t color = 0;
    if (__strlen(sequence) == 0) {
        // Reset ANSI attributes
        reset_text_attribute(terminal);
        return;
    }

    char *tok = __strtok(sequence, ";");
    bool invalid_state;
    while (tok != NULL) {
        int n = __atoi(tok);

        switch (terminal->ansi_state) {
        
        case PROCESS_NORMAL:

            // Set text attributes
            if (n >= SET_RESET && n <= SET_STRIKETHROUGH) {
                (*apply_set_attribute[n])(terminal);
            }
            
            // Reset text attributes
            else if (n >= RESET_BOLD && n <= RESET_STRIKETHROUGH) {
                (*apply_reset_attribute[n - RESET_BOLD])(terminal);
            }
            
            // Process 8-bit or 24-bit color foreground/background
            else if (n == FG_PSEUDO_MODE || n == BG_PSEUDO_MODE) {
                terminal->ansi_state = PROCESS_FG_BG;
                terminal->apply_to_fg = n == FG_PSEUDO_MODE;
            }

            // Apply 16 color mode normal foreground
            else if (n >= FG_BLACK && n <= FG_WHITE) {
                terminal->fg_color = rgb256[n - FG_BLACK];
            }

            // Apply 16 color mode bright foreground
            else if (n >= FG_BRIGHT_BLACK && n <= FG_BRIGHT_WHITE) {
                terminal->fg_color = rgb256[n - FG_BRIGHT_BLACK + 8];
            }

            // Apply 16 color mode background
            else if (n >= BG_BLACK && n <= BG_WHITE) {
                terminal->bg_color = rgb256[n - BG_BLACK];
            }

            else if (n >= BG_BRIGHT_BLACK && n <= BG_BRIGHT_WHITE) {
                terminal->bg_color = rgb256[n - BG_BRIGHT_BLACK + 8];
            }

            break;
        
        // Process FG/BG logic
        case PROCESS_FG_BG:
            if (n != ANSI8 && n!= ANSI24) {
                terminal->ansi_state = PROCESS_NORMAL;
                break;
            }
            
            terminal->ansi_state = n == ANSI8 ? PROCESS_ANSI8 : PROCESS_ANSI24;
            break;

        case PROCESS_ANSI8:
            if (!num_in_byte_bounds(n)) {
                terminal->ansi_state = PROCESS_NORMAL;
                break;
            }

            color = rgb256[n];
            
            apply_color(terminal, color);

            terminal->ansi_state = PROCESS_NORMAL;
            break;

        case PROCESS_ANSI24:
            invalid_state = false;

            for (int i = 2; i >= 0; i--) {
                n = __atoi(tok);
                if (!num_in_byte_bounds(n)) {
                    terminal->ansi_state = PROCESS_NORMAL;
                    invalid_state = true;
                    break;
                }

                color |= (n << (8 * (2 - i)));

                tok = __strtok(NULL, ";");
                if (!tok && i) {
                    terminal->ansi_state = PROCESS_NORMAL;
                    invalid_state = true;
                    break;
                }
            }

            if (!invalid_state) {
                apply_color(terminal, color);
            }

            break;
        }

        tok = __strtok(NULL, ";");
    }
}

static void terminal_parse_ansi(terminal_t* terminal) {
    char* ansi_sequence = terminal->ansi_sequence;

    if (ansi_sequence[0] != '[') {
        return;
    }

    size_t code_idx = __strlen(++ansi_sequence) - 1;
    char ansi_code = ansi_sequence[__strlen(ansi_sequence) - 1];
    ansi_sequence[code_idx] = '\0';

    switch (ansi_code) {
    case 'm': // Parse color code
        parse_sgr(terminal, ansi_sequence);
        break;
    }
}

static void terminal_printf(terminal_t* terminal, const char* buf) {
    const char* start = buf;

    draw_cursor(terminal, RESET_COLOR);
    
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
                newline(terminal);
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

    draw_cursor(terminal, CURSOR_COLOR);
}

int kprintf(const char* format, ...) {
    char buf[BUF_MAX];
    va_list valist;

    va_start(valist, format);
    int err = __vsnprintf(buf, BUF_MAX, format, valist);
    va_end(valist);
    ASSERT(err != -1);

    reset_text_attribute(&kterminal);

    spinlock_acquire(&kprint_lock);
    terminal_printf(&kterminal, buf);
    spinlock_release(&kprint_lock);

    return err;
}

static void print_color_palette() {
    for (size_t i = 0; i < 16; i++) {
        kprintf("\033[38;5;%i;48;5;%im%03i", i, i, i);
    }

    kprintf("\n\n");
    for (size_t i = 0; i < 6; i ++) {
        for (size_t j = 0; j < 36; j++) {
            uint8_t color_id = 16 + 36 * i + j;
            kprintf("\033[38;5;%i;48;5;%im%03i", color_id, color_id, color_id);
        }
        kprintf("\n");
    }

    kprintf("\n");
    for (size_t i = 232; i < 256; i++) {
        kprintf("\033[38;5;%i;48;5;%im%03i", i, i, i);
    }
    kprintf("\n\n");
}

static terminal_t* alloc_terminal_internal(
    terminal_t* term,
    uint32_t w_font,
    uint32_t h_font,
    uint32_t w_term_px,
    uint32_t h_term_px
) {
    if (!term) {
        term = kmalloc(sizeof(terminal_t));
    }

    term->h_cursor = 0;
    term->v_cursor = 0;
    term->w_font_px = w_font;
    term->h_font_px = h_font;
    term->w_term_px = w_term_px;
    term->h_term_px = h_term_px;
    term->h_cursor_max = term->w_term_px / term->w_font_px;
    term->v_cursor_max = term->h_term_px / term->h_font_px;

    term->fg_color = FG_COLOR_DEFAULT;
    term->bg_color = BG_COLOR_DEFAULT;
    term->apply_to_fg = false;
    term->is_ansi_state = false;
    term->ansi_state = PROCESS_NORMAL;

    return term;
}

void init_kterminal() {
    init_framebuffer();

    for (int i = 0; i < NUM_SET_TEXT_ATTRIBUTES; i++) {
        apply_set_attribute[i] = no_change_text_attribute;
    }

    for (int i = 0; i < NUM_RESET_TEXT_ATTRIBUTES; i++) {
        apply_reset_attribute[i] = no_change_text_attribute;
    }

    apply_set_attribute[SET_RESET] = reset_text_attribute;

    alloc_terminal_internal(&kterminal, 8, 14, get_fb_width(), get_fb_height());

    // print_color_palette();
}