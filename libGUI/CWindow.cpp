#include "Button.hpp"
#include "CWindow.hpp"
#include "Image.hpp"
#include "Taskbar.hpp"
#include "Terminal.hpp"

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

    FbInfo *fbInfo = new FbInfo;
    syscall_get_fb_context(fbInfo);

    instance = new CWindow(fbInfo);
    
    return instance;
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
        compositor->render(this, mouse);

        nEvents = 0;
    }
}

CWindow::CWindow(FbInfo *fbInfo)
    : Window(NULL, 0, 0, fbInfo->fb_width,
             fbInfo->fb_height), nEvents(0), fbInfo(fbInfo) {
    mouse = new Window("mouse", getWidth() / 2, getHeight() / 2, MOUSE_W,
                       MOUSE_H, WindowFlags::WNONE, WindowPriority::WPRIO9);
    mouse->loadBuff(mouseBitmap); // Copy mouse image to its buffer

    compositor = new Compositor(fbInfo);

    Image *background = new Image(0, 0, fbInfo->fb_width, fbInfo->fb_height);

    background->loadImage("/tmp/background.ppm");
    appendWindow(background);

    Taskbar *taskbar = new Taskbar(0, getHeight() - 40, getWidth(), 40);
    taskbar->loadBuff(0xffbebebe);
    appendWindow(taskbar);

    // Sample home button
    Button *homeButton = new Button(taskbar->getX(), taskbar->getY(), 40, 40,
                                    WindowFlags::WNONE, ButtonFlags::BHOVER);
    homeButton->loadImage("/tmp/homeButtonUnhover.ppm");
    homeButton->loadHoverImage("/tmp/homeButtonHover.ppm");
    taskbar->appendWindow(homeButton);

    // Sample clock
    Terminal *clock = new Terminal(taskbar->getWidth() - 50,
        taskbar->getY() + 14, 50, 20);
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
    setDirty(true);
}

CWindow::~CWindow() {}

// Todo: Screen bound only calculated by Window Server
void CWindow::updateMousePos(Device::MouseData *data) {
    int xNew = mouse->getX() + data->delta_x;
    int yNew = mouse->getY() - data->delta_y;

    if (xNew >= 0 && xNew < (int)(fbInfo->fb_width)) {
        mouse->setX(xNew);
    }

    if (yNew >= 0 && yNew < (int)(fbInfo->fb_height)) {
        mouse->setY(yNew);
    }

    mouse->setDirty(true);
}

}
