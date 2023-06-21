#pragma once

#include <stddef.h>
#include <stdint.h>
#include "syscalls.hpp"

namespace Device {

struct MouseData {
    uint8_t flags;
    int delta_x;
    int delta_y;
};

struct KeyboardData {
    uint8_t data;
};

int mousePoll(MouseData *data);
int keyboardPoll(KeyboardData *buff, size_t count);

}