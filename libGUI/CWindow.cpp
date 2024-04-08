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

void CWindow::applyDirtyRegions() {
    if (selectedWindow && !oldSelected->equals(selectedWindow->getWinRect())) {
        context->addDirtyRect(oldSelected);
        context->addDirtyRect(selectedWindow->getWinRect());

        *oldSelected = *(selectedWindow->getWinRect());
    }

    if (!oldMouse->equals(mouse)) {
        // Original mouse position
        Rect old(oldMouse->x, oldMouse->y, MOUSE_W, MOUSE_H);
        context->addDirtyRect(&old);

        // New mouse position
        Rect newMouse(mouse->x, mouse->y, MOUSE_W, MOUSE_H);
        context->addDirtyRect(&newMouse);

        *oldMouse = *mouse;
    }
}

void CWindow::refresh() {
    // If selectedWindow position changed since last refresh, add to dirty
    applyDirtyRegions();

    // Only refresh if dirty regions generated
    if (context->getNumDirty()) {
        // Draw the windows that intersect the dirty clipped regions
        drawWindow();

        // Draw mouse on top of final image, does not use clipped regions
        drawMouse();

        // Rendering done, reset dirty/clipped regions list
        context->resetClippedList();
        context->resetDirtyList();
    }
}

void CWindow::drawMouse() {
    context->drawBitmapNoRegion(mouse->x, mouse->y, MOUSE_W, MOUSE_H, mouseBitmap);
}

void CWindow::processEvent(Device::TellurEvent *event) {
    onEvent(event, mouse);

    if (event->isMouseMove()) {
        updateMousePos((Device::MouseData *)event->data);
    }
}

void CWindow::pollEvents() {
    Device::TellurEvent *event = devManager->pollDevices();
    if (event) {
        processEvent(event);
    }

    if (nEvents++ == 5) {
        refresh();
        nEvents = 0;
    }
}

CWindow::CWindow()
    : Window(NULL, 0, 0, FbContext::getInstance()->getFbContext()->fb_width,
             FbContext::getInstance()->getFbContext()->fb_height), nEvents(0) {
    mouse = new vec2(getWidth() / 2, getHeight() / 2);
    oldMouse = new vec2(mouse->x, mouse->y);

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

    // Entire screen is dirty on startup
    context->addDirtyRect(winRect);
}

CWindow::~CWindow() {}

void CWindow::updateMousePos(Device::MouseData *data) {
    int xNew = getMouseX() + data->delta_x;
    int yNew = getMouseY() - data->delta_y;

    FbMeta *meta = FbContext::getInstance()->getFbContext();
    if (xNew >= 0 && xNew < (int)meta->fb_width) {
        mouse->x = xNew;
    }

    if (yNew >= 0 && yNew < (int)meta->fb_height) {
        mouse->y = yNew;
    }
}

bool CWindow::mouseInBounds(Window *window) {
    return ((getMouseX() >= window->getX()) &&
            (getMouseX() <= (window->getX() + window->getWidth())) &&
            (getMouseY() >= window->getY()) &&
            (getMouseY() <= (window->getY() + window->getHeight())));
}

}
