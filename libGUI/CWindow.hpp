#ifndef CWINDOW_H
#define CWINDOW_H

#include "libGUI/Window.hpp"

namespace GUI {

class CWindow: public Window {
public:
    static CWindow *getInstance();

    void refresh(void);
    void drawMouse(void);
    void processMouseEvent(Device::MouseData *data);
    void pollEvents(void);
    void applyDirtyMouse(void);
    
    int getMouseX(void) { return mouseX; }
    int getMouseY(void) { return mouseY; }
private:
    CWindow(void);
    ~CWindow();

    void updateMousePos(Device::MouseData *data);
    bool mouseInBounds(Window *window);

    int mouseX;
    int mouseY;
    int oldMouseX;
    int oldMouseY;

    bool forceRefresh;
    // Event counter to facilitate refresh rate
    int nEvents;

    static CWindow *instance;
};

}

#endif // CWINDOW_H