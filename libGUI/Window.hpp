#ifndef WINDOW_H
#define WINDOW_H

#include "libGUI/Rect.hpp"
#include "libGUI/Utility.hpp"
#include "libTellur/DevPoll.hpp"
#include <stdbool.h>
#include <stdint.h>

#define WINDOW_MAX                          10
#define TITLE_HEIGHT                        31
#define TITLE_WIDTH                         (width)

#define BORDER_COLOR                        0xff333333
#define BORDER_WIDTH                        3
#define MENUBAR_SELECT                      0xff545454

#define WIN_PRIORITY_MIN                    0
#define WIN_PRIORITY_MAX                    9

namespace GUI {

class FbContext;

/// @brief Common window flags
enum WindowFlags {
    WNONE = 0x0,
    WDECORATION = 0x1,
    WHOVER = 0x2,
    WMOVABLE = 0x4,
    WUNBOUNDED = 0x8
};

/// @brief Common Window priorities
enum WindowPriority {
    WPRIO0 = 0,
    WPRIO1,
    WPRIO2,
    WPRIO3,
    WPRIO4,
    WPRIO5,
    WPRIO6,
    WPRIO7,
    WPRIO8,
    WPRIO9
};

class Window {
    friend class FbContext;

public:
    /// @brief Constructor to create a new window
    /// @param windowName Name of the window
    /// @param x Horizontal pixel coordinate position of window
    /// @param y Vertical pixel coordinate position of window
    /// @param width Width of window in pixels
    /// @param height Height of window in pixels
    /// @param flags Window flags
    Window(const char *windowName, int xPos, int yPos, int width, int height,
           WindowFlags flags = WindowFlags::WNONE,
           WindowPriority priority = WindowPriority::WPRIO2);

    /// @brief Execute destructor of base and derives classes of Window
    virtual ~Window();

    /// @brief Append a child window to current window
    /// @param windowName Name of the window
    /// @param xPos Horizontal pixel coordinate position
    /// @param yPos Vertical pixel coordiante position
    /// @param width Width of window in pixels
    /// @param height Height of window in pixels
    /// @param flags Windows flags
    /// @return Upon success, return a pointer to the new window, nullptr otherwise
    Window *appendWindow(const char *windowName, int xPos, int yPos, int width,
                         int height, WindowFlags flags = WindowFlags::WNONE);

    /// @brief Append a child window to current window
    /// @param window A pointer to the window to be added
    /// @return Upon success, return a pointer to the new window, nullptr otherwise
    Window *appendWindow(Window *window);

    /// @brief Remove the child window given a window ID
    /// @param deleteIndex The window ID (index) of the child to be removed
    /// @return Upon successful removal, return a pointer to the removed window,
    /// nullptr otherwise
    Window *removeWindow(int deleteIndex);

    /// @brief Remove the specified window from the windows list
    /// @param window The window to be removed
    /// @return If window is invalid, return nullptr, the delete window
    Window *removeWindow(Window *window);

    // TODO: Move drawing operations outside of Window definition
    virtual void drawObject(void);

    //// @brief Process event
    /// @param data Incoming mouse data
    /// @param mouse Position of mouse
    /// @return Event process status
    virtual bool onEvent(Device::TellurEvent *data, vec2 *mouse);
    
    /// @brief Process window raise event
    /// @return Event process status
    virtual bool onWindowRaise(void);

    /// @brief Process window drag event
    /// @param data Incoming mouse data
    /// @return Event process status
    virtual bool onWindowDrag(Device::MouseData *data);

    /// @brief Process window release event
    /// @return Event process status
    virtual bool onWindowRelease(void);

    /// @brief Process window click event
    /// @return Event process status
    virtual bool onWindowClick(void);

    /// @brief Process window select event
    /// @return Event process status
    virtual bool onWindowSelect(void);

    /// @brief Process window unselect event
    /// @return Event process status
    virtual bool onWindowUnselect(void);

    /// @brief Process window hover event
    /// @return Event process status
    virtual bool onWindowHover(void);

    /// @brief Process window unhover event
    /// @return Event process status
    virtual bool onWindowUnhover(void);

    /// @brief Unhover the subtree
    /// @return Operation status
    bool onSubtreeUnhover(void);

    /// @brief Unselect the subtree
    /// @return Operation status
    bool onSubtreeUnselect(void);

    /// @brief Determine if rectangle intersects with Window
    /// @param rect The rectangle to test
    bool intersects(Rect *rect);

    /// @brief Determine if rectangle intersects with Window
    /// @param rect The window to test
    bool intersects(Window *rect) { return intersects(rect->winRect); }

    /// @brief Get the child Window underneath the mouse
    /// @param mouse the position
    /// @return The child window underneath the mouse, or nullptr if the mouse
    ///         is not hovering over any child window
    Window *getWindowUnderMouse(vec2 *mouse);

    /// @brief Recursively update position of window and descendant
    /// @param data The delta (x, y) position
    void updateChildPositions(Device::MouseData *data);

    /// @brief Move current Window dimensions to previous Window dimension
    void updatePrevRect(void);

    /// @brief Move window to top of window stack
    /// @param child Recursively the child to raise on the window stack
    /// @return The highest level window that was raised
    bool moveToTop(Window *child);

    /// @brief Get ID of window
    int getWindowID(void);

    /// @brief Get window's x position
    int getX(void);

    /// @brief Get window's y position
    int getY(void);

    /// @brief Get window's width
    int getWidth(void);

    /// @brief Get window's height
    int getHeight(void);

    /// @brief Get window's color
    int getColor(void);

    /// @brief Get the number of children held by window
    int getNumChildren(void);

    Window *getChild(int windowID);

    /// @brief Get window's Rect boundary
    Rect *getWinRect(void);

    /// @brief Get window's previous Rect boundary
    Rect *getPrevRect(void);

    /// @brief Set window's ID
    void setWindowID(int windowID);

    /// @brief Set window's x position
    void setX(int x);

    /// @brief Set window's y position
    void setY(int y);

    /// @brief Set window's (x, y) position
    void setPosition(int xNew, int yNew);

    /// @brief Set window's width
    void setWidth(int width);

    /// @brief Set window's height
    void setHeight(int height);

    /// @brief Set window's color
    void setColor(uint32_t color);

    /// @brief Mark Window as dirty
    void setDirty(bool dirty);

    /// @brief Set window's priority
    void setPriority(WindowPriority priority);

    /// @brief Check if window is decorable
    bool hasDecoration(void);
    
    /// @brief Check if window is movable (TODO)
    bool hasMovable(void);

    /// @brief Check if window is unbounded by the boundaries of parent
    bool hasUnbounded(void);

    /// @brief Check if window is dirty
    bool isDirty(void);

    /// @brief Check if mouse coordinates are within Window's boundary
    bool isMouseInBounds(vec2 *mouse);

protected:
    char *windowName; // unused
    int windowID; // Used as index in window stack list
    WindowFlags flags; // Common window options
    Rect *winRect; // Window dimension as Rect
    uint32_t color; // Default background color of window
    WindowPriority priority; // Window priority, used in window list
    Window *parent; // Parent window that self is attached to
    Window *windows[WINDOW_MAX]; // Attached children windows
    int numWindows; // Number of windows currently attached
    const int maxWindows; // Max amount of windows you can attach
    Rect *m_pPrevRect; // Location/size of window on last refresh
    bool m_dirty; // Flags that indicates if Window is dirty
    Window *m_pHoverWindow;    // Which child window the mouse is hovering over
    Window *m_pSelectedWindow; // Which child window was last selected
    // FbContext *context; // Screen buffer info (TODO: remove)
};

} // GUI

#endif // WINDOW_H