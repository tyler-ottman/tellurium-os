#include "libTellur/mem.hpp"
#include "Terminal.hpp"

namespace GUI {

Terminal::Terminal(int x, int y, int width, int height)
    : Window::Window("terminal", x, y, width, height, 0) {
    FbMeta *meta = context->getFbContext();
    terminal = terminal_alloc(
        14, 8, height, width, meta->fb_height, meta->fb_width, meta->fb_pitch,
        meta->fb_bpp, FG_COLOR_DEFAULT, BG_COLOR_DEFAULT, NULL, NULL,
        (uint32_t *)context->getFbBuff(), user_malloc, user_free);

    terminal->clear(terminal);
}

Terminal::~Terminal() {}

void Terminal::drawObject() {
    context->drawBuff(getX(), getY(), getWidth(), getHeight(), terminal->buf1);
}

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

void Terminal::setFg(uint32_t color) {
    terminal->fg_color_default = color;
}

void Terminal::setBg(uint32_t color) {
    terminal->bg_color_default = color;
}

}
