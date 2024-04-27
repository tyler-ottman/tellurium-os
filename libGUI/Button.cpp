#include "Button.hpp"
#include "flibc/string.h"

namespace GUI {

Button::Button(int x, int y, int width, int height, WindowFlags flags,
               ButtonFlags bFlags, WindowPriority priority)
    : Window::Window("", x, y, width, height, flags, priority),
      imgNoHover(nullptr), imgHover(nullptr), buttonFlags(bFlags) {
    imgNoHover = (uint8_t *)surface->buff;
}

Button::Button(int x, int y, const char *path, WindowFlags flags,
               ButtonFlags bFlags, WindowPriority priority)
    : Window::Window("", x, y, path, flags, priority),
      imgNoHover(nullptr),
      imgHover(nullptr),
      buttonFlags(bFlags) {
    imgNoHover = (uint8_t *)surface->buff;
}

Button::~Button() {}

bool Button::onWindowHover() {
    if (!getFlag(ButtonFlags_Hover)) {
        return false;
    }

    if (imgHover) {
        surface->buff = (uint32_t *)imgHover;
        setFlags(WindowFlags_Dirty);
    }

    return true;
}

bool Button::onWindowUnhover() {
    if (!getFlag(ButtonFlags_Hover)) {
        return false;
    }

    surface->buff = (uint32_t *)imgNoHover;
    setFlags(WindowFlags_Dirty);

    return true;
}

void Button::loadHoverImage(const char *path) {
    ImageReader *img = imageReaderDriver(path);

    if (img->getWidth() == getWidth() && img->getHeight() == getHeight()) {
        int nBytes = (img->getBpp() / 8) * img->getWidth() * img->getHeight();
        imgHover = new uint8_t[nBytes];
        __memcpy((void *)imgHover, img->getBuff(), nBytes);
    }

    delete img;
}

}
