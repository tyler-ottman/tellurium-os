#include "libGUI/Desktop.hpp"

namespace GUI {

Desktop::Desktop()
    : Window("desktop", 0, 0,
             FbContext::getInstance()->getFbContext()->fb_width,
             FbContext::getInstance()->getFbContext()->fb_height,
             0) {
    mouseX = width / 2;
    mouseY = height / 2;
}

Desktop::~Desktop() {

}

void Desktop::drawWindow() {
    Window::drawWindow();

    // Draw mouse
    context->drawRectNoRegion(mouseX, mouseY, 10, 10, 0xffffffff);
}

void Desktop::onMouseEvent(Device::MouseData *data) {
    Window::onMouseEvent(data, mouseX, mouseY);

    updateMousePos(data);
}

void Desktop::updateMousePos(Device::MouseData *data) {
    int xNew = mouseX + data->delta_x;
    int yNew = mouseY - data->delta_y;

    FbMeta *meta = FbContext::getInstance()->getFbContext();
    if (xNew >= 0 && xNew < (int)meta->fb_width) {
        mouseX = xNew;
    }

    if (yNew >= 0 && yNew < (int)meta->fb_height) {
        mouseY = yNew;
    }
}

bool Desktop::mouseInBounds(Window *window) {
    return ((mouseX >= window->getXPos()) &&
            (mouseX <= (window->getXPos() + window->getWidth())) &&
            (mouseY >= window->getYPos()) &&
            (mouseY <= (window->getYPos() + window->getHeight())));
}
}