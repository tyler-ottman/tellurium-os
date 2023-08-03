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

void Terminal::drawObject() {
    context->drawBuff(x, y, width, height, terminal->buf1);
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

Terminal::~Terminal() {}

}
