#include "DevPoll.hpp"
#include "flibc/string.h"

namespace Device {
    
static int mouseFd = 0;
static int keyboardFd = 0;

int mousePoll(MouseData *data) {
    if (!mouseFd) {
        mouseFd = syscall_open("/dev/ms0", 0);
    }

    return syscall_read(mouseFd, (void *)data, sizeof(MouseData));
}

int keyboardPoll(KeyboardData *data, size_t count) {
    if (!keyboardFd) {
        keyboardFd = syscall_open("/dev/kb0", 0);
    }

    return syscall_read(keyboardFd, (void *)data,
                        count * sizeof(KeyboardData));
}

Device::Device(const char *devPath, size_t eventDataLen) {
    this->eventDataLen = eventDataLen;

    // Allocate device read buffer
    event = new TellurEvent();
    event->data = new uint8_t[eventDataLen];

    prevEvent = new TellurEvent();
    prevEvent->data = new uint8_t[eventDataLen];
    
    // Set device path name
    size_t devPathLen = __strlen(devPath);
    this->devPath = new char[devPathLen + 1];
    __memcpy(this->devPath, devPath, devPathLen);
    this->devPath[devPathLen] = '\0';

    devFd = syscall_open(this->devPath, 0);
}

Device::Device()
    : event(nullptr), eventDataLen(0), devPath(nullptr), devFd(-1) {}

Device::~Device() {
    delete event;
    delete devPath;
    
    // syscall_close(devFd);
}

TellurEvent *Device::devicePoll() {
    if (syscall_read(devFd, event->data, eventDataLen)) {
        event->type = getEventType();
        
        TellurEvent *temp = prevEvent;
        prevEvent = event;
        event = temp;

        return event;
    }

    return nullptr;
}

DeviceMousePs2::DeviceMousePs2(const char *devPath)
    : Device(devPath, sizeof(MouseData)) {
    // Initial state of mouse = nothing moved or clicked
    MouseData *prevMouseData = (MouseData *)prevEvent->data;
    MouseData *mouseData = (MouseData *)event->data;

    *prevMouseData = {
        .flags = 0,
        .delta_x = 0,
        .delta_y = 0
    };

    *mouseData = *prevMouseData;
}

DeviceMousePs2::~DeviceMousePs2() {}

TellurEventType DeviceMousePs2::getEventType() {
    MouseData *newData = (MouseData *)event->data;
    MouseData *prevData = (MouseData *)prevEvent->data;

    if (newData->lButton() && prevData->lButton()) {
        return TellurEventType::MouseMoveClick;
    } else if (newData->lButton() && !prevData->lButton()) {
        return TellurEventType::MouseLeftClick;
    } else if (!newData->lButton() && prevData->lButton()) {
        return TellurEventType::MouseLeftRelease;
    } else { // if (!newData->mButton() && !prevData->lButton()) {
        return TellurEventType::MouseMove;
    }
}

DeviceKeyboardPs2::DeviceKeyboardPs2(const char *devPath) : Device(devPath, sizeof(KeyboardData)) {}

DeviceKeyboardPs2::~DeviceKeyboardPs2() {}

TellurEventType DeviceKeyboardPs2::getEventType() {

    return TellurEventType::KeyboardDefault;
}

DeviceManager::DeviceManager() : numDevices(0), maxDevices(MAX_DEVICES) {
    devices = new Device*[maxDevices];
    for (size_t i = 0; i < maxDevices; i++) {
        devices[i] = nullptr;
    }
}

DeviceManager::~DeviceManager() {
    delete [] devices;
}

bool DeviceManager::addDevice(Device *device) {
    if (!device || numDevices == maxDevices) {
        return false;
    }

    devices[numDevices++] = device;

    return true;
}

TellurEvent *DeviceManager::pollDevices() {
    for (size_t i = 0; i < numDevices; i++) {
        TellurEvent *event = devices[i]->devicePoll();
        if (event) {
            return event;
        }
    }
    return nullptr;
}

}