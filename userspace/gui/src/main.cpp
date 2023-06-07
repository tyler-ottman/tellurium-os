#include "Windowing/FbContext.hpp"
#include "Windowing/Window.hpp"
#include <stddef.h>
#include <stdint.h>
#include "ulibc/mem.hpp"
#include "ulibc/syscalls.hpp"

int main() {
    GUI::Window *win1 = new GUI::Window("test", 300, 300, 50, 50);
    win1->windowPaint(0x00f893a4);

    for (;;) {}
}