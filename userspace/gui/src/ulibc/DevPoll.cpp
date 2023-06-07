#include "DevPoll.hpp"

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

}