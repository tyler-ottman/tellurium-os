#include "flibc/string.h"
#include "libGUI/WindowServer.hpp"
#include "libTellur/mem.hpp"

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

Window *WindowServer::getRoot() { return desktop; }

WindowServer::WindowServer() : nEvents(0) {
    desktop = new Desktop();
}

WindowServer::~WindowServer() {

}

}