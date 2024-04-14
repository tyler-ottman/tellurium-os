#ifndef BORDER_H
#define BORDER_H

#include "Window.hpp"

namespace GUI {

class Border : public Window {
public:
    Border(int x, int y, int width, int height,
        WindowFlags flags = WindowFlags::WNONE,
        WindowPriority priority = WindowPriority::WPRIO2);
    ~Border();
};

};

#endif // BORDER_H