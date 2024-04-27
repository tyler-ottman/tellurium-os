#ifndef WINDOW_H
#define WINDOW_H

#include "libGUI/Rect.hpp"
#include "libTellur/DevPoll.hpp"
#include "libTellur/ImageReader.hpp"
#include "Surface.hpp"
#include <stdbool.h>
#include <stdint.h>

#define WINDOW_MAX                          20
#define TITLE_HEIGHT                        31
#define TITLE_WIDTH                         (width)

namespace GUI {

class Compositor;

/// @brief Common window flags
typedef int WindowFlags;
enum WindowFlags_ {
    WindowFlags_None                            = 0x00,
    WindowFlags_Decoration                      = 0x01,
    WindowFlags_Hover                           = 0x02,
    WindowFlags_Movable                         = 0x04,

    // Flags used by compositor when rendering windows
    WindowFlags_Transparent                     = 0x08, // If window is transparent, use alpha blending
    WindowFlags_Invisible                       = 0x10, // If window is not marked invisible, then render it
    WindowFlags_Dirty                           = 0x20 // If window state has changed, re-render is required
};

/// @brief Common Window priorities
typedef int WindowPriority;
enum WindowPriority_ {
    WindowPriority_0                            = 0,
    WindowPriority_1                            = 1,
    WindowPriority_2                            = 2,
    WindowPriority_3                            = 3,
    WindowPriority_4                            = 4,
    WindowPriority_5                            = 5,
    WindowPriority_6                            = 6,
    WindowPriority_7                            = 7,
    WindowPriority_8                            = 8,
    WindowPriority_9                            = 9
};

class Window {
    friend class Compositor;

public:
    /// @brief Constructor to create a new window (TODO: 2-phase initialization)
    /// @param windowName Name of the window
    /// @param x Horizontal pixel coordinate position of window
    /// @param y Vertical pixel coordinate position of window
    /// @param width Width of window in pixels
    /// @param height Height of window in pixels
    /// @param flags Window flags
    Window(const char *windowName, int xPos, int yPos, int width, int height,
           WindowFlags flags = WindowFlags_None,
           WindowPriority priority = WindowPriority_2);

    /// @brief Constructor to create a new window using an image
    /// @param windowName Name of the window
    /// @param x Horizontal pixel coordinate position of window
    /// @param y Vertical pixel coordinate position of window
    /// @param img The image reader
    /// @param flags Window flags
    Window(const char *windowName, int xPos, int yPos, const char *path,
           WindowFlags flags = WindowFlags_None,
           WindowPriority priority = WindowPriority_2);

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
                         int height, WindowFlags flags = WindowFlags_None);

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
    Window *removeWindow(Window *window);;

    //// @brief Process event
    /// @param data Incoming mouse data
    /// @param mouse Position of mouse
    /// @return Event process status
    virtual bool onEvent(Device::TellurEvent *data, Window *mouse);
    
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
    bool intersects(Window *rect);

    /// @brief Get the child Window underneath the mouse
    /// @param mouse the position
    /// @return The child window underneath the mouse, or nullptr if the mouse
    ///         is not hovering over any child window
    Window *getWindowUnderMouse(Window *mouse);

    /// @brief Recursively update position of window and descendant
    /// @param data The delta (x, y) position
    void updateChildPositions(Device::MouseData *data);

    /// @brief Move current Window dimensions to previous Window dimension
    void updatePrevRect(void);

    /// @brief Move window to top of window stack
    /// @param child Recursively the child to raise on the window stack
    /// @return The highest level window that was raised
    bool moveToTop(Window *child);

    /// @brief Copy buff or color into winBuff
    void loadBuff(uint32_t *buff);
    void loadBuff(uint32_t color);
    void loadTransparentColor(uint32_t color); // For testing

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

    /// @brief Get window's Rect boundary
    Rect *getWinRect(void);

    /// @brief Get window's previous Rect boundary
    Rect *getPrevRect(void);

    /// @brief Get status of Window flag bit
    bool getFlag(WindowFlags field);

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

    /// @brief Set window's priority
    void setPriority(WindowPriority priority);

    /// @brief Set a single or multiple Window flag bits
    void setFlags(WindowFlags fields);
    
    /// @brief Clear a single of multiple Window flag bits
    void resetFlags(WindowFlags fields);

    /// @brief Check if coordinates are within Window's boundary
    bool isCoordInBounds(int x, int y);

protected:
    void initialize(const char *windowName, int x, int y, int width, int height,
               WindowFlags flags, WindowPriority priority);

    char *windowName; // Used for Titles on Windows with decorations
    int windowID; // Used as index in window stack list
    WindowFlags flags; // Common window options
    Surface *surface; // The Surface represents (TODO: use relative coordinates)
    uint32_t color; // Default background color of window
    WindowPriority priority; // Window priority, used in window list
    Window *parent; // Parent window that self is attached to
    Window *windows[WINDOW_MAX]; // Attached children windows
    int numWindows; // Number of windows currently attached
    int maxWindows; // Max amount of windows you can attach
    Rect *winPrevRect; // Location/size of window on last refresh
    Window *hoverWindow;    // Which child window the mouse is hovering over
    Window *selectedWindow; // Which child window was last selected
};

} // GUI

#endif // WINDOW_H