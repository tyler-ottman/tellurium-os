#include "ulibc/mem.hpp"
#include "ulibc/string.h"
#include "Windowing/FbContext.hpp"
#include "Windowing/Window.hpp"

namespace GUI {

Window::Window(const char *w_name, int xPos, int yPos, int width, int height) {
    this->xPos = xPos;
    this->yPos = yPos;
    this->width = width;
    this->height = height;

    this->windowName = new char[__strlen(w_name)];
}

Window::~Window() {

}

void Window::setWindowID(int windowID) {
    this->windowID = windowID;
}

int  Window::getWindowID() {
    return this->windowID;
}

void Window::windowPaint(uint32_t color) {
    GUI::FbContext *fbContext = GUI::FbContext::getInstance();
    if (fbContext == nullptr) {
        return;
    }

    fbContext->drawRect(this->xPos, this->yPos, this->width, this->height,
                        color);
}

}