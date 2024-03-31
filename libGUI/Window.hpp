#pragma once

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

enum WindowType {
    WindowBorder,
    WindowButton,
    WindowDefault,
    WindowImage,
    WindowMenuBar
};

/// @brief Common window flags
enum WindowFlags {
    WNONE = 0x0,
    WDECORATION = 0x1,
    WHOVER = 0x2,
    WMOVABLE = 0x4,
};

class MenuBar;

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
    /// @param windowID The window ID of the child to be removed
    /// @return Upon successful removal, return a pointer to the removed window, nullptr otherwise
    Window *removeWindow(int windowID);

    /// @brief Remove the specified window from the windows list
    /// @param window The window to be removed
    /// @return If window is invalid, return nullptr, the delete window
    Window *removeWindow(Window *window);

    bool attachMenuBar(MenuBar *menuBar);

    void applyBoundClipping(void);
    void applyDirtyDrag(void);

    bool intersects(Rect *rect);
    void updatePosition(int xNew, int yNew);

    void drawWindow(void);
    virtual void drawObject(void);
    
    // Mouse event stubs

    // Mouse events
    bool onMouseEvent(Device::MouseData *data, int mouseX, int mouseY);
    bool onWindowRaise(void);
    bool onWindowDrag(Device::MouseData *data);
    bool onWindowRelease(void);
    bool onWindowClick(void);
    bool onWindowSelect(void);
    bool onWindowUnselect(void);
    bool onWindowHover(void);
    bool onWindowUnhover(void);
    
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

    /// @brief Set window's ID
    /// @param windowID The window ID
    void setWindowID(int windowID);

    /// @brief Set window's x position
    /// @param x The x position to set
    void setX(int x);

    /// @brief Set window's y position
    /// @param y The y position to set
    void setY(int y);

    /// @brief Set window's width
    /// @param width The width to set
    void setWidth(int width);

    /// @brief Set window's height
    /// @param height The height to set
    void setHeight(int height);

    /// @brief Set window's color
    /// @param color The color to set
    void setColor(uint32_t color);

    /// @brief Set window's priority
    /// @param priority The priortiy to set
    void setPriority(int priority);

    bool isLastMousePressed(void);

    /// @brief Check if window is decorable
    /// @return If window is docorable
    bool hasDecoration(void);
    
    /// @brief Check if window is movable (TODO)
    /// @return If window is movable
    bool hasMovable(void);

    bool isOnMenuBar(int mouseX, int mouseY);
    bool isMouseInBounds(int mouseX, int mouseY);

protected:
    void moveToTop(Window *window);
    void moveThisToDirty(void);
    void drawBorder(void);
    void updateRect(void);
    void updateChildPositions(Device::MouseData *data);

    char *windowName; // unused
    int windowID; // Used as index in window stack list
    WindowFlags flags; // Common window options
    Rect *winRect; // Window dimension as Rect
    uint32_t color; // Default background color of window
    int type; // Type of window, if derived (TODO: remove)
    int priority; // Window priority, used in window list
    Window *parent; // Parent window that self is attached to
    Window *windows[WINDOW_MAX]; // Attached children windows
    int numWindows; // Number of windows currently attached
    const int maxWindows; // Max amount of windows you can attach
    MenuBar *menuBar; // Window bar (TODO: remove)
    FbContext *context; // Screen buffer info (TODO: remove)
    Window *activeChild; // Activetly selected window
    
    static uint8_t lastMouseState; // State of mouse from last event
    static Window *selectedWindow; // Current selected window
    static Window *hoverWindow; // Window that mouse is hovering over
    static int xOld; // Old x position of selectedWindow for dirty calculations
    static int yOld; // Old y position of selectedWindow
};

class MenuBar : public Window {
public:
    MenuBar(int x, int y, int width, int height, WindowFlags flags = WindowFlags::WNONE);
    ~MenuBar();

    void onMouseClick(void);
    void onBarSelect(void);
    void onBarUnselect(void);

    void drawObject(void);

    uint32_t getBarColor(void);
private:
    void setBarColor(uint32_t color);

    uint32_t barColor;
};

class Border : public Window {
public:
    Border(int x, int y, int width, int height, WindowFlags flags = WindowFlags::WNONE);
    ~Border();

    void drawObject(void);
};

}