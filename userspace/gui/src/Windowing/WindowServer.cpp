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

    for (int i = 0; i < this->maxWindows; i++) {
        if (!this->windows[i]) {
            this->windows[i] = window;
            window->setWindowID(i);

            return window;
        }
    }

    // unreachable
    // delete window;

    return nullptr;
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
}

WindowServer::WindowServer() : maxWindows(WINDOW_MAX), isImgLoaded(false) {
    for (int i = 0; i < this->maxWindows; i++) {
        this->windows[i] = nullptr;
    }

    this->context = GUI::FbContext::getInstance();
}

WindowServer::~WindowServer() {

}

}