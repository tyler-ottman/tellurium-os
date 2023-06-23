#include "libGUI/WindowServer.hpp"
#include "ulibc/mem.hpp"
#include "ulibc/string.h"

namespace GUI {

WindowServer *WindowServer::instance = nullptr;

WindowServer *WindowServer::getInstance() {
    if (instance) {
        return instance;
    }

    instance = new WindowServer();
    
    return instance;
}

void WindowServer::refreshScreen() {
    desktop->drawWindow();
}

void WindowServer::mouseHandle(Device::MouseData *data) {
    desktop->onMouseEvent(data);

    if (nEvents++ == 10) {
        refreshScreen();
        nEvents = 0;
    }
}

Window *WindowServer::createWindow(const char *name, int x, int y, int width,
                                   int height, uint16_t flags) {
    return desktop->createWindow(name, x, y, width, height, flags);
}

WindowServer::WindowServer() : nEvents(0) {
    desktop = new Desktop();
}

WindowServer::~WindowServer() {

}

}