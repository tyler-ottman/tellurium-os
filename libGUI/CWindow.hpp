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

    /// @brief Refresh the CWindow
    void refresh(void);

    /// @brief Force refresh the CWindow
    void forceRefresh(void);

private:
    CWindow(void);
    ~CWindow();

    /// @brief Process event 
    /// @param event The event to be processed
    void processEvent(Device::TellurEvent *event);

    void drawMouse(void);
    
    void applyDirtyMouse(void);
    
    int getMouseX(void) { return mouseX; }
    int getMouseY(void) { return mouseY; }

    void updateMousePos(Device::MouseData *data);
    bool mouseInBounds(Window *window);

    int mouseX;
    int mouseY;
    int oldMouseX;
    int oldMouseY;

    int nEvents;

    Device::DeviceManager *devManager;

    static CWindow *instance;
};

}

namespace Device {

}

#endif // CWINDOW_H