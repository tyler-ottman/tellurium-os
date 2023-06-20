#include "ulibc/mem.hpp"
#include "ulibc/string.h"
#include "Windowing/FbContext.hpp"
#include "Windowing/Window.hpp"

uint8_t pseudo_rand_8() {
    static uint16_t seed = 0;
    return (uint8_t)(seed = (12657 * seed + 12345) % 256);
}

namespace GUI {

void Window::updateRect() {
    Rect::setTop(y);
    Rect::setBottom(y + height - 1);
    Rect::setLeft(x);
    Rect::setRight(x + width - 1);
}

Window::Window(const char *w_name, int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height) {
    this->color = 0xff000000 | pseudo_rand_8() << 16 | pseudo_rand_8() << 8 |
                  pseudo_rand_8();

    this->windowName = new char[__strlen(w_name)];

    updateRect();
}

Window::~Window() {

}

bool Window::intersects(Rect *rect) {
    return Rect::intersects(rect);
}

void Window::updatePosition(int xNew, int yNew) {
    x = xNew;
    y = yNew;

    updateRect();
}

void Window::setWindowID(int windowID) {
    this->windowID = windowID;
}

int  Window::getWindowID() {
    return this->windowID;
}

int Window::getXPos() {
    return this->x;
}

int Window::getYPos() {
    return this->y;
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

void Window::setXPos(int x) {
    this->x = x;
}

void Window::setYPos(int y) {
    this->y = y;
}

void Window::windowPaint() {
    GUI::FbContext *fbContext = GUI::FbContext::getInstance();
    if (fbContext == nullptr) {
        return;
    }

    fbContext->drawRect(this->x, this->y, this->width, this->height,
                        this->color);
}

}