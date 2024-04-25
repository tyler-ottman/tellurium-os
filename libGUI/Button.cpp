#include "Button.hpp"
#include "flibc/string.h"

namespace GUI {

Button::Button(int x, int y, int width, int height, WindowFlags flags,
               ButtonFlags bFlags, WindowPriority priority)
    : Window::Window("", x, y, width, height, flags, priority),
      imgNoHover(nullptr), imgHover(nullptr), buttonFlags(bFlags) {
    imgNoHover = (uint8_t *)surface->buff;
}

Button::Button(int x, int y, ImageReader *img, WindowFlags flags,
               ButtonFlags bFlags, WindowPriority priority)
    : Window::Window("", x, y, img, flags, priority),
      imgNoHover(nullptr),
      imgHover(nullptr),
      buttonFlags(bFlags) {
    imgNoHover = (uint8_t *)surface->buff;
}

Button::~Button() {}

bool Button::onWindowHover() {
    if (!isFlagHover()) {
        return false;
    }

    if (imgHover) {
        surface->buff = (uint32_t *)imgHover;
        setDirty(true);
    }

    return true;
}

bool Button::onWindowUnhover() {
    if (!isFlagHover()) {
        return false;
    }

    surface->buff = (uint32_t *)imgNoHover;
    setDirty(true);

    return true;
}

void Button::loadHoverImage(ImageReader *img) {
    if (img->getWidth() == getWidth() && img->getHeight() == getHeight()) {
        int nBytes = (img->getBpp() / 8) * img->getWidth() * img->getHeight();
        imgHover = new uint8_t[nBytes];
        __memcpy((void *)imgHover, img->getBuff(), nBytes);
    }
}

}
