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

    // Restrict drawing of self to inside of border
    geoWin->appendWindow(new GUI::Window(NULL, geoWin->getX() + 1, geoWin->getY() + TITLE_HEIGHT, geoWin->getWidth() - 2, geoWin->getHeight() - TITLE_HEIGHT - 1, GUI::WindowFlags::WNONE, GUI::WindowPriority::WPRIO0));

    // Image window
    const char *imagePath = "/tmp/Universe.bmp";
    GUI::Window *universeWin = new GUI::Window(imagePath, 150, 100, 802, 377 + TITLE_HEIGHT + 1, GUI::WindowFlags::WDECORATION);
    ImageReader *universe = imageReaderDriver(imagePath);
    GUI::Window *universeImg = new GUI::Window(NULL, 151, 100 + TITLE_HEIGHT, universe);
    delete universe;
    universeWin->appendWindow(universeImg);

    // // Terminal window
    GUI::Window *termWin = new GUI::Window("Terminal", 100, 150, 800, 480, GUI::WindowFlags::WDECORATION);
    termWin->setVisible(false);
    GUI::Terminal *term = new GUI::Terminal(100 + 1, 150 + TITLE_HEIGHT + 1, termWin->getWidth() - 2, termWin->getHeight() - 2 - TITLE_HEIGHT);
    term->setTransparent(true);
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