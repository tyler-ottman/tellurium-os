#pragma once

#include "libGUI/Image.hpp"
#include "libGUI/Window.hpp"

/// @brief Common button flags
enum ButtonFlags {
    BNONE = 0x0,
    BBORDER = 0x1,
    BTOGGLE = 0x2,
    BHOVER = 0x4
};

namespace GUI {

class Button: public Window {
public:
    Button(int x, int y, int width, int height,
            WindowFlags flags = WindowFlags::WNONE,
            ButtonFlags bFlags = ButtonFlags::BNONE);
    ~Button();

    bool onMouseClick(void);
    bool onButtonHover(void);
    bool onButtonUnhover(void);
    void drawObject(void);
    void loadImage(const char *path);
    void loadHoverImage(const char *path);
private:
    inline bool isFlagBorder(void) { return buttonFlags & ButtonFlags::BBORDER; }
    inline bool isFlagToggle(void) { return buttonFlags & ButtonFlags::BTOGGLE; }
    inline bool isFlagHover(void) { return buttonFlags & ButtonFlags::BHOVER; }

    bool colorToggle;
    bool onHover;

    bool isImg;
    Image *imgDefault;
    Image *imgHover;

    ButtonFlags buttonFlags;
};

}