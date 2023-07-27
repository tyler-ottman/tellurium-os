#include "Taskbar.hpp"

namespace GUI {

Taskbar::Taskbar(int x, int y, int width, int height)
    : Window::Window("taskbar", x, y, width, height, WIN_DECORATE) {
    for (int i = 0; i < TASKBAR_BUTTONS; i++) {
        buttons[i] = nullptr;
    }
}

Taskbar::~Taskbar() {}

}