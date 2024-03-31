#pragma once

#include "modules/terminal.h"
#include "Window.hpp"

namespace GUI {

class Terminal : public Window {
public:
    Terminal(int x, int y, int width, int height,
             WindowFlags flags = WindowFlags::WNONE);
    ~Terminal();

    // Buffer opeations
    void drawObject(void);
    void clear(void);
    int printf(const char *format, ...);


    void enableCursor(void);
    void disableCursor(void);

    void setFg(uint32_t color);
    void setBg(uint32_t color);
private:
    terminal_t *terminal;
};

}