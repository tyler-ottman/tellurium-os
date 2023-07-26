#include "Button.hpp"

namespace GUI {

Button::Button(int x, int y, int width, int height, int buttonFlags)
    : Window::Window("", x, y, width, height, 0),
      colorToggle(false),
      onHover(false),
      buttonFlags(buttonFlags) {
    type = WindowButton;
}

Button::~Button() {

}

bool Button::onMouseClick() {
    if (isFlagToggle()) {
        colorToggle ^= 1;

        // Button dirty, needs refresh
        moveThisToDirty();
    }
    
    return true;
}

bool Button::onButtonHover() {
    if (!isFlagHover()) {
        return false;
    }

    onHover = true;

    moveThisToDirty();

    return true;
}

bool Button::onButtonUnhover() {
    onHover = false;

    moveThisToDirty();

    return true;
}

void Button::drawWindow() {
    color = colorToggle ? 0xff01796f : 0xffca3433;

    if (onHover) {
        color = context->translateLightColor(this->color);
    }

    if (isFlagBorder()) {
        context->drawRect(x + 2, y + 2, width - 4, height - 4, color);
        context->drawOutlinedRect(x, y, width, height, 0xffff66cc);
        context->drawOutlinedRect(x + 1, y + 1, width - 2, height - 2, 0xffff66cc);
    } else {
        context->drawRect(x, y, width, height, color);
    }
}

}
