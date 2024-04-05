#include "MenuBar.hpp"

namespace GUI {

MenuBar::MenuBar(int x, int y, int width, int height, WindowFlags flags)
    : Window::Window("menuBar", x, y, width, height, flags),
    barColor(0xffbebebe) {
    setPriority(5);
}

MenuBar::~MenuBar() {}

bool MenuBar::onWindowClick() {
    return true;
}

bool MenuBar::onWindowSelect() {
    setBarColor(0xffbebebe);
    selectedWindow = parent;

    return true;
}

bool MenuBar::onWindowUnselect() {
    // setBarColor(0xffa9a9a9);
    setBarColor(0xff00ff00);
    context->addClippedRect(winRect);

    return true;
}

bool MenuBar::onWindowDrag(Device::MouseData *data) {
    parent->setChildPositions(data);

    return true;
}

void MenuBar::drawObject() {
    context->drawRect(getX(), getY(), getWidth(), 32, getBarColor());
}

uint32_t MenuBar::getBarColor() {
    return barColor;
}

void MenuBar::setBarColor(uint32_t color) {
    barColor = color;
}

};