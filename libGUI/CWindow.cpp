#include "libGUI/CWindow.hpp"
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

CWindow *CWindow::instance = nullptr;

CWindow *CWindow::getInstance() {
    if (instance) {
        return instance;
    }

    instance = new CWindow();
    
    return instance;
}

void CWindow::refresh() {
    // If window was dragged since last refresh, update it
    applyDirtyDrag();
    applyDirtyMouse();

    drawWindow();

    // Draw mouse
    drawMouse();    
}

void CWindow::forceRefresh() {
    drawWindow();
    drawMouse(); 
}

void CWindow::drawMouse() {
    context->drawBitmapNoRegion(mouseX, mouseY, MOUSE_W, MOUSE_H, mouseBitmap);
}

void CWindow::processEvent(Device::TellurEvent *event) {
    context->resetClippedList();
    context->resetDirtyList();

    onEvent(event, mouseX, mouseY);

    // Regions that need immediate refresh
    if (context->getNumDirty()) {
        applyDirtyMouse();

        drawWindow();

        drawMouse();
        context->resetDirtyList();
    }

    if (event->isMouseEvent()) {
        Device::MouseData *mouseData = (Device::MouseData *)event->data;
        updateMousePos(mouseData);
        lastMouseState = mouseData->flags;
    }

    if (nEvents++ == 5) {
        refresh();
        nEvents = 0;
    }
}

void CWindow::pollEvents() {
    Device::TellurEvent *event = devManager->pollDevices();
    if (event) {
        processEvent(event);
    }
}

void CWindow::applyDirtyMouse(void) {
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

CWindow::CWindow()
    : Window(NULL, 0, 0, FbContext::getInstance()->getFbContext()->fb_width,
             FbContext::getInstance()->getFbContext()->fb_height), nEvents(0) {
    mouseX = getWidth() / 2;
    mouseY = getHeight() / 2;
    oldMouseX = mouseX;
    oldMouseY = mouseY;

    Image *background = new Image(0, 0, context->getFbContext()->fb_width,
                                  context->getFbContext()->fb_height);
    
    background->loadImage("/tmp/background.ppm");
    appendWindow(background);

    Taskbar *taskbar = new Taskbar(0, getHeight() - 40, getWidth(), 40);
    taskbar->setColor(0xffbebebe);
    appendWindow(taskbar);

    // Sample home button
    Button *homeButton = new Button(taskbar->getX(), taskbar->getY(), 40, 40,
                                    WindowFlags::WNONE, ButtonFlags::BHOVER);
    homeButton->loadImage("/tmp/homeButtonUnhover.ppm");
    homeButton->loadHoverImage("/tmp/homeButtonHover.ppm");
    taskbar->appendWindow(homeButton);

    // Sample clock
    Terminal *clock = new Terminal(taskbar->getWidth() - 50, taskbar->getY() + 14, 50, 20);
    clock->setBg(0xffbebebe);
    clock->setFg(0);
    clock->disableCursor();
    clock->clear();
    clock->printf("12:49");
    taskbar->appendWindow(clock);

    devManager = new Device::DeviceManager;
    devManager->addDevice(new Device::DeviceMousePs2("/dev/ms0"));
    // devManager->addDevice(new Device::DeviceKeyboardPs2("/dev/kb0"));    
}

CWindow::~CWindow() {}

void CWindow::updateMousePos(Device::MouseData *data) {
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

bool CWindow::mouseInBounds(Window *window) {
    return ((mouseX >= window->getX()) &&
            (mouseX <= (window->getX() + window->getWidth())) &&
            (mouseY >= window->getY()) &&
            (mouseY <= (window->getY() + window->getHeight())));
}

}

namespace Device {

}