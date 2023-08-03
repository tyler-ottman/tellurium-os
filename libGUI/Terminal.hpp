#pragma once

#include "modules/terminal.h"
#include "Window.hpp"

namespace GUI {

class Terminal : public Window {
public:
    Terminal(int x, int y, int width, int height);
    ~Terminal();

    void drawObject(void);
    int printf(const char *format, ...);
private:
    terminal_t *terminal;
};

}