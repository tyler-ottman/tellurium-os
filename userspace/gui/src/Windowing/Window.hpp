#pragma once

#include <stdint.h>
#include "ulibc/DevPoll.hpp"

namespace GUI {

class Window: public Rect {

protected:
    char *windowName;

    int x;
    int y;
    int width;
    int height;

    int color;

    int windowID;
    void updateRect(void);

public:
    Window(const char *name, int x, int y, int width, int height);
    ~Window();

    bool intersects(Rect *rect);

    void updatePosition(int xNew, int yNew);

    void setWindowID(int windowID);
    int getWindowID(void);
    int getXPos(void);
    int getYPos(void);
    int getWidth(void);
    int getHeight(void);
    int getColor(void);

    void setXPos(int x);
    void setYPos(int y);

    void windowPaint(void);
};

}