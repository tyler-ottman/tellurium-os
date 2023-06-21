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

    if (!appendWindow(window)) {
        delete window;
        return nullptr;
    }

    return window;
}

Window *WindowServer::appendWindow(Window *window) {
    if (numWindows == WINDOW_MAX) {
        return nullptr;
    }

    int windowID = numWindows++;
    windows[windowID] = window;
    window->setWindowID(windowID);

    return window;
}

Window *WindowServer::removeWindow(int windowID) {
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

void WindowServer::fillBackground() {
    if (!isImgLoaded) {
        FbMeta *meta = context->getFbContext();
        context->drawRect(0, 0, meta->fb_width, meta->fb_height, DEFAULT_COLOR);
        return;
    }

    // Todo: load image
}

void WindowServer::refreshScreen() {
    context->resetClippedList();

    Rect desktop(0, context->getFbContext()->fb_height - 1, 0,
                 context->getFbContext()->fb_width - 1);
    context->addClippedRect(&desktop);

    // Calculate clipping for background
    for (int i = 0; i < numWindows; i++) {
        context->reshapeRegion(windows[i]);
    }

    // Draw background
    context->drawRect(0, 0, context->getFbContext()->fb_width,
                      context->getFbContext()->fb_height, 0);

    // Calculate clipping for each window
    for (int i = 0; i < numWindows; i++) {
        context->resetClippedList();

        Window *win = windows[i];
        context->addClippedRect(win);
        
        for (int j = win->getWindowID() + 1; j < numWindows; j++) {
            Window *aboveWin = windows[j];
            if (win->intersects(aboveWin)) {
                context->reshapeRegion(aboveWin);
            }
        }

        if (context->getRegions() != 0) {
            win->windowPaint();
        }
    }

    // for (int i = 0; i < numWindows; i++) {
    //     context->addClippedRect(windows[i]);
    // }

    // context->drawClippedRegions();

    // Draw mouse
    context->resetClippedList();
    context->drawRect(mouseXPos, mouseYPos, 10, 10, 0xffffffff);
}

void WindowServer::mouseHandle(Device::MouseData *data) {
    updateMousePos(data);

    bool newMouseState = data->flags & 0x1;
    if (newMouseState) {
        if (!oldLeftState) {
            for (int i = numWindows - 1; i >= 0; i--) {
                Window *window = windows[i];

                if (mouseInBounds(window)) {
                    removeWindow(window->getWindowID());
                    appendWindow(window);

                    selectedWindow = window;

                    break;
                }
            }
        }
    } else {
        selectedWindow = nullptr;
    }

    
    if (selectedWindow) {
        selectedWindow->updatePosition(
            selectedWindow->getXPos() + data->delta_x,
            selectedWindow->getYPos() - data->delta_y);
    }

    oldLeftState = newMouseState;

    if (nEvents++ == 10) {
        refreshScreen();
        nEvents = 0;
    }
}

WindowServer::WindowServer()
    : maxWindows(WINDOW_MAX),
      isImgLoaded(false),
      oldLeftState(false),
      selectedWindow(nullptr),
      nEvents(0),
      numWindows(0) {

    for (int i = 0; i < maxWindows; i++) {
        windows[i] = nullptr;
    }

    context = GUI::FbContext::getInstance();

    FbMeta *meta = context->getFbContext();
    mouseXPos = meta->fb_width / 2;
    mouseYPos = meta->fb_height / 2;
}

WindowServer::~WindowServer() {

}

void WindowServer::updateMousePos(Device::MouseData *data) {
    int xNewPos = mouseXPos + data->delta_x;
    int yNewPos = mouseYPos - data->delta_y;

    FbMeta *meta = context->getFbContext();
    if (xNewPos >= 0 && xNewPos < (int)meta->fb_width) {
        mouseXPos = xNewPos;
    }

    if (yNewPos >= 0 && yNewPos < (int)meta->fb_height) {
        mouseYPos = yNewPos;
    }
}

bool WindowServer::mouseInBounds(Window *window) {
    return ((mouseXPos >= window->getXPos()) &&
            (mouseXPos <= (window->getXPos() + window->getWidth())) &&
            (mouseYPos >= window->getYPos()) &&
            (mouseYPos <= (window->getYPos() + window->getHeight())));
}

}