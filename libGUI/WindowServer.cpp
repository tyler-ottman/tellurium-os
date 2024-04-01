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

void WindowServer::mouseHandle(Device::MouseData *data) {

}

WindowServer::WindowServer() {

}

WindowServer::~WindowServer() {

}

}