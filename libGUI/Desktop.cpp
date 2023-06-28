#include "libGUI/Desktop.hpp"

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
}

Desktop::~Desktop() {

}

void Desktop::drawWindow() {
    context->resetClippedList();
    context->resetDirtyList();

    if (selectedWindow) { // Generate dirty region
        int tempX = selectedWindow->getXPos();
        int tempY = selectedWindow->getYPos();

        selectedWindow->updatePosition(xOld, yOld);
        context->addClippedRect(selectedWindow);
        selectedWindow->updatePosition(tempX, tempY);
        context->addClippedRect(selectedWindow);

        context->moveClippedToDirty();

        xOld = selectedWindow->getXPos();
        yOld = selectedWindow->getYPos();
    } else { // Simple mouse movement
        if (forceRefresh) {
            forceRefresh = false;
        } else {
            Rect oldMouse(oldMouseY, oldMouseY + 10 - 1, oldMouseX,
                      oldMouseX + 10 - 1);
            Rect newMouse(mouseY, mouseY + 10 - 1, mouseX, mouseX + 10 - 1);
            
            context->addClippedRect(&oldMouse);
            context->addClippedRect(&newMouse);
            context->moveClippedToDirty();

            oldMouseX = mouseX;
            oldMouseY = mouseY;
        }
        
    }

    Window::drawWindow();

    // Draw mouse
    context->drawRectNoRegion(mouseX, mouseY, 10, 10, 0xffffffff);
}

void Desktop::onMouseEvent(Device::MouseData *data) {
    Window::onMouseEvent(data, mouseX, mouseY);

    updateMousePos(data);

    lastMouseState = data->flags;
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