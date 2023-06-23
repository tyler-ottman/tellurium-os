#pragma once

#include "libGUI/Window.hpp"

namespace GUI {

class Button: public Window {
public:
    Button(int x, int y, int width, int height);
    ~Button();

    void onMouseClick(void);
    void drawWindow(void);

private:
    bool colorToggle;
};

}