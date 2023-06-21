#include "ulibc/mem.hpp"
#include "ulibc/string.h"
#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"

uint8_t pseudo_rand_8() {
    static uint16_t seed = 0;
    return (uint8_t)(seed = (12657 * seed + 12345) % 256);
}

namespace GUI {

void Window::updateRect() {
    Rect::setTop(y);
    Rect::setBottom(y + height - 1);
    Rect::setLeft(x);
    Rect::setRight(x + width - 1);
}

Window::Window(const char *w_name, int x, int y, int width, int height,
               uint16_t flags)
    : x(x),
      y(y),
      width(width),
      height(height),
      flags(flags),
      lastMouseState(0),
      windowID(-1),
      context(FbContext::getInstance()),
      parent(nullptr),
      maxWindows(WINDOW_MAX),
      numWindows(0),
      selectedWindow(nullptr),
      dragX(0),
      dragY(0) {
    color = 0xff000000 | pseudo_rand_8() << 16 | pseudo_rand_8() << 8 |
                  pseudo_rand_8();

    this->windowName = new char[__strlen(w_name)];

    for (int i = 0; i < maxWindows; i++) {
        windows[i] = nullptr;
    }

    updateRect();
}

Window::~Window() {

}

Window *Window::createWindow(const char *w_name, int x_pos, int y_pos,
                                   int width, int height) {
    Window *window = new Window(w_name, x_pos, y_pos, width, height, 0);
    if (!window) {
        return window;
    }

    if (!appendWindow(window)) {
        delete window;
        return nullptr;
    }

    return window;
}

Window *Window::appendWindow(Window *window) {
    if (numWindows == WINDOW_MAX) {
        return nullptr;
    }

    int windowID = numWindows++;
    windows[windowID] = window;
    window->setWindowID(windowID);

    return window;
}

Window *Window::removeWindow(int windowID) {
    if (windowID < 0 || windowID >= numWindows) {
        return nullptr;
    }

    Window *window = windows[windowID];

    for (int i = windowID; i < numWindows - 1; i++) {
        windows[i] = windows[i + 1];
        windows[i]->setWindowID(i);
    }
    
    windows[--numWindows] = nullptr;

    return window;
}

void Window::applyBoundClipping(bool recurse) {
    int screenX = getXPos();
    int screenY = getYPos();

    Rect rect;

    if ((!(flags & WIN_NODECORATION)) && recurse) {
        screenX += WIN_BORDERWIDTH;
        screenY += WIN_TITLEHEIGHT;
        rect = Rect(screenY,
                    screenY + height - WIN_TITLEHEIGHT - WIN_BORDERWIDTH - 1,
                    screenX, screenX + width - (2 * WIN_BORDERWIDTH) - 1);
    } else {
        rect =
            Rect(screenY, screenY + height - 1, screenX, screenX + width - 1);
    }

    if (!parent) {
        context->addClippedRect(&rect);
        return;
    }

    parent->applyBoundClipping(true);

    context->intersectClippedRect(&rect);

    // Get list of windows that intersect above current window
    for (int i = getWindowID() + 1; i < numWindows; i++) {
        Window *aboveWin = windows[i];
        if (intersects(aboveWin)) {
            context->reshapeRegion(aboveWin);
        }
    }
}

bool Window::intersects(Rect *rect) {
    return Rect::intersects(rect);
}

void Window::updatePosition(int xNew, int yNew) {
    x = xNew;
    y = yNew;

    updateRect();
}

void Window::setWindowID(int windowID) {
    this->windowID = windowID;
}

int  Window::getWindowID() {
    return this->windowID;
}

int Window::getXPos() {
    // int xTranslate = 0;

    // Window *curWindow = this;

    // while(curWindow->parent) {
    //     xTranslate += x;
    //     curWindow = curWindow->parent;
    // }

    // return xTranslate;

    return x;
}

int Window::getYPos() {
    // int yTranslate = 0;

    // Window *curWindow = this;

    // while(curWindow->parent) {
    //     yTranslate += y;
    //     curWindow = curWindow->parent;
    // }

    // return yTranslate;
    return y;
}

int Window::getWidth() {
    return this->width;
}

int Window::getHeight() {
    return this->height;
}

int Window::getColor() {
    return this->color;
}

void Window::setXPos(int x) {
    this->x = x;
}

void Window::setYPos(int y) {
    this->y = y;
}

void Window::windowPaint() {
    applyBoundClipping(false);

    if (!(flags & WIN_NODECORATION)) {
        context->intersectClippedRect(this);
    }

    for (int i = 0; i < numWindows; i++) {
        Window *child = windows[i];

        context->reshapeRegion(child);
    }

    context->drawRect(x, y, width, height, color);

    context->resetClippedList();
    
    // Call paint for child windows
    for (int i = 0; i < numWindows; i++) {
        windows[i]->windowPaint();
    }
}

}