#include "Border.hpp"
#include "Taskbar.hpp"

namespace GUI {

Taskbar::Taskbar(int x, int y, int width, int height, WindowFlags flags,
                 WindowPriority priority)
    : Window::Window("taskbar", x, y, width, height, flags, priority) {
    appendWindow(new Border(x, y, width, 1));
}

Taskbar::~Taskbar() {}

}