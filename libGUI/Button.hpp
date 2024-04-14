#ifndef BUTTON_H
#define BUTTON_H

#include "libTellur/ImageReader.hpp"
#include "Window.hpp"

/// @brief Common button flags
enum ButtonFlags {
    BNONE = 0x0,
    BBORDER = 0x1,
    BHOVER = 0x2
};

namespace GUI {

class Button: public Window {
public:
    Button(int x, int y, int width, int height,
            WindowFlags flags = WindowFlags::WNONE,
            ButtonFlags bFlags = ButtonFlags::BNONE,
            WindowPriority priority = WindowPriority::WPRIO2);
    Button(int x, int y, ImageReader *img,
           WindowFlags flags = WindowFlags::WNONE,
           ButtonFlags bFlags = ButtonFlags::BNONE,
           WindowPriority priority = WindowPriority::WPRIO2);
    ~Button();

    bool onWindowHover(void) override;
    bool onWindowUnhover(void) override;
    void loadHoverImage(ImageReader *img);

private:
    inline bool isFlagBorder(void) { return buttonFlags & ButtonFlags::BBORDER; }
    inline bool isFlagHover(void) { return buttonFlags & ButtonFlags::BHOVER; }

    // Image *imgDefault;
    // Image *imgHover;
    uint8_t *imgNoHover;
    uint8_t *imgHover;

    ButtonFlags buttonFlags;
};

}

#endif // BUTTON_H