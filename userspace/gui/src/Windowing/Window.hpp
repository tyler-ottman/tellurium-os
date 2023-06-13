#pragma once

#include <stdint.h>
#include "ulibc/DevPoll.hpp"

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
    int getXPos(void);
    int getYPos(void);
    int getWidth(void);
    int getHeight(void);

    void windowPaint(uint32_t color);
};

}