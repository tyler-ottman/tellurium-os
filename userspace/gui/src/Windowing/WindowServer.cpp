#include "Windowing/WindowServer.hpp"
#include "ulibc/mem.hpp"
#include "ulibc/string.h"

namespace GUI {

WindowServer *WindowServer::instance = nullptr;

WindowServer *WindowServer::getInstance() {
    if (instance) {
        return instance;
    }

    instance = new WindowServer();
    
    return instance;
}

Window *WindowServer::createWindow(const char *w_name, int x_pos, int y_pos,
                                   int width, int height) {
    Window *window = new Window(w_name, x_pos, y_pos, width, height);
    if (!window) {
        return window;
    }

    if (!this->appendWindow(window)) {
        delete window;
        return nullptr;
    }

    return window;
}

Window *WindowServer::appendWindow(Window *window) {
    if (this->numWindows == WINDOW_MAX) {
        return nullptr;
    }

    int windowID = this->numWindows++;
    this->windows[windowID] = window;
    window->setWindowID(windowID);

    return window;
}

Window *WindowServer::removeWindow(int windowID) {
    if (windowID < 0 || windowID >= this->numWindows) {
        return nullptr;
    }

    Window *window = this->windows[windowID];

    for (int i = windowID; i < this->numWindows - 1; i++) {
        this->windows[i] = this->windows[i + 1];
        this->windows[i]->setWindowID(i);
    }
    
    this->windows[this->numWindows - 1] = nullptr;
    this->numWindows--;

    return window;
}

void WindowServer::fillBackground() {
    if (!this->isImgLoaded) {
        FbMeta *meta = context->getFbContext();
        context->drawRect(0, 0, meta->fb_width, meta->fb_height,
                          DEFAULT_COLOR);
        return;
    }

    // Todo: load image
}

void WindowServer::refreshScreen() {
    // this->fillBackground();

    // for (Window *window : this->windows) {
    //     if (window) {
    //         window->windowPaint(window->getColor());
    //     }
    // }

    for (int i = 0; i < numWindows; i++) {
        Window *win = windows[i];
        Rect rect(win->getYPos(), win->getYPos() + win->getHeight() - 1,
                  win->getXPos(), win->getXPos() + win->getWidth() - 1);

        addClippedRect(&rect);
    }

    for (int i = 0; i < numRegions; i++) {
        Rect *rect = &clippedRects[i];

        context->drawRect(rect->getLeft(), rect->getTop(),
                          rect->getRight() - rect->getLeft() + 1,
                          rect->getBottom() - rect->getTop() + 1,
                          0xFF00FF00);
    }

    this->context->drawRect(this->mouseXPos, this->mouseYPos, 10, 10,
                            0xffffffff);
}

void WindowServer::mouseHandle(Device::MouseData *data) {
    this->updateMousePos(data);

    bool newMouseState = data->flags & 0x1;
    if (newMouseState) {
        if (!this->oldLeftState) {
            for (int i = this->numWindows - 1; i >= 0; i--) {
                Window *window = this->windows[i];

                if (this->mouseInBounds(window)) {
                    this->removeWindow(window->getWindowID());
                    this->appendWindow(window);

                    this->dragX = data->delta_x + window->getXPos();
                    this->dragY = data->delta_y + window->getYPos();
                    this->selectedWindow = window;

                    break;
                }
            }
        }
    } else {
        this->selectedWindow = nullptr;
    }

    if (this->selectedWindow) {
        selectedWindow->setXPos(selectedWindow->getXPos() + data->delta_x);
        selectedWindow->setYPos(selectedWindow->getYPos() - data->delta_y);
    }

    this->oldLeftState = newMouseState;

    if (this->nEvents++ == 15) {
        this->refreshScreen();
        this->nEvents = 0;
    }
}

WindowServer::WindowServer()
    : maxWindows(WINDOW_MAX),
      isImgLoaded(false),
      oldLeftState(false),
      selectedWindow(nullptr),
      numRegions(0),
      nEvents(0),
      numWindows(0) {

    for (int i = 0; i < this->maxWindows; i++) {
        this->windows[i] = nullptr;
    }

    this->context = GUI::FbContext::getInstance();

    FbMeta *meta = this->context->getFbContext();
    this->mouseXPos = meta->fb_width / 2;
    this->mouseYPos = meta->fb_height / 2;

    clippedRects = new Rect[CLIPPED_MAX];
}

WindowServer::~WindowServer() {

}

void WindowServer::addClippedRect(Rect *rect) {
    for (int i = 0; i < numRegions; i++) {
        Rect *clipped = &clippedRects[i];

        if (!clipped->isIntersect(rect)) {
            continue;
        }

        // Update and split selected clipped region
        removeClippedRect(i);

        Rect splitRects[4];
        int count = clipped->split(splitRects, rect);

        for (int j = 0; j < count; j++) {
            appendClippedRect(&splitRects[j]);
        }

        i = 0;
    }

    appendClippedRect(rect);
}

void WindowServer::appendClippedRect(Rect *rect) {
    if (numRegions >= CLIPPED_MAX) {
        return;
    }

    for (int i = 0; i < CLIPPED_MAX; i++) {
        if (clippedRects[i].isFree()) {
            clippedRects[i] = *rect;
            break;
        }
    }
}

void WindowServer::removeClippedRect(int index) {
    if (index < 0 || index >= numRegions) {
        return;
    }

    clippedRects[--numRegions].reset();
}

void WindowServer::resetClipList() {
    for (int i = 0; i < CLIPPED_MAX; i++) {
        clippedRects[i].reset();
    }
}

void WindowServer::updateMousePos(Device::MouseData *data) {
    int xNewPos = this->mouseXPos + data->delta_x;
    int yNewPos = this->mouseYPos - data->delta_y;

    FbMeta *meta = this->context->getFbContext();
    if (xNewPos >= 0 && xNewPos < (int)meta->fb_width) {
        this->mouseXPos = xNewPos;
    }

    if (yNewPos >= 0 && yNewPos < (int)meta->fb_height) {
        this->mouseYPos = yNewPos;
    }
}

bool WindowServer::mouseInBounds(Window *window) {
    return ((this->mouseXPos >= window->getXPos()) &&
            (this->mouseXPos <= (window->getXPos() + window->getWidth())) &&
            (this->mouseYPos >= window->getYPos()) &&
            (this->mouseYPos <= (window->getYPos() + window->getHeight())));
}

}