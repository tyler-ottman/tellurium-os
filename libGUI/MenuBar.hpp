#ifndef MENUBAR_H
#define MENUBAR_H

#include "Window.hpp"

namespace GUI {

class MenuBar : public Window {
public:
    MenuBar(int x, int y, int width, int height,
            WindowFlags flags = WindowFlags_None,
            WindowPriority priority = WindowPriority_5);
    ~MenuBar();

    bool onWindowDrag(Device::MouseData *data) override;
    uint32_t getBarColor(void);
    
private:
    void setBarColor(uint32_t color);

    uint32_t barColor;
};

};

#endif // MENUBAR_H