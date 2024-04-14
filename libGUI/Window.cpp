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

Window::Window(const char *windowName, int x, int y, int width, int height,
               WindowFlags flags, WindowPriority priority)
    : windowName(nullptr),
      windowID(-1),
      flags(flags),
      winRect(nullptr),
      color(0xff333333),
      priority(priority),
      parent(nullptr),
      numWindows(0),
      maxWindows(WINDOW_MAX),
      m_pPrevRect(nullptr),
      m_dirty(false),
      m_pHoverWindow(nullptr),
      m_pSelectedWindow(nullptr),
      context(FbContext::getInstance()) {

    if (windowName) {
        int len = __strlen(windowName);
        this->windowName = new char[len + 1];
        __memcpy(this->windowName, windowName, len);
        this->windowName[len] = '\0';
    }

    // This will always store the most up-to-date position/size of Window
    winRect = new Rect(x, y, width, height);

    // This will store the position/size of Window on last refresh
    m_pPrevRect = new Rect(*winRect);

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
        int nDirtyRegions = context->dirtyRegion->getNumClipped();
        if (nDirtyRegions) {
            Rect *dirtyRegions = context->dirtyRegion->getClippedRegions();

            for (int i = 0; i < nDirtyRegions; i++) {
                context->renderRegion->addClippedRect(&dirtyRegions[i]);
            }

            context->renderRegion->intersectClippedRect(winRect);
        } else {
            context->renderRegion->addClippedRect(winRect);
        }

        return;
    }

    // Reduce drawing to parent window
    parent->applyBoundClipping();

    // Reduce visibility to main drawing area
    context->renderRegion->intersectClippedRect(winRect);

    // Occlude areas of window where siblings overlap on top
    for (int i = getWindowID() + 1; i < parent->numWindows; i++) {
        Window *aboveWin = parent->windows[i];
        if (intersects(aboveWin->winRect)) {
            context->renderRegion->reshapeRegion(aboveWin->winRect);
        }
    }
}

bool rectIntersectsDirty(FbContext *context, Rect *rect) {
    Rect *dirtyRegions = context->dirtyRegion->getClippedRegions();
    int nDirtyRegions = context->dirtyRegion->getNumClipped();
    for (int j = 0; j < nDirtyRegions; j++) {
        if (rect->intersects(&dirtyRegions[j])) {
            return true;
        }
    }

    return false;
}

void Window::drawWindow() {
    applyBoundClipping();

    // Remove child window clipped rectangles
    for (int i = 0; i < numWindows; i++) {
        context->renderRegion->reshapeRegion(windows[i]->winRect);
    }

    // Draw self
    drawObject();

    // Redraw children if they intersect with dirty region
    for (int i = 0; i < numWindows; i++) {
        context->renderRegion->resetClippedList();
        Window *child = windows[i];

        if (rectIntersectsDirty(context, child->winRect)) {
            child->drawWindow();
        }        
    }
}

void Window::drawObject() {
    context->drawRect(*winRect, color);
}

bool Window::onEvent(Device::TellurEvent *event, vec2 *mouse) {
    Device::MouseData *mouseData = (Device::MouseData *)event->data; // temp fix

    // We use this pointer later if the event needs to be passed to a child
    Window *childWindow = getWindowUnderMouse(mouse);

    switch (event->type) {
        
    // Check here if the currently hovered over window changes
    case Device::TellurEventType::MouseMove:
        // Process hover event for this Window
        onWindowHover();
        
        // If hovered over window changes, process un-hover on old hover window
        if (m_pHoverWindow && (childWindow != m_pHoverWindow)) {
            m_pHoverWindow->onSubtreeUnhover();
        }

        // Now update current hovered window and process hover event
        m_pHoverWindow = childWindow;

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
        if (m_pSelectedWindow && (childWindow != m_pSelectedWindow)) {
            m_pSelectedWindow->onSubtreeUnselect();
        }

        // Now update current selected window and process select event
        m_pSelectedWindow = childWindow;

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

bool Window::onWindowRaise() {
    return true;
}

bool Window::onWindowDrag(Device::MouseData *data) {
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

bool Window::onSubtreeUnhover() {
    // Process an un-hover event for the current Window
    onWindowUnhover();

    // Process un-hover event for hovered over window (if any)
    if (m_pHoverWindow) {
        m_pHoverWindow->onSubtreeUnhover();
    }
    
    // Set the current hovered over Window to nullptr
    m_pHoverWindow = nullptr;

    return true;
}

bool Window::onSubtreeUnselect() {
    // Process an un-hover event for the current Window
    onWindowUnselect();

    // Process un-hover event for hovered over window (if any)
    if (m_pSelectedWindow) {
        m_pSelectedWindow->onSubtreeUnselect();
    }
    
    // Set the current hovered over Window to nullptr
    m_pSelectedWindow = nullptr;

    return true;
}

bool Window::intersects(Rect *rect) { return winRect->intersects(rect); }

Window *Window::getWindowUnderMouse(vec2 *mouse) {
    for (int i = numWindows - 1; i >= 0; i--) {
        Window *child = windows[i];

        // Return the first instance of the mouse being located within boundary
        if (child->isMouseInBounds(mouse)) {
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

void Window::updatePrevRect() { *m_pPrevRect = *winRect; }

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

int Window::getWindowID() { return this->windowID; }

int Window::getX() { return winRect->getX(); }

int Window::getY() { return winRect->getY(); }

int Window::getWidth() { return winRect->getWidth(); }

int Window::getHeight() { return winRect->getHeight(); }

int Window::getColor() { return color; }

int Window::getNumChildren() { return numWindows; }

Window *Window::getChild(int windowID) { return windows[windowID]; }

Rect *Window::getWinRect() { return winRect; }

Rect *Window::getPrevRect() { return m_pPrevRect; }

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

void Window::setDirty(bool dirty) { this->m_dirty = dirty; }

void Window::setPriority(WindowPriority priority) {
    if (priority >= WindowPriority::WPRIO0 &&
        priority <= WindowPriority::WPRIO9) {
        this->priority = priority;
    }
}

bool Window::hasDecoration() { return flags & WindowFlags::WDECORATION; }

bool Window::hasMovable() { return flags & WindowFlags::WMOVABLE; }

bool Window::hasUnbounded() { return flags & WindowFlags::WUNBOUNDED; }

bool Window::isDirty() { return m_dirty; }

bool Window::isMouseInBounds(vec2 *mouse) {
    return ((mouse->x >= getX()) && (mouse->x <= (getX() + getWidth())) &&
            (mouse->y >= getY()) && (mouse->y <= (getY() + getHeight())));
}

}  // namespace GUI
