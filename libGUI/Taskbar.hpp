#ifndef TASKBAR_H
#define TASKBAR_H

#include "Button.hpp"
#include "Window.hpp"

namespace GUI {

class Taskbar : public Window {
public:
    Taskbar(int x, int y, int width, int height,
            WindowFlags flags = WindowFlags::WNONE,
            WindowPriority priority = WindowPriority::WPRIO9);
    ~Taskbar();
};

}

#endif // TASKBAR_H