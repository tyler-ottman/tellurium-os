#pragma once

#include "libGUI/Window.hpp"

namespace GUI {

class Desktop: public Window {
public:
    Desktop(void);
    ~Desktop();

    

    void windowPaint(void);
    void onMouseMove(Device::MouseData *data);

private:
    void updateMousePos(Device::MouseData *data);
    bool mouseInBounds(Window *window);

    int mouseX;
    int mouseY;
};

}