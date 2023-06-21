#include "FbContext.hpp"
#include "ulibc/mem.hpp"
#include "ulibc/syscalls.hpp"

namespace GUI {

FbContext *FbContext::instance = nullptr;

FbContext::FbContext() {

}

FbContext::~FbContext() {
    
}

FbContext *FbContext::getInstance() {
    if (instance) {
        return instance;
    }
    
    instance = new FbContext();

    syscall_get_fb_context(&instance->fb_meta);

    instance->screen = new Rect(0, instance->fb_meta.fb_height - 1, 0,
                                instance->fb_meta.fb_width - 1);

    instance->numRegions = 0;
    instance->clippedRects = new Rect[CLIPPED_MAX];

    return instance;
}

FbMeta *FbContext::getFbContext() {
    return &instance->fb_meta;
}

void FbContext::drawClippedRect(int x, int y, int width, int height,
                                uint32_t color, Rect *area) {
    int xMax = x + width;
    int yMax = y + height;

    FbMeta &fb_meta = instance->fb_meta;
    uint32_t *buff = (uint32_t *)fb_meta.fb_buff;

    if (y < area->getTop()) {
        y = area->getTop();
    }

    if (yMax > area->getBottom() + 1) {
        yMax = area->getBottom() + 1;
    }

    if (x < area->getLeft()) {
        x = area->getLeft();
    }

    if (xMax > area->getRight() + 1) {
        xMax = area->getRight() + 1;
    }

    if (area->getLeft() < 0) {
        x = 0;
    }

    if (area->getRight() > screen->getRight()) {
        xMax = screen->getRight();
    }

    if (area->getTop() < 0) {
        y = 0;
    }

    if (area->getBottom() > screen->getBottom()) {
        yMax = screen->getBottom();
    }
    
    for (int i = y; i < yMax; i++) {
        for (int j = x; j < xMax; j++) {
            buff[i * fb_meta.fb_width + j] = color;
        }
    }
}

void FbContext::drawRect(int x, int y, int width, int height, uint32_t color) {
    if (numRegions) {
        for (int i = 0; i < numRegions; i++) {
            drawClippedRect(x, y, width, height, color, &clippedRects[i]);
        }
    } else {
        drawClippedRect(x, y, width, height, color, screen);
    }
}

void FbContext::drawVerticalLine(int xPos, int yPos, int length, int color) {
    drawClippedRect(xPos, yPos, 1, length, color, screen);
}

void FbContext::drawHorizontalLine(int xPos, int yPos, int length, int color) {
    drawClippedRect(xPos, yPos, length, 1, color, screen);
}

void FbContext::drawOutlinedRect(int xPos, int yPos, int width, int height,
                                 int color) {
    drawHorizontalLine(xPos, yPos, width, color);
    drawHorizontalLine(xPos, yPos + height - 1, width, color);
    drawVerticalLine(xPos, yPos + 1, height - 2, color);
    drawVerticalLine(xPos + width - 1, yPos + 1, height - 2, color);
}

void FbContext::addClippedRect(Rect *rect) {
    reshapeRegion(rect);

    appendClippedRect(rect);
}

void FbContext::reshapeRegion(Rect *rect) {
    for (int i = 0; i < numRegions; i++) {
        Rect *clipped = &clippedRects[i];

        if (!clipped->intersects(rect)) {
            continue;
        }

        Rect splitRects[4];
        int count = clipped->split(splitRects, rect);

        // Update and split selected clipped region
        removeClippedRect(i);

        for (int j = 0; j < count; j++) {
            appendClippedRect(&splitRects[j]);
        }

        i = -1;
    }
}

void FbContext::appendClippedRect(Rect *rect) {
    if (numRegions >= CLIPPED_MAX) {
        return;
    }

    clippedRects[numRegions++] = *rect;
}

void FbContext::removeClippedRect(int index) {
    if (index < 0 || index >= numRegions) {
        return;
    }

    for (int i = index; i < numRegions - 1; i++) {
        clippedRects[i] = clippedRects[i + 1];
    }

    numRegions--;
}

void FbContext::resetClippedList() {
    numRegions = 0;
}

void FbContext::drawClippedRegions() {
    int pixels = fb_meta.fb_height * fb_meta.fb_width;
    uint32_t *pixelBuff = (uint32_t *)fb_meta.fb_buff;

    for (int i = 0; i < pixels; i++) {
        pixelBuff[i] = 0;
    }

    for (int i = 0; i < numRegions; i++) {
        Rect *rect = &clippedRects[i];

        drawOutlinedRect(rect->getLeft(), rect->getTop(),
                         rect->getRight() - rect->getLeft() + 1,
                         rect->getBottom() - rect->getTop() + 1, 0xed872d);
    }
}

void FbContext::intersectClippedRect(Rect *rect) {
    // resetClippedList();

    for (int i = 0; i < numRegions; i++) {
        Rect *currentRect = &clippedRects[i];
        Rect intersectRect;

        bool ret = currentRect->intersect(&intersectRect, rect);
        if (ret) {
            appendClippedRect(&intersectRect);
        }
    }
}

}