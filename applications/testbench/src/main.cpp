#include "libGUI/Button.hpp"
#include "libGUI/FbContext.hpp"
#include "libGUI/Window.hpp"
#include "libGUI/WindowServer.hpp"
#include <stddef.h>
#include <stdint.h>
#include "ulibc/DevPoll.hpp"
#include "ulibc/mem.hpp"
#include "ulibc/syscalls.hpp"

int main() {
    GUI::WindowServer *wm = GUI::WindowServer::getInstance();
    wm->createWindow("test", 10, 10, 300, 200, WIN_MOVABLE | WIN_DECORATE);
    GUI::Window *win = wm->createWindow("test", 100, 150, 400, 400,
                                        WIN_MOVABLE | WIN_DECORATE);
    wm->createWindow("test", 200, 100, 200, 600, WIN_MOVABLE | WIN_DECORATE);

    GUI::Button *button = new GUI::Button(280, 357, 80, 30);
    win->appendWindow(button);

    button = new GUI::Button(175, 250, 80, 30);
    win->appendWindow(button);

    GUI::Window *smallWin =
        new GUI::Window("test", 300, 200, 100, 100, WIN_MOVABLE | WIN_DECORATE);
    win->appendWindow(smallWin);

    Device::KeyboardData key = {
        .data = 0
    };

    Device::MouseData mouse = {
        .flags = 0,
        .delta_x = 0,
        .delta_y = 0
    };

    wm->refreshScreen();

    while (1) {
        // if (Device::keyboardPoll(&key, 1)) {
        //     // wm->refreshScreen();
        // }

        if (Device::mousePoll(&mouse)) {
            wm->mouseHandle(&mouse);
        }
    }

    for (;;) {}
}