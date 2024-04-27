#pragma once

#include "modules/terminal.h"
#include "Window.hpp"

namespace GUI {

class Terminal : public Window {
public:
    Terminal(int x, int y, int width, int height,
             WindowFlags flags = WindowFlags_None,
             WindowPriority priority = WindowPriority_2);
    ~Terminal();

    // Buffer opeations
    void clear(void);
    int printf(const char *format, ...);

    void enableCursor(void);
    void disableCursor(void);
    void setScroll(bool enable);

    void setFg(uint32_t color);
    void setBg(uint32_t color);
private:
    terminal_t *terminal;
};

}