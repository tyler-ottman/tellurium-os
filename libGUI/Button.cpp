#include "Button.hpp"

namespace GUI {

Button::Button(int x, int y, int width, int height, WindowFlags flags,
               ButtonFlags bFlags, WindowPriority priority)
    : Window::Window("", x, y, width, height, flags, priority),
      imgDefault(nullptr),
      imgHover(nullptr),
      buttonFlags(bFlags) {}

Button::~Button() {

}

bool Button::onWindowHover() {
    if (!isFlagHover()) {
        return false;
    }

    winBuff = imgHover->getBuff();
    setDirty(true);

    return true;
}

bool Button::onWindowUnhover() {
    if (!isFlagHover()) {
        return false;
    }

    winBuff = imgDefault->getBuff();
    setDirty(true);

    return true;
}

void Button::loadImage(const char *path) {
    GUI::Image *image = new GUI::Image(getX(), getY(), getWidth(), getHeight());
    int err = image->loadImage(path);
    if (err) {
        return;
    }

    imgDefault = image;
    if (!imgHover) {
        imgHover = imgDefault;
        winBuff = imgHover->getBuff();
    }

    setWidth(image->getWidth());
    setHeight(image->getHeight());
}

void Button::loadHoverImage(const char *path) {
    GUI::Image *image = new GUI::Image(getX(), getY(), getWidth(), getHeight());
    int err = image->loadImage(path);
    if (err) {
        return;
    }

    imgHover = image;

    setWidth(image->getWidth());
    setHeight(image->getHeight());
}

}
