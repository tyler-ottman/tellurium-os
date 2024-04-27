#include "Button.hpp"
#include "CWindow.hpp"
#include "libTellur/ImageReader.hpp"
#include "Taskbar.hpp"
#include "Terminal.hpp"

namespace GUI {

CWindow *CWindow::instance = nullptr;

CWindow *CWindow::getInstance() {
    if (instance) {
        return instance;
    }

    FbInfo *fbInfo = new FbInfo;
    syscall_get_fb_context(fbInfo);

    Rect fbBoundary(0, 0, fbInfo->fb_width, fbInfo->fb_height);
    Surface *screen = new Surface(fbBoundary, (uint32_t*)fbInfo->fb_buff);

    instance = new CWindow(screen);
    
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

CWindow::CWindow(Surface *surface)
    : Window(NULL, 0, 0, surface->rect.getWidth(),
             surface->rect.getHeight()), nEvents(0), surface(surface) {
    mouse = new Window("mouse", getWidth() / 2, getHeight() / 2, "/tmp/mouse.ppm",
                       WindowFlags_None, WindowPriority_9);
    compositor = new Compositor(surface);

    Window *background = new Window(NULL, 0, 0, "/tmp/background.bmp");
    appendWindow(background);

    Taskbar *taskbar = new Taskbar(0, getHeight() - 40, getWidth(), 40);
    taskbar->loadBuff(0xffbebebe);
    appendWindow(taskbar);

    // Sample home button
    Button *homeButton = new Button(taskbar->getX(), taskbar->getY(), "/tmp/homeButtonUnhover.ppm", WindowFlags_None, ButtonFlags_Hover);
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
    setFlags(WindowFlags_Dirty);
}

CWindow::~CWindow() {}

// Todo: Screen bound only calculated by Window Server
void CWindow::updateMousePos(Device::MouseData *data) {
    int xNew = mouse->getX() + data->delta_x;
    int yNew = mouse->getY() - data->delta_y;

    if (xNew >= 0 && xNew < (int)(surface->rect.getWidth())) {
        mouse->setX(xNew);
    }

    if (yNew >= 0 && yNew < (int)(surface->rect.getHeight())) {
        mouse->setY(yNew);
    }

    mouse->setFlags(WindowFlags_Dirty);
}

}
