#include "libGUI/Desktop.hpp"
#include "libGUI/Image.hpp"
#include "libGUI/Taskbar.hpp"

namespace GUI {

Desktop::Desktop()
    : Window("desktop", 0, 0,
             FbContext::getInstance()->getFbContext()->fb_width,
             FbContext::getInstance()->getFbContext()->fb_height,
             WIN_DECORATE), forceRefresh(true) {
    mouseX = width / 2;
    mouseY = height / 2;
    oldMouseX = mouseX;
    oldMouseY = mouseY;

    color = 0xff2a2a2a;

    Image *background = new Image(0, 0, context->getFbContext()->fb_width,
                                  context->getFbContext()->fb_height);
    
    background->loadImage("/tmp/background.ppm");
    appendWindow(background);

    Taskbar *taskbar = new Taskbar(0, height - 40, width, 40);
    taskbar->setColor(0xffbebebe);
    appendWindow(taskbar);
}

Desktop::~Desktop() {

}

void Desktop::drawWindow() {
    if (forceRefresh) {
        Window::drawWindow();
        forceRefresh = false;
    }

    // If window was dragged since last refresh, update it
    applyDirtyDrag();
    applyDirtyMouse();

    Window::drawWindow();

    // Draw mouse
    context->drawRectNoRegion(mouseX, mouseY, 10, 10, 0xffffffff);
}

void Desktop::onMouseEvent(Device::MouseData *data) {
    context->resetClippedList();
    context->resetDirtyList();

    Window::onMouseEvent(data, mouseX, mouseY);

    // Regions that need immediate refresh
    if (context->getNumDirty()) {
        applyDirtyMouse();

        Window::drawWindow();

        context->drawRectNoRegion(mouseX, mouseY, 10, 10, 0xffffffff);
        context->resetDirtyList();
    }

    updateMousePos(data);

    lastMouseState = data->flags;
}

void Desktop::applyDirtyMouse(void) {
    // If mouse position changed since last refresh
    if (!(oldMouseX == mouseX && oldMouseY == mouseY)) {
        // Original mouse position
        Rect oldMouse(oldMouseY, oldMouseY + 10 - 1, oldMouseX, oldMouseX + 10 - 1);
        context->addClippedRect(&oldMouse);

        // New mouse position
        Rect newMouse(mouseY, mouseY + 10 - 1, mouseX, mouseX + 10 - 1);
        context->addClippedRect(&newMouse);

        oldMouseX = mouseX;
        oldMouseY = mouseY;

        context->moveClippedToDirty();
    }
}

void Desktop::updateMousePos(Device::MouseData *data) {
    int xNew = mouseX + data->delta_x;
    int yNew = mouseY - data->delta_y;

    FbMeta *meta = FbContext::getInstance()->getFbContext();
    if (xNew >= 0 && xNew < (int)meta->fb_width) {
        mouseX = xNew;
    }

    if (yNew >= 0 && yNew < (int)meta->fb_height) {
        mouseY = yNew;
    }
}

bool Desktop::mouseInBounds(Window *window) {
    return ((mouseX >= window->getXPos()) &&
            (mouseX <= (window->getXPos() + window->getWidth())) &&
            (mouseY >= window->getYPos()) &&
            (mouseY <= (window->getYPos() + window->getHeight())));
}
}

