#include "Border.hpp"

namespace GUI {

Border::Border(int x, int y, int width, int height, WindowFlags flags)
    : Window::Window("border", x, y, width, height, flags) {
    priority = 2;
    color = 0xffbebebe;
}

Border::~Border() {}

void Border::drawObject() {
    context->drawRect(getX(), getY(), getWidth(), getHeight(), color);
}

};