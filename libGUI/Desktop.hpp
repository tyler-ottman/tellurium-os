#pragma once

#include "libGUI/Window.hpp"

namespace GUI {

class Desktop: public Window {
public:
    Desktop(void);
    ~Desktop();

    void drawWindow(void);
    void onMouseEvent(Device::MouseData *data);
    void applyDirtyMouse(void);
    
    int getMouseX(void) { return mouseX; }
    int getMouseY(void) { return mouseY; }
private:
    void updateMousePos(Device::MouseData *data);
    bool mouseInBounds(Window *window);

    int mouseX;
    int mouseY;
    int oldMouseX;
    int oldMouseY;

    bool forceRefresh;
};

}