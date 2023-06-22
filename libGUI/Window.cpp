#include "ulibc/mem.hpp"
#include "ulibc/string.h"
#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"

uint8_t pseudo_rand_8() {
    static uint16_t seed = 0;
    return (uint8_t)(seed = (12657 * seed + 12345) % 256);
}

namespace GUI {

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
                             int width, int height, uint16_t flags) {
    Window *window = new Window(w_name, x_pos, y_pos, width, height, flags);
    if (!window) {
        return window;
    }

    if (!appendWindow(window)) {
        delete window;
        return nullptr;
    }

    window->parent = this;
    window->flags = WIN_DECORATE;

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

    if ((flags & WIN_DECORATE) && recurse) {
        rect = Rect(screenY,
                    screenY + height - TITLE_HEIGHT - BORDER_WIDTH - 1,
                    screenX, screenX + width - (2 * BORDER_WIDTH) - 1);
        // rect =
        //     Rect(screenY, screenY + height - 1, screenX, screenX + width - 1);
    } else {
        rect =
            Rect(screenY, screenY + height - 1, screenX, screenX + width - 1);
    }

    if (!parent) {
        context->addClippedRect(&rect);
        return;
    }

    // Reduce drawing to parent window
    parent->applyBoundClipping(true);

    // Reduce visibility to main drawing area
    context->intersectClippedRect(&rect);

    // Get ID of current window from parent's view
    int winID = -1;
    for (int i = 0; i < parent->numWindows; i++) {
        if (parent->windows[i] == this) {
            winID = i;
            break;
        }
    }

    // Occlude areas of window where siblings overlap on top   
    for (int i = winID + 1; i < parent->numWindows; i++) {
        Window *aboveWin = parent->windows[i];
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

void Window::drawWindow() {
    context->resetClippedList();

    applyBoundClipping(false);

    for (int i = 0; i < numWindows; i++) {
        context->reshapeRegion(windows[i]);
    }

    if (flags & WIN_DECORATE) {
        drawBorder();

        int xNew = x + BORDER_WIDTH;
        int yNew = y + TITLE_HEIGHT;
        Rect mainWindow(yNew,
                        yNew + height - TITLE_HEIGHT - BORDER_WIDTH - 1,
                        xNew, xNew + width - 2 * BORDER_WIDTH - 1);

        context->intersectClippedRect(&mainWindow);
    }

    if (context->getRegions() != 0) {
        context->drawRect(x, y, width, height, color);
    }
    
    // Call paint for child windows
    for (int i = 0; i < numWindows; i++) {
        windows[i]->drawWindow();
    }
}

void Window::setWindowID(int windowID) {
    this->windowID = windowID;
}

int  Window::getWindowID() {
    return this->windowID;
}

int Window::getXPos() {
    return x;
}

int Window::getYPos() {
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

void Window::drawBorder() {
    context->drawOutlinedRect(x, y, width, height, BORDER_COLOR);
    context->drawOutlinedRect(x + 1, y + 1, width - 2, height - 2, BORDER_COLOR);
    context->drawOutlinedRect(x + 2, y + 2, width - 4, height - 4, BORDER_COLOR);

    context->drawHorizontalLine(x + 3, y + 28, width - 6, BORDER_COLOR);
    context->drawHorizontalLine(x + 3, y + 29, width - 6, BORDER_COLOR);
    context->drawHorizontalLine(x + 3, y + 30, width - 6, BORDER_COLOR);
    context->drawRect(x + 3, y + 3, width - 6, 25, BORDER_COLOR);
}

void Window::updateRect() {
    Rect::setTop(y);
    Rect::setBottom(y + height - 1);
    Rect::setLeft(x);
    Rect::setRight(x + width - 1);
}

}