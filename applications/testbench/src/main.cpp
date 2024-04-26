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
    GUI::Window *geoWin = new GUI::Window("Geometry", 10, 10, 700, 400,
                                          GUI::WindowFlags::WDECORATION);

    ImageReader *redPentagon = imageReaderDriver("/tmp/Pentagon.bmp");
    GUI::Window *bmpPentagon = new GUI::Window("Pentagon", 275, 150, redPentagon);
    bmpPentagon->setTransparent(true);
    geoWin->appendWindow(bmpPentagon);

    ImageReader *pinkCircle = imageReaderDriver("/tmp/Circle.bmp");
    GUI::Window *bmpCircle = new GUI::Window("Circle", 90, 100, pinkCircle);
    bmpCircle->setTransparent(true);
    geoWin->appendWindow(bmpCircle);

    ImageReader *greenSquare = imageReaderDriver("/tmp/Square.bmp");
    GUI::Window *bmpSquare = new GUI::Window("Square", 170, 150, greenSquare);
    bmpSquare->setTransparent(true);
    geoWin->appendWindow(bmpSquare);

    ImageReader *blueTriangle = imageReaderDriver("/tmp/Triangle.bmp");
    GUI::Window *bmpTriangle = new GUI::Window("Triangle", 380, 150, blueTriangle);
    bmpTriangle->setTransparent(true);
    geoWin->appendWindow(bmpTriangle);

    ImageReader *orangeStar = imageReaderDriver("/tmp/Star.bmp");
    GUI::Window *bmpStar = new GUI::Window("Pentagon", 440, 100, orangeStar);
    bmpStar->setTransparent(true);
    geoWin->appendWindow(bmpStar);

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