#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "ulibc/DevPoll.hpp"

#define WINDOW_MAX                          10
#define TITLE_HEIGHT                        31
#define TITLE_WIDTH                         (width)
#define WIN_DECORATE                        1

#define BORDER_COLOR                        0xff333333
#define BORDER_WIDTH                        3

namespace GUI {

enum WindowType {
    WindowDefault,
    WindowButton,
};

class Window: public Rect {

public:
    Window(const char *name, int x, int y, int width, int height,
           uint16_t flags);
    ~Window();

    Window *createWindow(const char *w_name, int x_pos, int y_pos, int width,
                         int height, uint16_t flags);
    Window *appendWindow(Window *window);
    Window *removeWindow(int windowID);

    void applyBoundClipping(bool recurse);

    bool intersects(Rect *rect);
    void updatePosition(int xNew, int yNew);

    void drawWindow(void);
    
    // Mouse events
    bool onMouseEvent(Device::MouseData *data, int mouseX, int mouseY);
    bool onWindowStackTop(Window *win);
    bool onWindowDrag(Window *win, Device::MouseData *data);
    bool onWindowRelease(void);
    bool onWindowClick(void);
    
    bool mouseInBounds(int mouseX, int mouseY);
    bool isMovable(void) { return (flags & WIN_DECORATE); }
    bool isOnMenuBar(int mouseX, int mouseY);
    bool isLastMousePressed(void) { return (lastMouseState & 0x1); }

    int getWindowID(void);
    int getXPos(void);
    int getYPos(void);
    int getWidth(void);
    int getHeight(void);
    int getColor(void);
    void setWindowID(int windowID);
    void setXPos(int x);
    void setYPos(int y);

protected:
    void moveToTop(Window *window);
    void drawBorder(void);
    void updateRect(void);
    void updateChildPositions(Device::MouseData *data);

    char *windowName;
    int x;
    int y;
    int width;
    int height;
    uint16_t flags;
    int color;
    int windowID;
    int type;

    FbContext *context;
    Window *parent;

    Window *windows[WINDOW_MAX];
    const int maxWindows;
    int numWindows;
    
    static uint8_t lastMouseState;
    static Window *selectedWindow;
};

}