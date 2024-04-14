#ifndef CWINDOW_H
#define CWINDOW_H

#include "libTellur/DevPoll.hpp"
#include "libGUI/Window.hpp"

namespace GUI {

/// @brief Client Window/Application implementation
class CWindow: public Window {
public:
    /// @brief Get an instance of the CWindow
    /// @return The instance
    static CWindow *getInstance();

    /// @brief Poll for received mouse/keyboard events
    void pollEvents(void);

    /// @brief Force refresh the CWindow
    void forceRefresh(void);

private:
    CWindow(void);
    ~CWindow();

    /// @brief Process event 
    /// @param event The event to be processed
    void processEvent(Device::TellurEvent *event);
        
    int getMouseX(void) { return mouse->x; }
    int getMouseY(void) { return mouse->y; }

    void updateMousePos(Device::MouseData *data);
    bool mouseInBounds(Window *window);

    vec2 *mouse;
    vec2 *oldMouse;

    int nEvents;

    Device::DeviceManager *devManager;

    static CWindow *instance;
};

}

#endif // CWINDOW_H