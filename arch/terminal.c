#include <arch/terminal.h>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.
static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static struct limine_terminal *terminal;

void init_terminal() {
    if (terminal_request.response == NULL
     || terminal_request.response->terminal_count < 1) {
        done();
    }

    terminal = terminal_request.response->terminals[0];
}

// Halt CPU activity
void done(void) {
    for (;;) {
        __asm__("hlt");
    }
}

void terminal_print(const char* str) {
    int len = __strlen(str);
    terminal_request.response->write(terminal, str, len);
}

void terminal_println(const char* str) {
    int len = __strlen(str);
    terminal_request.response->write(terminal, str, len);
    terminal_request.response->write(terminal, "\n", 1);
}