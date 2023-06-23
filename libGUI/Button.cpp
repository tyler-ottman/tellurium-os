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
}

void Button::drawWindow() {
    color = colorToggle ? 0xff01796f : 0xffca3433;
    context->drawRect(x, y, width, height, color);
}

}
