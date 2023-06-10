#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "Windowing/FbContext.hpp"
#include "Windowing/Window.hpp"

#define WINDOW_MAX                          10
#define DEFAULT_COLOR                       0xed872d

namespace GUI {

class WindowServer {
public:
    static WindowServer *getInstance();
    Window *createWindow(const char *w_name, int x_pos, int y_pos, int width,
                         int height);
    void fillBackground(void);
    void refreshScreen(void);

private:
    WindowServer();
    ~WindowServer();
    
    const int maxWindows;
    bool isImgLoaded;

    Window *windows[WINDOW_MAX];
    FbContext *context;

    static WindowServer *instance;
};

}