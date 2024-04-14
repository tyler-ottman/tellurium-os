#include "Border.hpp"
#include "Button.hpp"
#include "flibc/string.h"
#include "libTellur/mem.hpp"
#include "MenuBar.hpp"
#include "Terminal.hpp"
#include "Window.hpp"

namespace GUI {

Window::Window(const char *windowName, int x, int y, int width, int height,
               WindowFlags flags, WindowPriority priority)
    : windowName(nullptr),
      windowID(-1),
      flags(flags),
      winRect(nullptr),
      winBuff(nullptr),
      color(0xff333333),
      priority(priority),
      parent(nullptr),
      numWindows(0),
      maxWindows(WINDOW_MAX),
      winPrevRect(nullptr),
      dirty(false),
      hoverWindow(nullptr),
      selectedWindow(nullptr) {

    if (windowName) {
        int len = __strlen(windowName);
        this->windowName = new char[len + 1];
        __memcpy(this->windowName, windowName, len);
        this->windowName[len] = '\0';
    }

    // This will always store the most up-to-date position/size of Window
    winRect = new Rect(x, y, width, height);

    // The actual contents of the window
    winBuff = new uint32_t[width * height];
    loadBuff(color);

    // This will store the position/size of Window on last refresh
    winPrevRect = new Rect(*winRect);

    for (int i = 0; i < maxWindows; i++) {
        windows[i] = nullptr;
    }

    // Menu bar can only be attached to movable windows
    if (hasDecoration()) {
        MenuBar *menuBar = new MenuBar(x, y, width, TITLE_HEIGHT);

        if (this->windowName) {
            int titleLen = __strlen(this->windowName) * 8;  // 8 is default font width
            Terminal *title = new Terminal(x + width / 2 - titleLen / 2,
                y + 10, titleLen, 30);
            title->disableCursor();
            title->setBg(0xffbebebe);
            title->setFg(0);
            title->clear();
            title->printf("%s", this->windowName);
            menuBar->appendWindow(title);
        }

        PpmReader exitImg("/tmp/exitButtonUnhover.ppm");
        PpmReader exitHoverImg("/tmp/exitButtonHover.ppm");
        Button *exitButton = new Button(x + width - 20, y + 5, &exitImg,
            WindowFlags::WNONE, ButtonFlags::BHOVER);
        exitButton->loadHoverImage(&exitHoverImg);
        menuBar->appendWindow(exitButton);

        PpmReader minimizeImg("/tmp/minimizeButtonUnhover.ppm");
        PpmReader minimizeHoverImg("/tmp/minimizeButtonHover.ppm");
        Button *minimizeButton = new Button(x + width - 40, y + 5, &minimizeImg,
            WindowFlags::WNONE, ButtonFlags::BHOVER);
        minimizeButton->loadHoverImage(&minimizeHoverImg);
        menuBar->appendWindow(minimizeButton);

        appendWindow(menuBar);
        appendWindow(new Border(x, y + TITLE_HEIGHT, 1, height - TITLE_HEIGHT - 1));
        appendWindow(new Border(x + width - 1, y + TITLE_HEIGHT, 1, height - TITLE_HEIGHT - 1));
        appendWindow(new Border(x, y + height - 1, width, 1));
        appendWindow(new Border(x, y + TITLE_HEIGHT, width, 1));
    }
}

Window::~Window() {}

Window *Window::appendWindow(Window *window) {
    if (numWindows == maxWindows || !window) {
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

bool Window::onEvent(Device::TellurEvent *event, Window *mouse) {
    Device::MouseData *mouseData = (Device::MouseData *)event->data; // temp fix

    // We use this pointer later if the event needs to be passed to a child
    Window *childWindow = getWindowUnderMouse(mouse);

    switch (event->type) {
        
    // Check here if the currently hovered over window changes
    case Device::TellurEventType::MouseMove:
        // Process hover event for this Window
        onWindowHover();
        
        // If hovered over window changes, process un-hover on old hover window
        if (hoverWindow && (childWindow != hoverWindow)) {
            hoverWindow->onSubtreeUnhover();
        }

        // Now update current hovered window and process hover event
        hoverWindow = childWindow;

        break;

    // Process window dragging event
    case Device::TellurEventType::MouseMoveClick:
        onWindowDrag(mouseData);

        break;

    // Check here if currently clicked on window changes
    case Device::TellurEventType::MouseLeftClick:
        // Process select even for this Window
        onWindowSelect();

        // If selected window changes, process un-select on old selected window
        if (selectedWindow && (childWindow != selectedWindow)) {
            selectedWindow->onSubtreeUnselect();
        }

        // Now update current selected window and process select event
        selectedWindow = childWindow;

        // Notify parent to move Window to top of stack if it's decorable
        if (parent && hasDecoration()) {
            parent->moveToTop(this);
        }

        break;

    // TODO: Maybe releasing mouse is an operation on a subtree and not window
    case Device::TellurEventType::MouseLeftRelease:
        onWindowRelease();
        
        break;
    
    // Never reach here
    case Device::TellurEventType::MouseDefault:
        break;

    // Never reach here
    case Device::TellurEventType::KeyboardDefault:
    case Device::TellurEventType::Default:
        break;

    }

    // If mouse was on child window, process event for child
    if (childWindow) {
        childWindow->onEvent(event, mouse);
    }

    return true;
}

bool Window::onWindowRaise() { return true; }

bool Window::onWindowDrag(Device::MouseData *data) { return true; }

bool Window::onWindowRelease() { return true; }

bool Window::onWindowClick() { return true; }

bool Window::onWindowSelect() { return true; }

bool Window::onWindowUnselect() { return true; }

bool Window::onWindowHover() { return true; }

bool Window::onWindowUnhover() { return true; }

bool Window::onSubtreeUnhover() {
    // Process an un-hover event for the current Window
    onWindowUnhover();

    // Process un-hover event for hovered over window (if any)
    if (hoverWindow) {
        hoverWindow->onSubtreeUnhover();
    }
    
    // Set the current hovered over Window to nullptr
    hoverWindow = nullptr;

    return true;
}

bool Window::onSubtreeUnselect() {
    // Process an un-hover event for the current Window
    onWindowUnselect();

    // Process un-hover event for hovered over window (if any)
    if (selectedWindow) {
        selectedWindow->onSubtreeUnselect();
    }
    
    // Set the current hovered over Window to nullptr
    selectedWindow = nullptr;

    return true;
}

bool Window::intersects(Rect *rect) { return winRect->intersects(rect); }

Window *Window::getWindowUnderMouse(Window *mouse) {
    for (int i = numWindows - 1; i >= 0; i--) {
        Window *child = windows[i];

        // Return the first instance of the mouse being located within boundary
        if (child->isCoordInBounds(mouse->getX(), mouse->getY())) {
            return child;
        }
    }

    // The mouse is not hovering over any child
    return nullptr;
}

void Window::updateChildPositions(Device::MouseData *data) {
    for (int i = 0; i < numWindows; i++) {
        windows[i]->updateChildPositions(data);
    }

    setX(getX() + data->delta_x);
    setY(getY() - data->delta_y);
}

void Window::updatePrevRect() { *winPrevRect = *winRect; }

bool Window::moveToTop(Window *child) {
    // Invalid window
    if (child != windows[child->windowID]) {
        return false;
    }

    int oldWindowID = child->windowID;
    removeWindow(child);

    // If the Window actually moved in the stack, mark as dirty
    Window *win = appendWindow(child);
    if (win->getWindowID() != oldWindowID) {
        win->setDirty(true);
    }

    return true;
}

void Window::loadBuff(uint32_t *buff) {
    size_t buffSize = winRect->getWidth() * winRect->getHeight();
    for (size_t i = 0; i < buffSize; i++) {
        winBuff[i] = buff[i];
    }
}

void Window::loadBuff(uint32_t color) {
    size_t buffSize = winRect->getWidth() * winRect->getHeight();
    for (size_t i = 0; i < buffSize; i++) {
        winBuff[i] = color;
    }
    this->color = color;
}

int Window::getWindowID() { return this->windowID; }

int Window::getX() { return winRect->getX(); }

int Window::getY() { return winRect->getY(); }

int Window::getWidth() { return winRect->getWidth(); }

int Window::getHeight() { return winRect->getHeight(); }

int Window::getColor() { return color; }

int Window::getNumChildren() { return numWindows; }

Rect *Window::getWinRect() { return winRect; }

Rect *Window::getPrevRect() { return winPrevRect; }

void Window::setWindowID(int windowID) { this->windowID = windowID; }

void Window::setX(int x) { winRect->setX(x); }

void Window::setY(int y) { winRect->setY(y); }

void Window::setPosition(int xNew, int yNew) {
    setX(xNew);
    setY(yNew);
}

void Window::setWidth(int width) { winRect->setWidth(width); }

void Window::setHeight(int height) { winRect->setHeight(height); }

void Window::setDirty(bool dirty) { this->dirty = dirty; }

void Window::setPriority(WindowPriority priority) {
    if (priority >= WindowPriority::WPRIO0 &&
        priority <= WindowPriority::WPRIO9) {
        this->priority = priority;
    }
}

bool Window::hasDecoration() { return flags & WindowFlags::WDECORATION; }

bool Window::hasMovable() { return flags & WindowFlags::WMOVABLE; }

bool Window::hasUnbounded() { return flags & WindowFlags::WUNBOUNDED; }

bool Window::isDirty() { return dirty; }

bool Window::isCoordInBounds(int x, int y) {
    return ((x >= getX()) && (x <= (getX() + getWidth())) &&
            (y >= getY()) && (y <= (getY() + getHeight())));
}

}  // namespace GUI
