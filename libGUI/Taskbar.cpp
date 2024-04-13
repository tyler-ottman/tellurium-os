#include "Border.hpp"
#include "Taskbar.hpp"

namespace GUI {

Taskbar::Taskbar(int x, int y, int width, int height, WindowFlags flags,
                 WindowPriority priority)
    : Window::Window("taskbar", x, y, width, height, flags, priority) {
    Border *border = new Border(x, y, width, 1);
    border->setColor(0);
    appendWindow(border);
}

Taskbar::~Taskbar() {}

}