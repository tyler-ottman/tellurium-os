#include "Compositor.hpp"

namespace GUI {

Compositor::Compositor(FbInfo *fbInfo) : fbInfo(fbInfo) {
    screen = new Rect(0, 0, fbInfo->fb_width, fbInfo->fb_height);
    
    renderRegion = new ClipRegions();
}

Compositor::~Compositor() {}

void Compositor::drawBuffRaw(Rect &rect, uint32_t *bitmap) {
    // Not clipped in the sense that the entire client window is the restricted area
    blit(rect, bitmap, screen);
}

void Compositor::blit(Rect &rect, uint32_t *buff, Rect *area) {
    Rect drawRect(rect);
    
    int translateX = drawRect.getX();
    int translateY = drawRect.getY();

    area->restrictRect(&drawRect);
    screen->restrictRect(&drawRect);

    int x = drawRect.getX();
    int y = drawRect.getY();
    int width = rect.getWidth();
    int xMax = drawRect.getRight();
    int yMax = drawRect.getBottom();

    int screenWidth = fbInfo->fb_width;
    uint32_t *screenBuff = (uint32_t *)fbInfo->fb_buff;
    for (int i = y; i < yMax; i++) {
        for (int j = x; j < xMax; j++) {
            screenBuff[i * screenWidth + j] = buff[(i - translateY) * width + (j - translateX)];
        }
    }
}

// https://en.wikipedia.org/wiki/Alpha_compositing for equation
// The original equation represents the pixel's channels as a decimal, 0.0 - 1.0
// Converting an input channel from a decimal to an 8-bit value, we have
// alphaB' = alphaB * (256 - alphaA) / 256 (no overflow if alphaB' is 8-bit value)
// alphaOut = alphaA + alphaB' (no overflow if alphaOut is 8-bit value)
// pixelOut = (colorA * alphaA + colorB * alphaB) / alphaOut
uint32_t alphaBlendPixel(uint32_t colorA, uint32_t colorB) {
    uint8_t alphaA = colorA >> 24; // Pixel A's alpha channel
    uint8_t alphaB = colorB >> 24; // Pixel B's alpha channel

    // Optimization: Do not blend if front pixel is fully opaque
    if (alphaA == 0xff) {
        return colorA;
    }
    
    // Optimization: Do not blend if front pixel is fully transparent
    else if (alphaA == 0x00)  {
        return colorB;
    }

    // The adjusted alpha channel component of Pixel B
    uint8_t alphaB_ = (alphaB * (256 - alphaA)) >> 8;

    // The output alpha channel of blended pixel
    uint8_t alphaOut = alphaA + alphaB_;

    // The output color channels of blended pixel
    uint8_t redOut = (((colorA >> 16) & 0xff) * alphaA + ((colorB >> 16) & 0xff) * alphaB_) / alphaOut;
    uint8_t greenOut = (((colorA >> 8) & 0xff) * alphaA + ((colorB >> 8) & 0xff) * alphaB_) / alphaOut;
    uint8_t blueOut = ((colorA & 0xff) * alphaA + (colorB & 0xff) * alphaB_) / alphaOut;

    return (alphaOut << 24) | (redOut << 16) | (greenOut << 8) | blueOut;
}

void Compositor::alphaBlit(Rect &rect, uint32_t *buff, Rect *area) {
    Rect drawRect(rect);
    
    int translateX = drawRect.getX();
    int translateY = drawRect.getY();

    area->restrictRect(&drawRect);
    screen->restrictRect(&drawRect);

    int x = drawRect.getX();
    int y = drawRect.getY();
    int width = rect.getWidth();
    int xMax = drawRect.getRight();
    int yMax = drawRect.getBottom();

    int screenWidth = fbInfo->fb_width;
    uint32_t *screenBuff = (uint32_t *)fbInfo->fb_buff;
    for (int i = y; i < yMax; i++) {
        for (int j = x; j < xMax; j++) {
            screenBuff[i * screenWidth + j] = alphaBlendPixel(
                screenBuff[i * screenWidth + j],
                buff[(i - translateY) * width + (j - translateX)]);
        }
    }
}

// The Window structure is sorted by priority, so the surfaces can be drawn by
// depth, starting with the shallowest depth (Z-Depth Buffering). Once a surface
// at a given depth is drawn, any surface at a deeper depth will be occluded.
void Compositor::drawWindow(Window *win) {
    for (int i = win->numWindows - 1; i >= 0; i--) {
        drawWindow(win->windows[i]);
    }

    // Draw self (TODO: put in surface)
    Rect &rect = *win->winRect;
    uint32_t *buff = win->winBuff;
    for (int i = 0; i < renderRegion->numClipped; i++) {
        ClipRect *clippedRect = &renderRegion->clippedRects[i];

        // The clipped region is marked as transparent, perform alpha blit
        if (clippedRect->win->isTransparent()) {
            alphaBlit(rect, buff, &clippedRect->rect);
        }

        // The clipped region is not marked as transparent, perform normal blit
        else {
            blit(rect, buff, &clippedRect->rect);
        }
    }

    // The Window just drawn is transparent, existing parts of clipped
    // region must be marked as transparent to indicate to the Compositor
    // that the surface must be alpha blended with the existing surface
    if (win->isTransparent()) {
        renderRegion->divideClippedRegion(win->winRect, win);
    }

    // If the Window just drawn was opaque, existing parts of the clipped
    // region must be deleted to indicate to the Compositor that surfaces
    // beneath the Window will be occluded
    else {
        renderRegion->removeClippedRegion(win->winRect);
    }
}

void Compositor::createDirtyWindowRegions(Window *win, Window *dirtyAncestor) {
    // If window is marked as dirty, add it to dirty list
    // for the compositor
    if (win->isDirty()) {
        // Reset dirty flag because it won't be dirty after refresh
        win->setDirty(false);

        if (win->hasUnbounded()) {
            // TODO
        } else {
            // This is an optimization, dirty child Windows bounded by a dirty
            // ancestor (parent or above) Window do not need to be added to 
            // the dirty list for the compositor, because the ancestor already
            // covers the region
            if (!dirtyAncestor) {
                dirtyAncestor = win;

                // Add new and old location of window to dirty list
                renderRegion->addClippedRegion(dirtyAncestor->getWinRect(), win);
                renderRegion->addClippedRegion(dirtyAncestor->getPrevRect(), win);
            }
        }

        // Now update previous Rect to be the current Rent (formaly dirty)
        win->updatePrevRect();
    }

    // Process dirty Windows for children
    for (int i = 0; i < win->getNumChildren(); i++) {
        createDirtyWindowRegions(win->windows[i], dirtyAncestor);
    }
}

void Compositor::createDirtyMouseRegion(Window *mouse) {
    if (mouse->isDirty()) {
        // Reset dirty flag because it won't be dirty after refresh
        mouse->setDirty(false);

        // Original mouse position
        renderRegion->addClippedRegion(mouse->getPrevRect(), mouse);

        // New mouse position
        renderRegion->addClippedRegion(mouse->getWinRect(), mouse);

        // Now store current position of mouse in previous position
        mouse->updatePrevRect();
    }
}

void Compositor::render(Window *root, Window *mouse) {
    // Add any Window state change to list of regions to re-render
    // for the compositor
    createDirtyWindowRegions(root, nullptr);

    // If mouse position has changes since last refresh, patch old position
    // and draw new position
    createDirtyMouseRegion(mouse);

    // Only refresh if dirty regions generated
    if (renderRegion->getNumClipped()) {
        // Draw the windows that intersect the dirty clipped regions
        drawWindow(root);

        // Draw mouse on top of final image, does not use clipped regions
        drawBuffRaw(*mouse->winRect, mouse->winBuff);

        // Rendering done, reset dirty/render regions list
        renderRegion->resetClippedList();
    }
}

void ClipRegions::addClippedRegion(Rect *rect, Window *win) {
    // First, punch a hole in the clipped list
    removeClippedRegion(rect);

    // Second, now just add the Rect
    ClipRect clipRect(*rect, win);
    pushRect(&clipRect);
}

ClipRegions *overlapping = nullptr; // TODO: optimize

void ClipRegions::divideClippedRegion(Rect *rect, Window *win) {
    if (!overlapping) { overlapping = new ClipRegions(); }
    overlapping->resetClippedList();

    Rect splitRects[4];

    for (int i = 0; i < numClipped; i++) {
        ClipRect *clip = &clippedRects[i];

        if (clip->rect.overlaps(rect)) {
            ClipRect overlap(win);
            rect->getOverlapRect(&overlap.rect, &clip->rect);

            int count = clip->rect.getSplitRects(splitRects, rect);

            // Add the split rects to clip list
            for (int j = 0; j < count; j++) {
                ClipRect split(splitRects[j], clip->win);
                pushRect(&split);
            }

            // Update and split selected clipped region
            deleteRect(i);

            // Now add the overlapped rect to list of overlapping rects
            // pushRect(&overlap);
            overlapping->pushRect(&overlap);

            i = -1;  // Resets to 0 next iteration
        }
    }

    for (int i = 0; i < overlapping->numClipped; i++) {
        pushRect(&overlapping->clippedRects[i]);
    }
}

void ClipRegions::removeClippedRegion(Rect *rect) {
    Rect splitRects[4];

    for (int i = 0; i < numClipped; i++) {
        ClipRect *clip = &clippedRects[i];

        if (clip->rect.overlaps(rect)) {
            int count = clip->rect.getSplitRects(splitRects, rect);

            // Add the split rects to clip list
            for (int j = 0; j < count; j++) {
                ClipRect split(splitRects[j], clip->win);
                pushRect(&split);
            }

            // Update and split selected clipped region
            deleteRect(i);

            i = -1;  // Resets to 0 next iteration
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

};