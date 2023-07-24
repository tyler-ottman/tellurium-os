#include "Button.hpp"

namespace GUI {

Button::Button(int x, int y, int width, int height)
    : Window::Window("", x, y, width, height, 0), colorToggle(false) {
    type = WindowButton;
}

Button::~Button() {

}

void Button::onMouseClick() {
    colorToggle ^= 1;

    // Button dirty, needs refresh
    context->addClippedRect(this);
    context->moveClippedToDirty();
}

void Button::drawWindow() {
    color = colorToggle ? 0xff01796f : 0xffca3433;
    context->drawRect(x + 2, y + 2, width - 4, height - 4, color);

    context->drawOutlinedRect(x, y, width, height, 0xffff66cc);
    context->drawOutlinedRect(x + 1, y + 1, width - 2, height - 2, 0xffff66cc);
    
}

}
