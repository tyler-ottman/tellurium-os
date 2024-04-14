#include "Button.hpp"
#include "Utility.hpp"

namespace GUI {

Button::Button(int x, int y, int width, int height, WindowFlags flags,
               ButtonFlags bFlags, WindowPriority priority)
    : Window::Window("", x, y, width, height, flags, priority),
      colorToggle(false),
      onHover(false),
      isImg(false),
      imgDefault(nullptr),
      imgHover(nullptr),
      buttonFlags(bFlags) {}

Button::~Button() {

}

bool Button::onWindowClick() {
    if (isFlagToggle()) {
        colorToggle ^= 1;

        // Button dirty, needs refresh
        setDirty(true);
    }
    
    return true;
}

bool Button::onWindowHover() {
    if (!isFlagHover()) {
        return false;
    }

    onHover = true;
    setDirty(true);

    return true;
}

bool Button::onWindowUnhover() {
    onHover = false;
    setDirty(true);

    return true;
}

void Button::drawObject() {
    color = colorToggle ? 0xff01796f : 0xffca3433;

    if (onHover) {
        color = translateLightColor(this->color);
    }

    // Draw window border if flag set
    // if (isFlagBorder()) {
    //     context->drawRect(getX() + 2, getY() + 2, getWidth() - 4,
    //                       getHeight() - 4, color);
    //     context->drawOutlinedRect(getX(), getY(), getWidth(), getHeight(),
    //                               0xffff66cc);
    //     context->drawOutlinedRect(getX() + 1, getY() + 1, getWidth() - 2,
    //                               getHeight() - 2, 0xffff66cc);
    // }

    if (isImg) {
        Image *image = onHover ? imgHover : imgDefault;
        context->drawBuff(*winRect, image->getBuff());
    } else {
        context->drawRect(*winRect, color);
    }
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
    }

    isImg = true;

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
