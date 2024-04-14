#include "Border.hpp"

namespace GUI {

Border::Border(int x, int y, int width, int height, WindowFlags flags,
               WindowPriority priority)
    : Window::Window("border", x, y, width, height, flags, priority) {
    loadBuff(0xffbebebe);
}

Border::~Border() {}

};