#include "libGUI/Button.hpp"
#include "libGUI/CWindow.hpp"
#include "libGUI/Terminal.hpp"
#include "libGUI/Window.hpp"
#include "libTellur/mem.hpp"
#include "libTellur/syscalls.hpp"
#include <stddef.h>
#include <stdint.h>

int main() {
    // Geometry Window
    GUI::Window *geoWin = new GUI::Window("Geometry", 10, 10, 700, 400, GUI::WindowFlags_Decoration);
    geoWin->appendWindow(new GUI::Window(nullptr, 275, 150, "/tmp/Pentagon.bmp", GUI::WindowFlags_Transparent));
    geoWin->appendWindow(new GUI::Window(nullptr, 90, 100, "/tmp/Circle.bmp", GUI::WindowFlags_Transparent));
    geoWin->appendWindow(new GUI::Window(nullptr, 170, 150, "/tmp/Square.bmp", GUI::WindowFlags_Transparent));
    geoWin->appendWindow(new GUI::Window(nullptr, 380, 150, "/tmp/Triangle.bmp", GUI::WindowFlags_Transparent));
    geoWin->appendWindow(new GUI::Window(nullptr, 440, 100, "/tmp/Star.bmp", GUI::WindowFlags_Transparent));
    geoWin->appendWindow(new GUI::Window(nullptr, geoWin->getX() + 1, geoWin->getY() + TITLE_HEIGHT, geoWin->getWidth() - 2, geoWin->getHeight() - TITLE_HEIGHT - 1, GUI::WindowFlags_None, GUI::WindowPriority_0));

    // Image window
    const char *imagePath = "/tmp/Universe.bmp";
    GUI::Window *universeWin = new GUI::Window(imagePath, 150, 100, 802, 377 + TITLE_HEIGHT + 1, GUI::WindowFlags_Decoration);
    universeWin->appendWindow(new GUI::Window(nullptr, 151, 100 + TITLE_HEIGHT, imagePath));

    // Terminal window
    GUI::Window *termWin = new GUI::Window("Terminal", 100, 150, 800, 480, GUI::WindowFlags_Decoration | GUI::WindowFlags_Invisible);
    GUI::Terminal *term = new GUI::Terminal(100 + 1, 150 + TITLE_HEIGHT + 1, termWin->getWidth() - 2, termWin->getHeight() - 2 - TITLE_HEIGHT, GUI::WindowFlags_Transparent);
    termWin->appendWindow(term);
    term->printf("Hello World! This is a testbench.\n");
    for (size_t i = 0; i < 16; i++) {
        if (i % 8 == 0) term->printf("\n");
        term->printf("\033[38;5;%i;48;5;%im%03i", i, i, i);
    }

    // Attach test windows to root window
    GUI::CWindow *wapp = GUI::CWindow::getInstance();

    wapp->appendWindow(geoWin);
    wapp->appendWindow(universeWin);
    wapp->appendWindow(termWin);

    while (1) {
        wapp->pollEvents();
    }
}