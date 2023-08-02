#pragma once

#include "libGUI/Desktop.hpp"
#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"
#include "libTellur/DevPoll.hpp"
#include <stdbool.h>
#include <stddef.h>

#define WINDOW_MAX                          10
#define DEFAULT_COLOR                       0x00000000 // 0xed872d

namespace GUI {

class WindowServer {
public:
    static WindowServer *getInstance();

    void refreshScreen(void);
    void mouseHandle(Device::MouseData *data);

    Window *createWindow(const char *name, int x, int y, int width, int height,
                         uint16_t flags);

   private:
    WindowServer();
    ~WindowServer();

    // Event counter to facilitate refresh rate
    int nEvents;

    Desktop *desktop;

    static WindowServer *instance;
};

}