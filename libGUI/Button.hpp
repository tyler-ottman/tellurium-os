#pragma once

#include "libGUI/Window.hpp"

// Button flags
#define BUTTON_BORDER                       0x1
#define BUTTON_TOGGLE                       0x2
#define BUTTON_HOVER                        0x4

namespace GUI {

class Button: public Window {
public:
    Button(int x, int y, int width, int height, int buttonFlags);
    ~Button();

    bool onMouseClick(void);
    bool onButtonHover(void);
    bool onButtonUnhover(void);
    void drawWindow(void);

private:
    inline bool isFlagBorder(void) { return buttonFlags & BUTTON_BORDER; }
    inline bool isFlagToggle(void) { return buttonFlags & BUTTON_TOGGLE; }
    inline bool isFlagHover(void) { return buttonFlags & BUTTON_HOVER; }

    bool colorToggle;
    bool onHover;

    uint8_t buttonFlags;
};

}