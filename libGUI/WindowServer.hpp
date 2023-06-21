#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "ulibc/DevPoll.hpp"
#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"

#define WINDOW_MAX                          10
#define DEFAULT_COLOR                       0x00000000 // 0xed872d

namespace GUI {

class WindowServer {
public:
    static WindowServer *getInstance();
    Window *createWindow(const char *w_name, int x_pos, int y_pos, int width,
                         int height);

    Window *appendWindow(Window *window);
    Window *removeWindow(int windowID);

    void fillBackground(void);
    void refreshScreen(void);

    void mouseHandle(Device::MouseData *data);

private:
    WindowServer();
    ~WindowServer();

    void updateMousePos(Device::MouseData *data);
    bool mouseInBounds(Window *window);

    const int maxWindows;
    bool isImgLoaded;

    // Mouse data
    int mouseXPos;
    int mouseYPos;
    bool oldLeftState;
    Window *mouseWindow;

    // Selected window data
    Window *selectedWindow;

    // Event counter to facilitate refresh rate
    int nEvents;

    int numWindows;
    Window *windows[WINDOW_MAX];
    FbContext *context;

    static WindowServer *instance;
};

}