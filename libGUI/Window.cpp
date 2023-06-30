#include "libGUI/Button.hpp"
#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"
#include "ulibc/mem.hpp"
#include "ulibc/string.h"

uint8_t pseudo_rand_8() {
    static uint16_t seed = 0;
    return (uint8_t)(seed = (12657 * seed + 12345) % 256);
}

namespace GUI {

uint8_t Window::lastMouseState = 0;
Window *Window::selectedWindow = nullptr;
int Window::xOld = 0;
int Window::yOld = 0;

Window::Window(const char *w_name, int x, int y, int width, int height,
               uint16_t flags)
    : x(x),
      y(y),
      width(width),
      height(height),
      flags(flags),
      windowID(-1),
      type(WindowDefault),
      menuBar(nullptr),
      context(FbContext::getInstance()),
      parent(nullptr),
      activeChild(nullptr),
      maxWindows(WINDOW_MAX),
      numWindows(0) {
    color = 0xff000000 | pseudo_rand_8() << 16 | pseudo_rand_8() << 8 |
            pseudo_rand_8();

    this->windowName = new char[__strlen(w_name)];

    for (int i = 0; i < maxWindows; i++) {
        windows[i] = nullptr;
    }

    updateRect();

    // Attach menu bar
    if (isDecorable()) {
        MenuBar *menuBar = new MenuBar(x + BORDER_WIDTH, y + BORDER_WIDTH,
                                       width - 2 * BORDER_WIDTH, TITLE_HEIGHT);

        if (menuBar) {
            attachMenuBar(menuBar);
        }
    }
}

Window::~Window() {}

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

bool Window::attachMenuBar(MenuBar *menuBar) {
    // Menu bar already exists
    if (this->menuBar) {
        return false;
    }

    appendWindow(menuBar);
    this->menuBar = menuBar;

    return true;
}

void Window::applyBoundClipping(bool recurse) {
    Rect rect;

    if (isDecorable() && recurse && parent) {
        // rect = Rect(y + TITLE_HEIGHT, y + height - BORDER_WIDTH - 1,
        //             x + BORDER_WIDTH, x + width - BORDER_WIDTH - 1);
        rect = Rect(y, y + height - 1, x, x + width - 1);
    } else {
        rect = Rect(y, y + height - 1, x, x + width - 1);
    }

    if (!parent) {
        if (context->getNumDirty()) {
            Rect *dirtyRegions = context->getDirtyRegions();

            for (int i = 0; i < context->getNumDirty(); i++) {
                context->addClippedRect(&dirtyRegions[i]);
            }

            context->intersectClippedRect(&rect);
        } else {
            context->addClippedRect(&rect);
        }

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

void Window::applyDirtyDrag() {
    if (!selectedWindow) {
        return;
    }

    context->resetClippedList();

    if (!(xOld == selectedWindow->getXPos() &&
          yOld == selectedWindow->getYPos())) {
        int tempX = selectedWindow->getXPos();
        int tempY = selectedWindow->getYPos();

        // Original window position
        selectedWindow->updatePosition(xOld, yOld);
        context->addClippedRect(selectedWindow);

        // New window position
        selectedWindow->updatePosition(tempX, tempY);
        context->addClippedRect(selectedWindow);

        xOld = selectedWindow->getXPos();
        yOld = selectedWindow->getYPos();

        context->moveClippedToDirty();
    }
}

bool Window::intersects(Rect *rect) { return Rect::intersects(rect); }

void Window::updatePosition(int xNew, int yNew) {
    x = xNew;
    y = yNew;

    updateRect();
}

bool Window::onMouseEvent(Device::MouseData *data, int mouseX, int mouseY) {
    bool isNewMousePressed = data->flags & 0x1;

    for (int i = numWindows - 1; i >= 0; i--) {
        Window *child = windows[i];

        if (!child->isMouseInBounds(mouseX, mouseY)) {
            continue;
        }

        // Pass event to child window
        return child->onMouseEvent(data, mouseX, mouseY);
    }

    // Window raise event
    if (isNewMousePressed && !isLastMousePressed() && isDecorable()) {
        return onWindowRaise();
    }

    // If here, event is on a leaf window
    if (this == selectedWindow && selectedWindow->isDecorable() &&
        isNewMousePressed && selectedWindow->isOnMenuBar(mouseX, mouseY)) {
        return selectedWindow->onWindowDrag(data);
    }

    // Window released from dragging
    if (this == selectedWindow && isLastMousePressed() && !isNewMousePressed) {
        return onWindowRelease();
    }

    // Window click event
    if (isNewMousePressed && !isLastMousePressed()) {
        return onWindowClick();
    }

    return true;
}

bool Window::onWindowRaise() {
    if (!parent) {
        return false;
    }

    Window *prevParent = parent;

    // Get top level window to stack on top
    while (prevParent->parent) {
        prevParent = prevParent->parent;
    }

    parent->removeWindow(windowID);
    parent->appendWindow(this);

    parent->activeChild = this;

    // Update unselected menu bar's color
    // if (selectedWindow && selectedWindow != this) {
    //     int xNew = selectedWindow->x + BORDER_WIDTH;
    //     int yNew = selectedWindow->y + TITLE_HEIGHT;
    //     Rect menuBar(
    //         yNew,
    //         yNew + selectedWindow->height - TITLE_HEIGHT - BORDER_WIDTH - 1,
    //         xNew, xNew + selectedWindow->width - 2 * BORDER_WIDTH - 1);

    //     context->addClippedRect(&menuBar);
    // }

    selectedWindow = this;

    if (selectedWindow->getXPos() == 175) {
        __asm__ ("cli");
    }

    xOld = selectedWindow->getXPos();
    yOld = selectedWindow->getYPos();

    // Window needs immediate refresh
    context->addClippedRect(selectedWindow);
    context->moveClippedToDirty();

    return true;
}

bool Window::onWindowDrag(Device::MouseData *data) {
    updateChildPositions(data);

    return true;
}

bool Window::onWindowRelease() {
    // Window needs immediate refresh
    if (this == selectedWindow) {
        selectedWindow->applyDirtyDrag();
    }

    return true;
}

bool Window::onWindowClick() {
    switch (type) {
        case GUI::WindowButton:
            ((Button *)this)->onMouseClick();
            break;
        case GUI::WindowMenuBar:
            // ((MenuBar *)this)->
            break;
        case GUI::WindowDefault:
            break;
    }

    return true;
}

void Window::drawWindow() {
    applyBoundClipping(false);

    if (isDecorable() && parent) {
        drawBorder();

        int xNew = x + BORDER_WIDTH;
        int yNew = y + TITLE_HEIGHT;
        Rect mainWindow(yNew, yNew + height - TITLE_HEIGHT - BORDER_WIDTH - 1,
                        xNew, xNew + width - 2 * BORDER_WIDTH - 1);

        context->intersectClippedRect(&mainWindow);
    }

    // Remove child window clipped rectangles
    for (int i = 0; i < numWindows; i++) {
        context->reshapeRegion(windows[i]);
    }

    // Draw self
    if (type == GUI::WindowDefault) {
        context->drawRect(x, y, width, height, color);
    } else if (type == GUI::WindowButton) {
        ((Button *)this)->drawWindow();
    }

    context->resetClippedList();

    Rect *dirtyRegions = context->getDirtyRegions();
    for (int i = 0; i < numWindows; i++) {
        context->resetClippedList();

        Window *child = windows[i];

        if (context->getNumDirty()) {
            bool isIntersect = false;
            for (int j = 0; j < context->getNumDirty(); j++) {
                Rect *dirtyRegion = &dirtyRegions[j];
                if (child->intersects(dirtyRegion)) {
                    isIntersect = true;
                    break;
                }
            }

            if (!isIntersect) {
                continue;
            }
        }

        child->drawWindow();
    }
}

int Window::getWindowID() { return this->windowID; }

int Window::getXPos() { return x; }

int Window::getYPos() { return y; }

int Window::getWidth() { return width; }

int Window::getHeight() { return height; }

int Window::getColor() { return color; }

void Window::setWindowID(int windowID) { this->windowID = windowID; }

void Window::setXPos(int x) { this->x = x; }

void Window::setYPos(int y) { this->y = y; }

bool Window::isLastMousePressed() { return lastMouseState & 0x1; }

bool Window::isDecorable() { return flags & WIN_DECORATE; }

bool Window::isRefreshNeeded() { return flags & WIN_REFRESH_NEEDED; }

bool Window::isOnMenuBar(int mouseX, int mouseY) {
    return (mouseY >= y && mouseY < (y + TITLE_HEIGHT) && mouseX >= x &&
            mouseX < (x + TITLE_WIDTH));
}

bool Window::isMouseInBounds(int mouseX, int mouseY) {
    return ((mouseX >= x) && (mouseX <= (x + width)) && (mouseY >= y) &&
            (mouseY <= (y + height)));
}

void Window::moveToTop(Window *win) {
    removeWindow(win->getWindowID());

    appendWindow(win);
}

void Window::drawBorder() {
    if (!parent) {
        return;
    }

    context->drawOutlinedRect(x, y, width, height, BORDER_COLOR);
    context->drawOutlinedRect(x + 1, y + 1, width - 2, height - 2,
                              BORDER_COLOR);
    context->drawOutlinedRect(x + 2, y + 2, width - 4, height - 4,
                              BORDER_COLOR);

    context->drawHorizontalLine(x + 3, y + 28, width - 6, BORDER_COLOR);
    context->drawHorizontalLine(x + 3, y + 29, width - 6, BORDER_COLOR);
    context->drawHorizontalLine(x + 3, y + 30, width - 6, BORDER_COLOR);

    // Menu bar
    context->drawRect(x + 3, y + 3, width - 6, 25,
                      parent->activeChild == this ? 0xff545454 : BORDER_COLOR);
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

MenuBar::MenuBar(int x, int y, int width, int height)
    : Window::Window("menuBar", x, y, width, height, 0) {
    type = GUI::WindowMenuBar;
}

MenuBar::~MenuBar() {}

}  // namespace GUI