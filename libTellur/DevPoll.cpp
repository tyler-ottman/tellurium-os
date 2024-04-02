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
    // Allocate device read buffer
    event = new TellurEvent();
    this->eventDataLen = eventDataLen;
    event->data = new uint8_t[eventDataLen];
    
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
        return event;
    }

    return nullptr;
}

DeviceMousePs2::DeviceMousePs2(const char *devPath) : Device(devPath, sizeof(MouseData)) {}

DeviceMousePs2::~DeviceMousePs2() {}

TellurEventType DeviceMousePs2::getEventType() {
    return TellurEventType::MouseDefault;
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