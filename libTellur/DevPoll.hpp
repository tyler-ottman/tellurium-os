#ifndef DEVPOLL_H
#define DEVPOLL_H

#include <stddef.h>
#include <stdint.h>
#include "syscalls.hpp"

#define MAX_DEVICES 10

namespace Device {

struct MouseData {
    bool lButton(void) { return flags & 0x1; }
    bool rButton(void) { return flags & 0x2; }
    bool mButton(void) { return flags & 0x4; }
    uint8_t flags;
    int delta_x;
    int delta_y;
};

struct KeyboardData {
    uint8_t data;
};

int mousePoll(MouseData *data);
int keyboardPoll(KeyboardData *buff, size_t count);

/// @brief Common window events
enum TellurEventType {
    MouseMove,
    MouseMoveClick,
    MouseLeftClick,
    MouseLeftRelease,
    MouseDefault,
    KeyboardDefault,
    Default
};

struct TellurEvent {
    TellurEvent(TellurEventType type, void *data) : type(type), data(data) {}
    TellurEvent() : type(TellurEventType::Default), data(nullptr) {}
    bool isMosueMove() { return type == MouseMove || type == MouseMoveClick; }
    bool isMouseEvent(void) { return type <= MouseDefault; }
    bool isKeyboardEvent(void) { return type > MouseDefault; }
    TellurEventType type;
    void *data;
};

class Device {
public:
    Device(const char *devPath, size_t eventDataLen);
    Device(void);
    virtual ~Device(void);

    /// @brief Poll device for event
    /// @return If there's an event, return it. Otherwise, return nullptr
    virtual TellurEvent *devicePoll(void);

    /// @brief Get the type of event received from device
    /// @return The event type
    virtual TellurEventType getEventType(void) = 0;

protected:
    TellurEvent *event; /// @brief Where a new event is stored
    TellurEvent *prevEvent; /// @brief Previous device event
    size_t eventDataLen; /// @brief The length of the device data returned
    char *devPath; /// @brief The device path in the VFS
    int devFd; /// @brief The device's file descriptor
};

class DeviceMousePs2 : public Device {
public:
    /// @brief PS/2 Mouse Device Constructor
    /// @param devPath Path to PS/2 mouse device
    DeviceMousePs2(const char *devPath);

    /// @brief The destructor
    ~DeviceMousePs2(void);

    /// @brief Given data received, get type of mouse event
    /// @return The type of event
    TellurEventType getEventType(void) override;
};

class DeviceKeyboardPs2 : public Device {
public:
    /// @brief PS/2 Keyboard Device Constructor
    /// @param devPath Path to PS/2 mouse device
    DeviceKeyboardPs2(const char *devPath);

    /// @brief The destructor
    ~DeviceKeyboardPs2(void);

    /// @brief Given data received, get type of mouse event
    /// @return The type of event
    TellurEventType getEventType(void) override;
};

class DeviceManager {
public:
    /// @brief The constructor
    DeviceManager(void);

    /// @brief The constructor
    ~DeviceManager(void);

    /// @brief Add a device to the manager
    /// @param device The device to add
    /// @return True if device successfully added, false otherwise
    bool addDevice(Device *device);

    /// @brief Poll all devices attached to manager
    /// @return Return an event if one is found, nullptr otherwise
    TellurEvent *pollDevices(void);

private:
    Device **devices; /// @brief Attached devices, TODO: ring buffer
    size_t numDevices; /// @brief Number of attached devices
    size_t maxDevices; /// @brief Attached device capacity
};

}

#endif // DEVPOLL_H