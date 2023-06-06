#include "Window.hpp"

namespace GUI {

Window::Window(const char *w_name, int xPos, int yPos, int width, int height) {
    this->xPos = xPos;
    this->yPos = yPos;
    this->width = width;
    this->height = height;
}

}