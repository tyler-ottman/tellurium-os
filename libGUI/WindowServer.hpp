#pragma once

#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"
#include "libTellur/DevPoll.hpp"
#include <stdbool.h>
#include <stddef.h>

#define DEFAULT_COLOR                       0x00000000 // 0xed872d

namespace GUI {

class WindowServer {
public:
    static WindowServer *getInstance();

    void mouseHandle(Device::MouseData *data);

private:
    WindowServer();
    ~WindowServer();

    // Desktop *desktop;

    static WindowServer *instance;
};

}