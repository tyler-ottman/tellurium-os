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
    GUI::Window *geoWin = new GUI::Window("Geometry", 10, 10, 400, 400,
                                          GUI::WindowFlags::WDECORATION);
    GUI::Window *square1 = new GUI::Window("square", 80, 70, 200, 220);
    GUI::Window *square2 = new GUI::Window("square", 150, 100, 200, 70);
    GUI::Window *square3 = new GUI::Window("square", 100, 250, 150, 100);
    GUI::Window *square4 = new GUI::Window("square", 220, 130, 100, 140);
    square1->loadTransparentColor(0xff89cff0);
    square2->loadTransparentColor(0xffd1ffbd);
    square3->loadTransparentColor(0xfffaa0a0);
    square4->loadTransparentColor(0xffffd580);

    ImageReader *pinkCircle = imageReaderDriver("/tmp/Circle.bmp");
    GUI::Window *bmpCircle = new GUI::Window("Circle", 100, 100, pinkCircle);
    bmpCircle->setTransparent(true);
    geoWin->appendWindow(bmpCircle);
    
    geoWin->appendWindow(square1);
    geoWin->appendWindow(square2);
    geoWin->appendWindow(square3);
    geoWin->appendWindow(square4);
    geoWin->appendWindow(new GUI::Window(NULL, 20, 70, 100, 100,
                         GUI::WindowFlags::WDECORATION));

    // Image window
    const char *imagePath = "/tmp/basketOfFruits.ppm";
    GUI::Window *fruitWin = new GUI::Window(imagePath, 150, 100, 402, 316 + TITLE_HEIGHT, GUI::WindowFlags::WDECORATION);
    ImageReader *fruit = imageReaderDriver(imagePath);
    GUI::Window *fruitImg = new GUI::Window(NULL, 151, 101 + TITLE_HEIGHT, fruit);
    delete fruit;
    fruitWin->appendWindow(fruitImg);

    // Terminal window
    GUI::Window *termWin = new GUI::Window("Terminal", 100, 150, 500, 300,
        GUI::WindowFlags::WDECORATION);
    termWin->setVisible(false);
    GUI::Terminal *term = new GUI::Terminal(
        100 + 1, 150 + TITLE_HEIGHT + 1, termWin->getWidth() - 2,
        termWin->getHeight() - 2 - TITLE_HEIGHT);
    term->setTransparent(true);
    termWin->appendWindow(term);
    term->printf("Hello World!\n");
    for (size_t i = 0; i < 16; i++) {
        if (i % 8 == 0) term->printf("\n");
        term->printf("\033[38;5;%i;48;5;%im%03i", i, i, i);
    }

    // Attach test windows to root window
    GUI::CWindow *wapp = GUI::CWindow::getInstance();

    GUI::Window *transparent = new GUI::Window(nullptr, 350, 350, 100, 100);
    transparent->loadTransparentColor(0x008080);
    wapp->appendWindow(transparent);
    transparent = new GUI::Window(nullptr, 400, 400, 100, 100);
    transparent->loadTransparentColor(0x800000);
    wapp->appendWindow(transparent);

    wapp->appendWindow(geoWin);
    wapp->appendWindow(fruitWin);
    wapp->appendWindow(termWin);

    while (1) {
        wapp->pollEvents();
    }
}