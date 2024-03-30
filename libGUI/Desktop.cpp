#include "libGUI/Desktop.hpp"
#include "libGUI/Image.hpp"
#include "libGUI/Taskbar.hpp"
#include "libGUI/Terminal.hpp"

namespace GUI {

#define MOUSE_W                             11
#define MOUSE_H                             18

#define BL                                  0xff000000
#define CL                                  0x0
#define WH                                  0xffffffff

uint32_t mouseBitmap[MOUSE_W * MOUSE_H] = {  
    BL, CL, CL, CL, CL, CL, CL, CL, CL, CL, CL,
    BL, BL, CL, CL, CL, CL, CL, CL, CL, CL, CL,
    BL, WH, BL, CL, CL, CL, CL, CL, CL, CL, CL,
    BL, WH, WH, BL, CL, CL, CL, CL, CL, CL, CL,
    BL, WH, WH, WH, BL, CL, CL ,CL, CL, CL, CL,
    BL, WH, WH, WH, WH, BL, CL, CL, CL, CL, CL,
    BL, WH, WH, WH, WH, WH, BL, CL, CL, CL, CL,
    BL, WH, WH, WH, WH, WH, WH, BL, CL, CL, CL,
    BL, WH, WH, WH, WH, WH, WH, WH, BL, CL, CL,
    BL, WH, WH, WH, WH, WH, WH, WH, WH, BL, CL,
    BL, WH, WH, WH, WH, WH, WH, WH, WH, WH, BL,
    BL, WH, WH, WH, WH, WH, WH, BL, BL, BL, BL,
    BL, WH, WH, WH, BL, WH, WH, BL, CL, CL, CL,
    BL, WH, WH, BL, BL, WH, WH, BL, CL, CL, CL,
    BL, WH, BL, CL, CL, BL, WH, WH, BL, CL, CL,
    BL, BL, CL, CL, CL, BL, WH, WH, BL, CL, CL,
    BL, CL, CL, CL, CL, CL, BL, WH, BL, CL, CL,
    CL, CL, CL, CL, CL, CL, CL, BL, BL, CL, CL 
};

Desktop::Desktop()
    : Window(NULL, 0, 0, FbContext::getInstance()->getFbContext()->fb_width,
             FbContext::getInstance()->getFbContext()->fb_height, WIN_DECORATE),
      forceRefresh(true) {
    mouseX = winRect->getWidth() / 2;
    mouseY = winRect->getHeight() / 2;
    oldMouseX = mouseX;
    oldMouseY = mouseY;

    Image *background = new Image(0, 0, context->getFbContext()->fb_width,
                                  context->getFbContext()->fb_height);
    
    background->loadImage("/tmp/background.ppm");
    appendWindow(background);

    Taskbar *taskbar = new Taskbar(0, winRect->getHeight() - 40, winRect->getWidth(), 40);
    taskbar->setColor(0xffbebebe);
    appendWindow(taskbar);

    // Sample home button
    Button *homeButton = new Button(taskbar->getXPos(), taskbar->getYPos(), 40,
                                    40, BUTTON_HOVER);
    homeButton->loadImage("/tmp/homeButtonUnhover.ppm");
    homeButton->loadHoverImage("/tmp/homeButtonHover.ppm");
    taskbar->appendWindow(homeButton);

    // Sample clock
    Terminal *clock = new Terminal(taskbar->getWidth() - 50, taskbar->getYPos() + 14, 50, 20);
    clock->setBg(0xffbebebe);
    clock->setFg(0);
    clock->disableCursor();
    clock->clear();
    clock->printf("12:49");
    taskbar->appendWindow(clock);
}

Desktop::~Desktop() {

}

void Desktop::drawWindow() {
    if (forceRefresh) {
        Window::drawWindow();
        forceRefresh = false;
    }

    // If window was dragged since last refresh, update it
    applyDirtyDrag();
    applyDirtyMouse();

    Window::drawWindow();

    // Draw mouse
    drawMouse();    
}

void Desktop::drawMouse() {
    context->drawBitmapNoRegion(mouseX, mouseY, MOUSE_W, MOUSE_H, mouseBitmap);
}

void Desktop::onMouseEvent(Device::MouseData *data) {
    context->resetClippedList();
    context->resetDirtyList();

    Window::onMouseEvent(data, mouseX, mouseY);

    // Regions that need immediate refresh
    if (context->getNumDirty()) {
        applyDirtyMouse();

        Window::drawWindow();

        drawMouse();
        context->resetDirtyList();
    }

    updateMousePos(data);

    lastMouseState = data->flags;
}

void Desktop::applyDirtyMouse(void) {
    // If mouse position changed since last refresh
    if (!(oldMouseX == mouseX && oldMouseY == mouseY)) {
        // Original mouse position
        Rect oldMouse(oldMouseX, oldMouseY, MOUSE_W, MOUSE_H);
        context->addClippedRect(&oldMouse);

        // New mouse position
        Rect newMouse(mouseX, mouseY, MOUSE_W, MOUSE_H);
        context->addClippedRect(&newMouse);

        oldMouseX = mouseX;
        oldMouseY = mouseY;

        context->moveClippedToDirty();
    }
}

void Desktop::updateMousePos(Device::MouseData *data) {
    int xNew = mouseX + data->delta_x;
    int yNew = mouseY - data->delta_y;

    FbMeta *meta = FbContext::getInstance()->getFbContext();
    if (xNew >= 0 && xNew < (int)meta->fb_width) {
        mouseX = xNew;
    }

    if (yNew >= 0 && yNew < (int)meta->fb_height) {
        mouseY = yNew;
    }
}

bool Desktop::mouseInBounds(Window *window) {
    return ((mouseX >= window->getXPos()) &&
            (mouseX <= (window->getXPos() + window->getWidth())) &&
            (mouseY >= window->getYPos()) &&
            (mouseY <= (window->getYPos() + window->getHeight())));
}
}

