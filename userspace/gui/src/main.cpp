#include "Windowing/FbContext.hpp"
#include "Windowing/Window.hpp"
#include "Windowing/WindowServer.hpp"
#include <stddef.h>
#include <stdint.h>
#include "ulibc/DevPoll.hpp"
#include "ulibc/mem.hpp"
#include "ulibc/syscalls.hpp"

int main() {
    GUI::WindowServer *wm = GUI::WindowServer::getInstance();
    wm->createWindow("test", 10, 10, 300, 200);
    wm->createWindow("test", 100, 150, 400, 400);
    // wm->createWindow("test", 200, 100, 200, 600);

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