#include "MenuBar.hpp"
#include "libGUI/FbContext.hpp"

namespace GUI {

MenuBar::MenuBar(int x, int y, int width, int height, WindowFlags flags,
                 WindowPriority priority)
    : Window::Window("menuBar", x, y, width, height, flags, priority) {
    // barColor = 0xffbebebe;
    setColor(0xffbebebe);
}

MenuBar::~MenuBar() {}

bool MenuBar::onWindowClick() {
    return true;
}

bool MenuBar::onWindowSelect() {
    return true;
}

bool MenuBar::onWindowUnselect() {
    return true;
}

bool MenuBar::onWindowDrag(Device::MouseData *data) {
    parent->updateChildPositions(data);
    parent->setDirty(true);

    return true;
}

void MenuBar::drawObject() {
    FbContext *context = FbContext::getInstance();
    context->drawBuff(*winRect, winBuff);
}

uint32_t MenuBar::getBarColor() {
    return barColor;
}

void MenuBar::setBarColor(uint32_t color) {
    barColor = color;
}

};