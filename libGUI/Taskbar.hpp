#ifndef TASKBAR_H
#define TASKBAR_H

#include "Button.hpp"
#include "Window.hpp"

namespace GUI {

class Taskbar : public Window {
public:
    Taskbar(int x, int y, int width, int height,
            WindowFlags flags = WindowFlags_None,
            WindowPriority priority = WindowPriority_9);
    ~Taskbar();
};

}

#endif // TASKBAR_H