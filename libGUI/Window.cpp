#include "flibc/string.h"
#include "libGUI/Border.hpp"
#include "libGUI/Button.hpp"
#include "libGUI/FbContext.hpp"
#include "libGUI/Image.hpp"
#include "libGUI/MenuBar.hpp"
#include "libGUI/Terminal.hpp"
#include "libGUI/Window.hpp"
#include "libTellur/mem.hpp"

namespace GUI {

uint8_t Window::lastMouseState = 0;
Window *Window::selectedWindow = nullptr;
Window *Window::hoverWindow = nullptr;
int Window::xOld = 0;
int Window::yOld = 0;

Window::Window(const char *windowName, int x, int y, int width, int height,
               WindowFlags flags)
    : windowID(-1),
      flags(flags),
      type(WindowDefault),
      priority(2),
      parent(nullptr),
      numWindows(0),
      maxWindows(WINDOW_MAX),
      context(FbContext::getInstance()),
      activeChild(nullptr) {
    color = 0xff333333;

    if (windowName) {
        int len = __strlen(windowName);
        this->windowName = new char[len + 1];
        __memcpy(this->windowName, windowName, len);
        this->windowName[len] = '\0';
    }

    winRect = new Rect(x, y, width, height);

    for (int i = 0; i < maxWindows; i++) {
        windows[i] = nullptr;
    }

    // Menu bar can only be attached to movable windows
    if (hasDecoration()) {
        MenuBar *menuBar = new MenuBar(x, y, width, TITLE_HEIGHT);

        if (this->windowName) {
            int titleLen = __strlen(this->windowName) * 8;  // 8 is default font width
            Terminal *title =
                new Terminal(x + width / 2 - titleLen / 2, y + 10, titleLen, 30);
            title->disableCursor();
            title->setBg(0xffbebebe);
            title->setFg(0);
            title->clear();
            title->printf("%s", this->windowName);
            menuBar->appendWindow(title);
        }

        Button *exitButton = new Button(x + width - 20, y + 5, 31, 31,
            WindowFlags::WNONE, ButtonFlags::BHOVER);
        menuBar->appendWindow(exitButton);
        exitButton->loadImage("/tmp/exitButtonUnhover.ppm");
        exitButton->loadHoverImage("/tmp/exitButtonHover.ppm");

        Button *minimizeButton = new Button(x + width - 40, y + 5, 31, 31,
            WindowFlags::WNONE, ButtonFlags::BHOVER);
        menuBar->appendWindow(minimizeButton);
        minimizeButton->loadImage("/tmp/minimizeButtonUnhover.ppm");
        minimizeButton->loadHoverImage("/tmp/minimizeButtonHover.ppm");

        appendWindow(menuBar);

        Border *border =
            new Border(x, y + TITLE_HEIGHT, 1, height - TITLE_HEIGHT - 1);
        appendWindow(border);

        border = new Border(x + width - 1, y + TITLE_HEIGHT, 1,
                            height - TITLE_HEIGHT - 1);
        appendWindow(border);

        border = new Border(x, y + height - 1, width, 1);
        appendWindow(border);

        border = new Border(x, y + TITLE_HEIGHT, width, 1);
        appendWindow(border);
    }
}

Window::~Window() {}

Window *Window::appendWindow(Window *window) {
    if (numWindows == WINDOW_MAX || !window) {
        return nullptr;
    }

    int insertIndex = 0;
    for (int i = numWindows - 1; i >= 0; i--) {
        if (window->priority >= windows[i]->priority) {
            insertIndex = i + 1;
            break;
        }
    }

    for (int i = numWindows - 1; i >= insertIndex; i--) {
        windows[i + 1] = windows[i];
        windows[i + 1]->setWindowID(i + 1);
    }
    windows[insertIndex] = window;
    
    window->setWindowID(insertIndex);
    window->parent = this;
    numWindows++;
    return window;
}

Window *Window::appendWindow(const char *windowName, int xPos, int yPos,
                             int width, int height, WindowFlags flags) {
    Window *window = new Window(windowName, xPos, yPos, width, height, flags);
    if (!window) {
        return nullptr;
    }

    if (!appendWindow(window)) {
        delete window;
        return nullptr;
    }

    return window;
}

Window *Window::removeWindow(int deleteIndex) {
    if (deleteIndex < 0 || deleteIndex >= numWindows) {
        return nullptr;
    }

    Window *window = windows[deleteIndex];

    for (int i = deleteIndex; i < numWindows - 1; i++) {
        windows[i] = windows[i + 1];
        windows[i]->setWindowID(i);
    }

    // windows[--numWindows] = nullptr;
    numWindows--;
    window->parent = nullptr;
    return window;
}

Window *Window::removeWindow(Window *window) {
    if (!window) {
        return nullptr;
    }

    return removeWindow(window->getWindowID());
}

void Window::applyBoundClipping() {
    if (!parent) {
        if (context->getNumDirty()) {
            Rect *dirtyRegions = context->getDirtyRegions();

            for (int i = 0; i < context->getNumDirty(); i++) {
                context->addClippedRect(&dirtyRegions[i]);
            }

            context->intersectClippedRect(winRect);
        } else {
            context->addClippedRect(winRect);
        }

        return;
    }

    // Reduce drawing to parent window
    parent->applyBoundClipping();

    // Reduce visibility to main drawing area
    context->intersectClippedRect(winRect);

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
        if (intersects(aboveWin->winRect)) {
            context->reshapeRegion(aboveWin->winRect);
        }
    }
}

void Window::applyDirtyDrag() {
    if (!selectedWindow) {
        return;
    }

    context->resetClippedList();

    if (!(xOld == selectedWindow->getX() && yOld == selectedWindow->getY())) {
        int tempX = selectedWindow->getX();
        int tempY = selectedWindow->getY();

        // Original window position
        selectedWindow->setPosition(xOld, yOld);
        context->addClippedRect(selectedWindow->winRect);

        // New window position
        selectedWindow->setPosition(tempX, tempY);
        context->addClippedRect(selectedWindow->winRect);

        xOld = selectedWindow->getX();
        yOld = selectedWindow->getY();

        context->moveClippedToDirty();
    }
}

void Window::drawWindow() {
    applyBoundClipping();

    // Remove child window clipped rectangles
    for (int i = 0; i < numWindows; i++) {
        context->reshapeRegion(windows[i]->winRect);
    }

    // Draw self
    drawObject();

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

void Window::drawObject() {
    context->drawRect(getX(), getY(), getWidth(), getHeight(), color);
}

bool Window::onEvent(Device::TellurEvent *event, int mouseX, int mouseY) {
    Device::MouseData *mouseData = (Device::MouseData *)event->data; // temp fix
    bool isNewMousePressed = mouseData->flags & 0x1;

    for (int i = numWindows - 1; i >= 0; i--) {
        Window *child = windows[i];

        if (!child->isMouseInBounds(mouseX, mouseY)) {
            continue;
        }

        // Pass event to child window
        return child->onEvent(event, mouseX, mouseY);
    }

    // Window raise event (non-terminal event)
    bool isNextClick = isNewMousePressed && !isLastMousePressed();
    if (isNextClick) {
        // Unselect old window
        if (selectedWindow && (selectedWindow != this)) {
            selectedWindow->onWindowUnselect();
        }       
        
        Window *windowRaise = type  == WindowMenuBar ? parent : this;
        // Window *windowRaise = this;
        windowRaise->onWindowSelect();
        selectedWindow = windowRaise;

        xOld = selectedWindow->getX();
        yOld = selectedWindow->getY();
        windowRaise->onWindowRaise();
    }

    // Terminal events

    // Event on menu bar, drag its window
    if (type == GUI::WindowMenuBar && (parent == selectedWindow) &&
        isNewMousePressed) {
        return parent->onWindowDrag(mouseData);
    }

    // Window released from dragging
    if (this == selectedWindow && isLastMousePressed() && !isNewMousePressed) {
        // Window needs immediate refresh
        if (this == selectedWindow) {
            selectedWindow->applyDirtyDrag();
        }

        return onWindowRelease();
    }

    // Window click event
    if (isNewMousePressed && !isLastMousePressed()) {
        return onWindowClick();
    }

    // Window hover event
    if (!isNewMousePressed && !isLastMousePressed()) {
        // Check if hovering over new component
        if (hoverWindow && hoverWindow != this) {
            hoverWindow->onWindowUnhover();
        }

        hoverWindow = this;

        return onWindowHover();
    }

    return true;
}

bool Window::onWindowRaise() {
    Window *refreshWindow = nullptr;
    moveToTop(&refreshWindow, true);
    if (refreshWindow) { // Window needs immediate refresh
        context->addClippedRect(refreshWindow->winRect);
        context->moveClippedToDirty();
    }

    return true;
}

bool Window::onWindowDrag(Device::MouseData *data) {
    setChildPositions(data);

    return true;
}

bool Window::onWindowRelease() {
    return true;
}

bool Window::onWindowClick() {
    return true;
}

bool Window::onWindowSelect() {
    return true;
}

bool Window::onWindowUnselect() {
    return true;
}

bool Window::onWindowHover() {
    return true;
}

bool Window::onWindowUnhover() {
    return true;
}

bool Window::intersects(Rect *rect) { return winRect->intersects(rect); }

int Window::getWindowID() { return this->windowID; }

int Window::getX() { return winRect->getX(); }

int Window::getY() { return winRect->getY(); }

int Window::getWidth() { return winRect->getWidth(); }

int Window::getHeight() { return winRect->getHeight(); }

int Window::getColor() { return color; }

void Window::setWindowID(int windowID) { this->windowID = windowID; }

void Window::setX(int x) { winRect->setX(x); }

void Window::setY(int y) { winRect->setY(y); }

void Window::setPosition(int xNew, int yNew) {
    setX(xNew);
    setY(yNew);
}

void Window::setWidth(int width) { winRect->setWidth(width); }

void Window::setHeight(int height) { winRect->setHeight(height); }

void Window::setColor(uint32_t color) { this->color = color; }

void Window::setPriority(int priority) {
    if (priority < WIN_PRIORITY_MIN || priority > WIN_PRIORITY_MAX) {
        return;
    }

    this->priority = priority;
}

bool Window::isLastMousePressed() { return lastMouseState & 0x1; }

bool Window::hasDecoration() { return flags & WindowFlags::WDECORATION; }

bool Window::hasMovable() { return flags & WindowFlags::WMOVABLE; }

bool Window::isMouseInBounds(int mouseX, int mouseY) {
    return ((mouseX >= getX()) && (mouseX <= (getX() + getWidth())) &&
            (mouseY >= getY()) && (mouseY <= (getY() + getHeight())));
}

void Window::moveToTop(Window **refresh, bool recurse) {
    Window *topWin = parent;
    if (!topWin) { // Root window, terminate
        return;
    }

    int oldWinID = windowID;
    if (hasDecoration()) {
        topWin->removeWindow(windowID);
        topWin->appendWindow(this);

        // Window was moved in stack list
        if (oldWinID != windowID) {
            *refresh = this;
        }
    }
    
    if (recurse) {
        parent->moveToTop(refresh, recurse);
    }
}

void Window::moveThisToDirty() {
    context->addClippedRect(winRect);
    context->moveClippedToDirty();
}

void Window::setChildPositions(Device::MouseData *data) {
    for (int i = 0; i < numWindows; i++) {
        windows[i]->setChildPositions(data);
    }

    setX(getX() + data->delta_x);
    setY(getY() - data->delta_y);
}

}  // namespace GUI
