#include "ulibc/mem.hpp"
#include "ulibc/string.h"
#include "libGUI/Button.hpp"
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
      type(WindowDefault),
      context(FbContext::getInstance()),
      parent(nullptr),
      maxWindows(WINDOW_MAX),
      numWindows(0),
      dragWindow(nullptr),
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

    return window;
}

Window *Window::appendWindow(Window *window) {
    if (numWindows == WINDOW_MAX) {
        return nullptr;
    }

    int windowID = numWindows++;
    windows[windowID] = window;
    window->setWindowID(windowID);
    window->parent = this;

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
    Rect rect;

    if (isMovable() && recurse && parent) {
        rect = Rect(y + TITLE_HEIGHT, y + height - BORDER_WIDTH - 1,
                    x + BORDER_WIDTH, x + width - BORDER_WIDTH - 1);
    } else {
        rect = Rect(y, y + height - 1, x, x + width - 1);
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

bool Window::onMouseEvent(Device::MouseData *data, int mouseX, int mouseY) {
    bool isNewMousePressed = data->flags & 0x1;

    for (int i = numWindows - 1; i >= 0; i--) {
        Window *child = windows[i];

        if (!child->mouseInBounds(mouseX, mouseY)) {
            continue;
        }

        // Non-terminal reached, window select event
        if (isNewMousePressed && !isLastMousePressed() && !dragWindow &&
            child->isMovable()) {
            onWindowStackTop(child);
        }

        // Terminal reached, window drag event
        if (dragWindow && child->isMovable() && isNewMousePressed &&
            isLastMousePressed() && child->isOnMenuBar(mouseX, mouseY)) {
            return onWindowDrag(child, data);
        }

        // Pass event to child window
        child->onMouseEvent(data, mouseX, mouseY);
    }

    // If here, event is on a leaf window

    // Window released from dragging
    if (isLastMousePressed() && !isNewMousePressed) {
        return onWindowRelease();
    }

    // Window click event
    if (isNewMousePressed && !isLastMousePressed()) {
        return onWindowClick();
    }

    return true;
}

bool Window::onWindowStackTop(Window *win) {
    moveToTop(win);

    dragWindow = win;

    return true;
}

bool Window::onWindowDrag(Window *win, Device::MouseData *data) {
    win->updateChildPositions(data);

    return true;
}

bool Window::onWindowRelease() {
    dragWindow = nullptr;
    lastMouseState &= ~(1);

    return true;
}

bool Window::onWindowClick() {
    switch (type) {
    case GUI::WindowButton: ((Button *)this)->onMouseClick(); break;
    case GUI::WindowDefault: break;
    }

    lastMouseState |= 0x1;

    return true;
}

bool Window::mouseInBounds(int mouseX, int mouseY) {
    return ((mouseX >= x) && (mouseX <= (x + width)) && (mouseY >= y) &&
            (mouseY <= (y + height)));
}

bool Window::isOnMenuBar(int mouseX, int mouseY) {
    return (mouseY >= y && mouseY < (y + TITLE_HEIGHT) && mouseX >= x &&
            mouseX < (x + TITLE_WIDTH));
}

void Window::drawWindow() {
    // context->resetClippedList();

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

    if (type == GUI::WindowDefault) {
        context->drawRect(x, y, width, height, color);
    } else if (type == GUI::WindowButton) {
        ((Button *)this)->drawWindow();
    }
    
    context->resetClippedList();
    
    // Call paint for child windows
    for (int i = 0; i < numWindows; i++) {
        windows[i]->drawWindow();
    }
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

void Window::setWindowID(int windowID) {
    this->windowID = windowID;
}

void Window::setXPos(int x) {
    this->x = x;
}

void Window::setYPos(int y) {
    this->y = y;
}

void Window::moveToTop(Window *win) {
    removeWindow(win->getWindowID());
    
    appendWindow(win);
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

void Window::updateChildPositions(Device::MouseData *data) {
    for (int i = 0; i < numWindows; i++) {
        windows[i]->updateChildPositions(data);
    }

    x += data->delta_x;
    y -= data->delta_y;

    updateRect();
}

}