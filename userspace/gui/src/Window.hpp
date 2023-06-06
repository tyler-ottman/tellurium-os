#pragma once

#include <stdint.h>

namespace GUI {

class Window {

private:
    char *windowName;

    int xPos;
    int yPos;
    int width;
    int height;

    Window(const char *w_name, int xPos, int yPos, int width, int height);
    ~Window();
};

}