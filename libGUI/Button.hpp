#ifndef BUTTON_H
#define BUTTON_H

#include "libTellur/ImageReader.hpp"
#include "Window.hpp"

/// @brief Common button flags
typedef int ButtonFlags;
enum ButtonFlags_ {
    ButtonFlags_None                        = 0x0,
    ButtonFlags_Border                      = 0x1,
    ButtonFlags_Hover                       = 0x2
};

namespace GUI {

class Button: public Window {
public:
    Button(int x, int y, int width, int height,
            WindowFlags flags = WindowFlags_None,
            ButtonFlags bFlags = ButtonFlags_None,
            WindowPriority priority = WindowPriority_2);
    Button(int x, int y, const char *path,
           WindowFlags flags = WindowFlags_None,
           ButtonFlags bFlags = ButtonFlags_None,
           WindowPriority priority = WindowPriority_2);
    ~Button();

    bool onWindowHover(void) override;
    bool onWindowUnhover(void) override;
    void loadHoverImage(const char *path);

private:
    bool getFlag(ButtonFlags field) { return buttonFlags & field; }

    uint8_t *imgNoHover;
    uint8_t *imgHover;

    ButtonFlags buttonFlags;
};

}

#endif // BUTTON_H