#include "Border.hpp"
#include "libGUI/FbContext.hpp"
namespace GUI {

Border::Border(int x, int y, int width, int height, WindowFlags flags,
               WindowPriority priority)
    : Window::Window("border", x, y, width, height, flags, priority) {
    // color = 0xffbebebe;
    setColor(0xffbebebe);
}

Border::~Border() {}

void Border::drawObject() {
    FbContext *context = FbContext::getInstance();
    context->drawBuff(*winRect, winBuff);
}

};