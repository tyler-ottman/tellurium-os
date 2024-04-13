#ifndef MENUBAR_H
#define MENUBAR_H

#include "Window.hpp"

namespace GUI {

class MenuBar : public Window {
public:
    MenuBar(int x, int y, int width, int height,
            WindowFlags flags = WindowFlags::WNONE,
            WindowPriority priority = WindowPriority::WPRIO5);
    ~MenuBar();

    bool onWindowClick(void) override;
    bool onWindowSelect(void) override;
    bool onWindowUnselect(void) override;
    bool onWindowDrag(Device::MouseData *data) override;

    void drawObject(void) override;

    uint32_t getBarColor(void);
    
private:
    void setBarColor(uint32_t color);

    uint32_t barColor;
};

};

#endif // MENUBAR_H