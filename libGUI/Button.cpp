#include "Button.hpp"

namespace GUI {

Button::Button(int x, int y, int width, int height, int buttonFlags)
    : Window::Window("", x, y, width, height, 0),
      colorToggle(false),
      onHover(false),
      isImg(false),
      imgDefault(nullptr),
      imgHover(nullptr),
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

void Button::drawObject() {
    color = colorToggle ? 0xff01796f : 0xffca3433;

    if (onHover) {
        color = context->translateLightColor(this->color);
    }

    // Draw window border if flag set
    if (isFlagBorder()) {
        context->drawRect(winRect->getX() + 2, winRect->getY() + 2,
                          winRect->getWidth() - 4, winRect->getHeight() - 4,
                          color);
        context->drawOutlinedRect(winRect->getX(), winRect->getY(),
                                  winRect->getWidth(), winRect->getHeight(),
                                  0xffff66cc);
        context->drawOutlinedRect(winRect->getX() + 1, winRect->getY() + 1,
                                  winRect->getWidth() - 2,
                                  winRect->getHeight() - 2, 0xffff66cc);
    }

    if (isImg) {
        Image *image = onHover ? imgHover : imgDefault;
        context->drawBuff(winRect->getX(), winRect->getY(), winRect->getWidth(),
                          winRect->getHeight(), image->getBuff());
    } else {
        context->drawRect(winRect->getX(), winRect->getY(), winRect->getWidth(),
                          winRect->getHeight(), color);
    }
}

void Button::loadImage(const char *path) {
    GUI::Image *image = new GUI::Image(winRect->getX(), winRect->getY(), winRect->getWidth(), winRect->getHeight());
    int err = image->loadImage(path);
    if (err) {
        __asm__("cli");
        return;
    }

    imgDefault = image;
    if (!imgHover) {
        imgHover = imgDefault;
    }

    isImg = true;

    winRect->setWidth(image->getWidth());
    winRect->setHeight(image->getHeight());
    updateRect();
}

void Button::loadHoverImage(const char *path) {
    GUI::Image *image = new GUI::Image(winRect->getX(), winRect->getY(),
                                       winRect->getWidth(),
                       winRect->getHeight());
    int err = image->loadImage(path);
    if (err) {
        return;
    }

    imgHover = image;

    winRect->setWidth(image->getWidth());
    winRect->setHeight(image->getHeight());
    updateRect();
}

}
