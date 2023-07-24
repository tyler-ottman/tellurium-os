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

    // Menu bar can only be attached to movable windows
    if (isMovable()) {
        MenuBar *menuBar = new MenuBar(x, y, width, TITLE_HEIGHT);

        attachMenuBar(menuBar);
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

bool Window::onMouseEvent(Device::MouseData *data, int mouseX,
                                 int mouseY) {
    bool isNewMousePressed = data->flags & 0x1;

    for (int i = numWindows - 1; i >= 0; i--) {
        Window *child = windows[i];

        if (!child->isMouseInBounds(mouseX, mouseY)) {
            continue;
        }

        // Pass event to child window
        return child->onMouseEvent(data, mouseX, mouseY);
    }

    // Window raise event (non-terminal event)
    if (isNewMousePressed && !isLastMousePressed() && (isMovable() ||
        type == GUI::WindowMenuBar)) {
        Window *windowRaise = type == WindowMenuBar ? parent : this;
        windowRaise->onWindowRaise();
    }

    // Terminal events

    // Event on menu bar, drag its window
    if (isNewMousePressed && type == GUI::WindowMenuBar) {
        return parent->onWindowDrag(data);
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

    Window *topWindow = this;
    Window *prevWindow = topWindow;

    // Get top level window to stack on top
    while (topWindow->parent) {
        prevWindow = topWindow;
        topWindow = topWindow->parent;
    }

    // If desktop was clicked, ignore
    if (topWindow == prevWindow) {
        return true;
    }

    // Otherwise, raise window
    topWindow->removeWindow(prevWindow->getWindowID());
    topWindow->appendWindow(prevWindow);
    topWindow->activeChild = prevWindow;

    // Unselect old window
    if (selectedWindow && (selectedWindow != this)) {
        selectedWindow->onWindowUnselect();
    }

    // Select new window
    onWindowSelect();
    selectedWindow = this;

    xOld = selectedWindow->getXPos();
    yOld = selectedWindow->getYPos();

    // Window needs immediate refresh
    context->addClippedRect(prevWindow);
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
            ((MenuBar *)this)->onMouseClick();
            break;
        case GUI::WindowDefault:
            break;
    }

    return true;
}

bool Window::onWindowSelect() {
    if (menuBar) {
        menuBar->onBarSelect();
    }

    return true;
}

bool Window::onWindowUnselect() {
    if (menuBar) {
        menuBar->onBarUnselect();
    }

    return true;
}

void Window::drawWindow() {
    applyBoundClipping(false);

    // Remove child window clipped rectangles
    for (int i = 0; i < numWindows; i++) {
        context->reshapeRegion(windows[i]);
    }

    // Draw self
    if (type == GUI::WindowDefault) {
        context->drawRect(x, y, width, height, color);
    } else if (type == GUI::WindowButton) {
        ((Button *)this)->drawWindow();
    } else if (type == GUI::WindowMenuBar && parent) {
        ((MenuBar *)this)->drawMenuBar();
    }

    context->resetClippedList();

    // Redraw children if they intersect with dirty region
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

bool Window::isMovable() { return flags & WIN_MOVABLE; }

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
    : Window::Window("menuBar", x, y, width, height, 0),
    barColor(BORDER_COLOR) {
    type = GUI::WindowMenuBar;
}

MenuBar::~MenuBar() {}

void MenuBar::onMouseClick() {

}

void MenuBar::onBarSelect() {
    setBarColor(MENUBAR_SELECT);
}

void MenuBar::onBarUnselect() {
    setBarColor(BORDER_COLOR);
    context->addClippedRect(this);

    // close drop down menus
}

void MenuBar::drawMenuBar() {
    context->drawRect(x, y, width, 32, getBarColor());
}

uint32_t MenuBar::getBarColor() {
    return barColor;
}

void MenuBar::setBarColor(uint32_t color) {
    barColor = color;
}

}  // namespace GUI
