#include "libTellur/mem.hpp"
#include "libTellur/syscalls.hpp"
#include "Terminal.hpp"

namespace GUI {

Terminal::Terminal(int x, int y, int width, int height, WindowFlags flags,
                   WindowPriority priority)
    : Window::Window("terminal", x, y, width, height, flags, priority) {
    FbInfo fbInfo;
    syscall_get_fb_context(&fbInfo); // TODO: don't use syscall

    terminal = terminal_alloc(
        14, 8, height, width, fbInfo.fb_height, fbInfo.fb_width,
        fbInfo.fb_pitch, fbInfo.fb_bpp, FG_COLOR_DEFAULT, BG_COLOR_DEFAULT,
        NULL, NULL, (uint32_t *)fbInfo.fb_buff, user_malloc, user_free);

    terminal->clear(terminal);

    surface->buff = terminal->buf1;
}

Terminal::~Terminal() {}

void Terminal::clear() {
    terminal->clear(terminal);
}

int Terminal::printf(const char *format, ...) {
    char buf[512];
    va_list valist;

    va_start(valist, format);
    __vsnprintf(buf, BUF_MAX, format, valist);
    va_end(valist);

    terminal->apply_set_attribute[0](terminal);
    terminal->print(terminal, buf);

    return 0;
}

void Terminal::enableCursor() {
    terminal->cursor_enabled = true;
}

void Terminal::disableCursor() {
    terminal->cursor_enabled = false;
}

void Terminal::setScroll(bool enable) {
    terminal->scroll_enabled = enable;
}

void Terminal::setFg(uint32_t color) {
    terminal->fg_color_default = color;
}

void Terminal::setBg(uint32_t color) {
    terminal->bg_color_default = color;
}

}
