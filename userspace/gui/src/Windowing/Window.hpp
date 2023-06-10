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

    int windowID;
public:
    Window(const char *w_name, int xPos, int yPos, int width, int height);
    ~Window();

    void setWindowID(int windowID);
    int getWindowID(void);
    void windowPaint(uint32_t color);
};

}