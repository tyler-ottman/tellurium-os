#include "libGUI/Button.hpp"
#include "libGUI/FbContext.hpp"
#include "libGUI/Terminal.hpp"
#include "libGUI/Window.hpp"
#include "libGUI/WindowServer.hpp"
#include "libTellur/DevPoll.hpp"
#include "libTellur/mem.hpp"
#include "libTellur/syscalls.hpp"
#include <stddef.h>
#include <stdint.h>

int main() {
    GUI::WindowServer *wm = GUI::WindowServer::getInstance();
    
    // Geometry Window
    GUI::Window *win0 =
        wm->createWindow("Geometry", 10, 10, 400, 400, WIN_MOVABLE | WIN_DECORATE);
    GUI::Window *square1 = new GUI::Window("square", 80, 70, 200, 220, 0);
    GUI::Window *square2 = new GUI::Window("square", 150, 100, 200, 70, 0);
    GUI::Window *square3 = new GUI::Window("square", 100, 250, 150, 100, 0);
    GUI::Window *square4 = new GUI::Window("square", 220, 130, 100, 140, 0);
    square1->setColor(0xff89cff0);
    square2->setColor(0xffd1ffbd);
    square3->setColor(0xfffaa0a0);
    square4->setColor(0xffffd580);
    win0->appendWindow(square1);
    win0->appendWindow(square2);
    win0->appendWindow(square3);
    win0->appendWindow(square4);
    GUI::Window *smallWin =
        new GUI::Window(NULL, 20, 70, 100, 100, WIN_MOVABLE | WIN_DECORATE);
    win0->appendWindow(smallWin);

    // Image window
    const char *imagePath = "/tmp/basketOfFruits.ppm";
    GUI::Window *win1 =
        wm->createWindow(imagePath, 150, 100, 402, 316 + TITLE_HEIGHT,
                         WIN_MOVABLE | WIN_DECORATE);
    GUI::Image *fruit =
        new GUI::Image(151, 101 + TITLE_HEIGHT, win1->getWidth() - 2,
                       win1->getHeight() - 2 - TITLE_HEIGHT);
    win1->appendWindow(fruit);
    fruit->loadImage(imagePath);

    // Terminal window
    GUI::Window *win2 = wm->createWindow("Terminal", 100, 150, 500, 300,
                                         WIN_MOVABLE | WIN_DECORATE);
    GUI::Terminal *term =
        new GUI::Terminal(100 + 1, 150 + TITLE_HEIGHT + 1, win2->getWidth() - 2,
                          win2->getHeight() - 2 - TITLE_HEIGHT);
    win2->appendWindow(term);
    term->printf("TelluriumOS - Hello World!\n");
    for (size_t i = 0; i < 16; i++) {
        if (i % 8 == 0) {
            term->printf("\n");
        }
        term->printf("\033[38;5;%i;48;5;%im%03i", i, i, i);
    }

    Device::KeyboardData key = {.data = 0};

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