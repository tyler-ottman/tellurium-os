#ifndef WINDOW_H
#define WINDOW_H

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

/// @brief Common window flags
enum WindowFlags {
    WNONE = 0x0,
    WDECORATION = 0x1,
    WHOVER = 0x2,
    WMOVABLE = 0x4,
    WUNBOUNDED = 0x8
};

class Window {

public:
    /// @brief Constructor to create a new window
    /// @param windowName Name of the window
    /// @param x Horizontal pixel coordinate position of window
    /// @param y Vertical pixel coordinate position of window
    /// @param width Width of window in pixels
    /// @param height Height of window in pixels
    /// @param flags Window flags
    Window(const char *windowName, int xPos, int yPos, int width, int height,
           WindowFlags flags = WindowFlags::WNONE);

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

    void applyBoundClipping(void);
    void drawWindow(void);
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
    bool onSubtreeUnhover();

    /// @brief Unselect the subtree
    /// @return Operation status
    bool onSubtreeUnselect();

    /// @brief Determine if rectangle intersects with Window
    /// @param rect The rectangle to test
    /// @return If they intersect return true, false otherwise
    bool intersects(Rect *rect);

    /// @brief Get the child Window underneath the mouse
    /// @param mouse the position
    /// @return The child window underneath the mouse, or nullptr if the mouse
    ///         is not hovering over any child window
    Window *getWindowUnderMouse(vec2 *mouse);

    /// @brief Get ID of window
    /// @return windowID
    int getWindowID(void);

    /// @brief Get window's x position
    /// @return Window's x position
    int getX(void);

    /// @brief Get window's y position
    /// @return Window's y position
    int getY(void);

    /// @brief Get window's width
    /// @return Window's width
    int getWidth(void);

    /// @brief Get window's height
    /// @return Window's height
    int getHeight(void);

    /// @brief Get window's color
    /// @return Window's color
    int getColor(void);

    int getNumChildren(void) { return numWindows; }

    Window *getChild(int windowID) { return windows[windowID]; }

    /// @brief Get window's Rect boundary
    /// @return The Rect
    Rect *getWinRect(void);

    /// @brief Set window's ID
    /// @param windowID The window ID
    void setWindowID(int windowID);

    /// @brief Set window's x position
    /// @param x The x position to set
    void setX(int x);

    /// @brief Set window's y position
    /// @param y The y position to set
    void setY(int y);

    /// @brief Set window's (x, y) position
    /// @param xNew The x position to set
    /// @param yNew The y position to set
    void setPosition(int xNew, int yNew);

    /// @brief Set window's width
    /// @param width The width to set
    void setWidth(int width);

    /// @brief Set window's height
    /// @param height The height to set
    void setHeight(int height);

    /// @brief Set window's color
    /// @param color The color to set
    void setColor(uint32_t color);

    /// @brief Mark Window as dirty
    void setDirty(bool dirty);

    /// @brief Set window's priority
    /// @param priority The priortiy to set
    void setPriority(int priority);

    /// @brief Check if window is decorable
    /// @return If window is docorable
    bool hasDecoration(void);
    
    /// @brief Check if window is movable (TODO)
    /// @return If window is movable
    bool hasMovable(void);

    /// @brief Check if window is unbounded by the boundaries of parent
    /// @return If window is unbounded
    bool hasUnbounded(void);

    /// @brief Check if window is dirty
    /// @return true if dirty, false otherwise
    bool isDirty(void);

    /// @brief Check if mouse coordinates are in bounds of window
    /// @param mouse
    /// @return If mouse is in bounds of window, return true, false otherwise
    bool isMouseInBounds(vec2 *mouse);

    /// @brief Recursively update position of parent windows
    /// @param data The delta (x, y) position
    void setChildPositions(Device::MouseData *data);

    void setPrevRect(void) { *m_pPrevRect = *winRect; }

   protected:
    /// @brief Move window to top of window stack
    /// @param refresh The highest level window that was stack, implicates refresh
    /// @param recurse Recursively raise parent windows until root
    /// @return The highest level window that was raised
    void moveToTop(Window **refresh, bool recurse);

    char *windowName; // unused
    int windowID; // Used as index in window stack list
    WindowFlags flags; // Common window options
    Rect *winRect; // Window dimension as Rect
    uint32_t color; // Default background color of window
    int priority; // Window priority, used in window list
    Window *parent; // Parent window that self is attached to
    Window *windows[WINDOW_MAX]; // Attached children windows
    int numWindows; // Number of windows currently attached
    const int maxWindows; // Max amount of windows you can attach
    Rect *m_pPrevRect; // Location/size of window on last refresh
    bool m_dirty; // Flags that indicates if Window is dirty
    Window *m_pHoverWindow;    // Which child window the mouse is hovering over
    Window *m_pSelectedWindow; // Which child window was last selected
    FbContext *context; // Screen buffer info (TODO: remove)
};

} // GUI

#endif // WINDOW_H