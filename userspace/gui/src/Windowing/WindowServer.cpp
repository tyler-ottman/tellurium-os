#include "Windowing/WindowServer.hpp"
#include "ulibc/mem.hpp"

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
    window->setWindowID(-1);

    for (int i = windowID; i < this->numWindows - 1; i++) {
        this->windows[i] = this->windows[i + 1];
    }

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
    this->fillBackground();

    int tmp_counter = 1;
    for (Window *window : this->windows) {
        if (window) {
            window->windowPaint(300 * tmp_counter++);
        }
    }

    this->context->drawRect(this->xPos, this->yPos, 10, 10, 0xffffffff);
}

void WindowServer::mouseHandle(Device::MouseData *data) {
    this->updateMousePos(data);

    bool newMouseState = data->flags & 0x1;
    if (newMouseState && !this->oldLeftState) {
        for (int i = this->numWindows - 1; i >= 0; i--) {
            Window *window = this->windows[i];

            if (this->mouseInBounds(window)) {
                this->removeWindow(window->getWindowID());
                this->appendWindow(window);

                break;
            }
        }
    }

    this->oldLeftState = newMouseState;

    this->refreshScreen();
}

WindowServer::WindowServer()
    : maxWindows(WINDOW_MAX),
      isImgLoaded(false),
      oldLeftState(false),
      numWindows(0) {

    for (int i = 0; i < this->maxWindows; i++) {
        this->windows[i] = nullptr;
    }

    this->context = GUI::FbContext::getInstance();

    FbMeta *meta = this->context->getFbContext();
    this->xPos = meta->fb_width / 2;
    this->yPos = meta->fb_height / 2;
}

WindowServer::~WindowServer() {

}

void WindowServer::updateMousePos(Device::MouseData *data) {
    int xNewPos = this->xPos + data->delta_x;
    int yNewPos = this->yPos - data->delta_y;

    FbMeta *meta = this->context->getFbContext();
    if (xNewPos >= 0 && xNewPos < (int)meta->fb_width) {
        this->xPos = xNewPos;
    }

    if (yNewPos >= 0 && yNewPos < (int)meta->fb_height) {
        this->yPos = yNewPos;
    }
}

bool WindowServer::mouseInBounds(Window *window) {
    return ((this->xPos >= window->getXPos()) &&
            (this->xPos <= (window->getXPos() + window->getWidth())) &&
            (this->yPos >= window->getYPos()) &&
            (this->yPos <= (window->getYPos() + window->getHeight())));
}

}