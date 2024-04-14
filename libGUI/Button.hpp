#ifndef BUTTON_H
#define BUTTON_H

#include "Image.hpp"
#include "Window.hpp"

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
            ButtonFlags bFlags = ButtonFlags::BNONE,
            WindowPriority priority = WindowPriority::WPRIO2);
    ~Button();

    bool onWindowHover(void) override;
    bool onWindowUnhover(void) override;
    void loadImage(const char *path);
    void loadHoverImage(const char *path);

private:
    inline bool isFlagBorder(void) { return buttonFlags & ButtonFlags::BBORDER; }
    inline bool isFlagToggle(void) { return buttonFlags & ButtonFlags::BTOGGLE; }
    inline bool isFlagHover(void) { return buttonFlags & ButtonFlags::BHOVER; }

    Image *imgDefault;
    Image *imgHover;

    ButtonFlags buttonFlags;
};

}

#endif // BUTTON_H