#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "ulibc/DevPoll.hpp"

#define WINDOW_MAX                          10
#define TITLE_HEIGHT                        31
#define TITLE_WIDTH                         (width)

#define WIN_DECORATE                        0x0001
#define WIN_MOVABLE                         0x0002
#define WIN_REFRESH_NEEDED                  0x0003

#define BORDER_COLOR                        0xff333333
#define BORDER_WIDTH                        3
#define MENUBAR_SELECT                      0xff545454

namespace GUI {

enum WindowType {
    WindowButton,
    WindowDefault,
    WindowMenuBar
};

class MenuBar;

class Window: public Rect {

public:
    Window(const char *name, int x, int y, int width, int height,
           uint16_t flags);
    ~Window();

    Window *createWindow(const char *w_name, int x_pos, int y_pos, int width,
                         int height, uint16_t flags);
    Window *appendWindow(Window *window);
    Window *removeWindow(int windowID);
    bool attachMenuBar(MenuBar *menuBar);

    void applyBoundClipping(bool recurse);
    void applyDirtyDrag(void);

    bool intersects(Rect *rect);
    void updatePosition(int xNew, int yNew);

    void drawWindow(void);
    
    // Mouse event stubs

    // Mouse events
    bool onMouseEvent(Device::MouseData *data, int mouseX, int mouseY);
    bool onWindowRaise(void);
    bool onWindowDrag(Device::MouseData *data);
    bool onWindowRelease(void);
    bool onWindowClick(void);
    bool onWindowSelect(void);
    bool onWindowUnselect(void);
    
    int getWindowID(void);
    int getXPos(void);
    int getYPos(void);
    int getWidth(void);
    int getHeight(void);
    int getColor(void);
    void setWindowID(int windowID);
    void setXPos(int x);
    void setYPos(int y);
    bool isLastMousePressed(void);
    bool isDecorable(void);
    bool isMovable(void);
    bool isRefreshNeeded(void);
    bool isOnMenuBar(int mouseX, int mouseY);
    bool isMouseInBounds(int mouseX, int mouseY);

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

    MenuBar *menuBar;

    FbContext *context;
    Window *parent;
    Window *activeChild;

    Window *windows[WINDOW_MAX];
    const int maxWindows;
    int numWindows;

    static uint8_t lastMouseState;

    // Info for window dragging and dirty regions
    static Window *selectedWindow;

    // Use (X, Y) position of selectedWindow on last refresh
    // to calculate dirty rectangles
    static int xOld;
    static int yOld;
};

class MenuBar : public Window {
public:
    MenuBar(int x, int y, int width, int height);
    ~MenuBar();

    void onMouseClick(void);
    void onBarSelect(void);
    void onBarUnselect(void);

    void drawMenuBar(void);

    uint32_t getBarColor(void);
private:
    void setBarColor(uint32_t color);

    uint32_t barColor;
};

}