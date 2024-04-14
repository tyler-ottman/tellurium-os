#include "FbContext.hpp"
#include "libTellur/mem.hpp"
#include "libTellur/syscalls.hpp"

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

    instance->fb_buff = (uint32_t *)instance->fb_meta.fb_buff;
    instance->screen =
        new Rect(0, 0, instance->fb_meta.fb_width, instance->fb_meta.fb_height);

    instance->renderRegion = new ClippingManager();
    instance->dirtyRegion = new ClippingManager();

    return instance;
}

FbMeta *FbContext::getFbContext() {
    return &instance->fb_meta;
}

// TODO: Deprecate with Surfaces implementation
void FbContext::drawRect(Rect &rect, uint32_t color) {
    Rect *renderRects = renderRegion->getClippedRegions();
    int numRenderRects = renderRegion->getNumClipped();

    for (int i = 0; i < numRenderRects; i++) {
        drawClippedRect(rect, color, &renderRects[i]);
    }
}

void FbContext::drawBuff(Rect &rect, uint32_t *buff) {
    Rect *renderRects = renderRegion->getClippedRegions();
    int numRenderRects = renderRegion->getNumClipped();

    for (int i = 0; i < numRenderRects; i++) {
        drawClippedBuff(rect, buff, &renderRects[i]);
    }
}

void FbContext::drawRectNoRegion(Rect &rect, uint32_t color) {
    drawClippedRect(rect, color, screen);
}

void FbContext::drawBitmapNoRegion(Rect &rect, uint32_t *bitmap) {
    drawClippedBitmap(rect, bitmap, screen);
}

void FbContext::drawClippedRect(Rect &rect, uint32_t color, Rect *area) {
    Rect drawRect(rect);

    area->restrictRect(&drawRect);
    screen->restrictRect(&drawRect);

    for (int i = drawRect.getY(); i <= drawRect.getBottom(); i++) {
        for (int j = drawRect.getX(); j <= drawRect.getRight(); j++) {
            fb_buff[i * fb_meta.fb_width + j] = color;
        }
    }
}

void FbContext::drawClippedBuff(Rect &rect, uint32_t *buff, Rect *area) {
    Rect drawRect(rect);
    
    int translateX = drawRect.getX();
    int translateY = drawRect.getY();

    area->restrictRect(&drawRect);
    screen->restrictRect(&drawRect);

    for (int i = drawRect.getY(); i <= drawRect.getBottom(); i++) {
        for (int j = drawRect.getX(); j <= drawRect.getRight(); j++) {
            fb_buff[i * fb_meta.fb_width + j] = buff[(i - translateY) * rect.getWidth() + (j - translateX)];
        }
    }
}

void FbContext::drawClippedBitmap(Rect &rect, uint32_t *bitmap, Rect *area) {
    Rect drawRect(rect);
    
    int translateX = drawRect.getX();
    int translateY = drawRect.getY();

    area->restrictRect(&drawRect);
    screen->restrictRect(&drawRect);

    for (int i = drawRect.getY(); i <= drawRect.getBottom(); i++) {
        for (int j = drawRect.getX(); j <= drawRect.getRight(); j++) {
            if (bitmap[(i - translateY) * rect.getWidth() + (j - translateX)] & 0xff000000) {
                fb_buff[i * fb_meta.fb_width + j] = bitmap[(i - translateY) * rect.getWidth() + (j - translateX)];
            }
        }
    }
}

// void FbContext::drawClippedRegions() {
//     int pixels = fb_meta.fb_height * fb_meta.fb_width;
//     uint32_t *pixelBuff = (uint32_t *)fb_meta.fb_buff;

//     for (int i = 0; i < pixels; i++) {
//         pixelBuff[i] = 0;
//     }

//     for (int i = 0; i < numClipped; i++) {
//         Rect *rect = &clippedRects[i];

//         drawOutlinedRect(rect->getLeft(), rect->getTop(),
//                          rect->getRight() - rect->getLeft() + 1,
//                          rect->getBottom() - rect->getTop() + 1, 0xed872d);
//     }
// }

}