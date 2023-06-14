#include "ulibc/mem.hpp"
#include "ulibc/string.h"
#include "Windowing/FbContext.hpp"
#include "Windowing/Window.hpp"

uint8_t pseudo_rand_8() {
    static uint16_t seed = 0;
    return (uint8_t)(seed = (12657 * seed + 12345) % 256);
}

namespace GUI {

Window::Window(const char *w_name, int xPos, int yPos, int width, int height) {
    this->xPos = xPos;
    this->yPos = yPos;
    this->width = width;
    this->height = height;

    this->color = 0xff000000 |
                  pseudo_rand_8() << 16 |
                  pseudo_rand_8() << 8 |
                  pseudo_rand_8();

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

int Window::getXPos() {
    return this->xPos;
}

int Window::getYPos() {
    return this->yPos;
}

int Window::getWidth() {
    return this->width;
}

int Window::getHeight() {
    return this->height;
}

int Window::getColor() {
    return this->color;
}

void Window::setXPos(int xPos) {
    this->xPos = xPos;
}

void Window::setYPos(int yPos) {
    this->yPos = yPos;
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