#pragma once

#include <stdint.h>
#include "ulibc/DevPoll.hpp"

#define WINDOW_MAX                          10
#define WIN_TITLEHEIGHT                     31 
#define WIN_BORDERWIDTH                     3
#define WIN_NODECORATION                    1

namespace GUI {

class Window: public Rect {

public:
    Window(const char *name, int x, int y, int width, int height,
           uint16_t flags);
    ~Window();

    Window *createWindow(const char *w_name, int x_pos, int y_pos, int width,
                         int height);
    Window *appendWindow(Window *window);
    Window *removeWindow(int windowID);

    void applyBoundClipping(bool recurse);

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
    

protected:
    void updateRect(void);

    char *windowName;
    int x;
    int y;
    int width;
    int height;
    uint16_t flags;
    uint8_t lastMouseState;
    int color;
    int windowID;

    FbContext *context;
    Window *parent;

    Window *windows[WINDOW_MAX];
    const int maxWindows;
    int numWindows;
    
    Window *selectedWindow;
    int dragX;
    int dragY;
};

}