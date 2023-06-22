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
    desktop->onMouseMove(data);

    if (nEvents++ == 10) {
        refreshScreen();
        nEvents = 0;
    }
}

void WindowServer::createWindow(const char *name, int x, int y, int width,
                                int height, uint16_t flags) {
    desktop->createWindow(name, x, y, width, height, flags);
}

WindowServer::WindowServer() : nEvents(0) {
    desktop = new Desktop();
}

WindowServer::~WindowServer() {

}

}