#pragma once

#include "Button.hpp"
#include "Window.hpp"

#define TASKBAR_BUTTONS                     15

namespace GUI {

class Taskbar : public Window {
public:
    Taskbar(int x, int y, int width, int height);
    ~Taskbar();

private:
    Button *buttons[TASKBAR_BUTTONS];
};

}