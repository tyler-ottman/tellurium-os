#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "ulibc/DevPoll.hpp"
#include "libGUI/Desktop.hpp"
#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"

#define WINDOW_MAX                          10
#define DEFAULT_COLOR                       0x00000000 // 0xed872d

namespace GUI {

class WindowServer {
public:
    static WindowServer *getInstance();

    void refreshScreen(void);
    void mouseHandle(Device::MouseData *data);

    void createWindow(const char *name, int x, int y, int width, int height);
private:
    WindowServer();
    ~WindowServer();

    // Event counter to facilitate refresh rate
    int nEvents;

    Desktop *desktop;

    static WindowServer *instance;
};

}