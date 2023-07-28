#include "Taskbar.hpp"

namespace GUI {

Taskbar::Taskbar(int x, int y, int width, int height)
    : Window::Window("taskbar", x, y, width, height, WIN_DECORATE) {
    setPriority(9);

    Border *border = new Border(x, y, width, 1);
    border->setColor(0);
    appendWindow(border);
}

Taskbar::~Taskbar() {}

void Taskbar::drawObject(void) {
    Window::drawObject();
}

}