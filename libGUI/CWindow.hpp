#ifndef CWINDOW_H
#define CWINDOW_H

#include "libTellur/DevPoll.hpp"
#include "libGUI/FbContext.hpp"
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
    CWindow(FbContext *context);
    ~CWindow();

    /// @brief Process event 
    /// @param event The event to be processed
    void processEvent(Device::TellurEvent *event);

    void updateMousePos(Device::MouseData *data);

    Window *mouse;
    int nEvents;
    Device::DeviceManager *devManager;

    Compositor *compositor;
    FbContext *context;

    static CWindow *instance;
};

}

#endif // CWINDOW_H