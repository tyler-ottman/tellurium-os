#include "MenuBar.hpp"

namespace GUI {

MenuBar::MenuBar(int x, int y, int width, int height, WindowFlags flags,
                 WindowPriority priority)
    : Window::Window("menuBar", x, y, width, height, flags, priority) {
    loadBuff(0xffbebebe);
}

MenuBar::~MenuBar() {}

bool MenuBar::onWindowDrag(Device::MouseData *data) {
    parent->updateChildPositions(data);
    parent->setDirty(true);

    return true;
}

uint32_t MenuBar::getBarColor() {
    return barColor;
}

void MenuBar::setBarColor(uint32_t color) {
    barColor = color;
}

};