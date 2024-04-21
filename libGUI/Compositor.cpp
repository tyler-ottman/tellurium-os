#include "Compositor.hpp"

namespace GUI {

Compositor::Compositor(FbInfo *fbInfo) : fbInfo(fbInfo) {
    screen = new Rect(0, 0, fbInfo->fb_width, fbInfo->fb_height);
    
    renderRegion = new ClippingManager();
}

Compositor::~Compositor() {}

void Compositor::drawBuff(Rect &rect, uint32_t *buff) {
    Rect *renderRects = renderRegion->getClippedRegions();
    int numRenderRects = renderRegion->getNumClipped();

    for (int i = 0; i < numRenderRects; i++) {
        drawClippedBuff(rect, buff, &renderRects[i]);
    }
}

void Compositor::drawBuffRaw(Rect &rect, uint32_t *bitmap) {
    // Not clipped in the sense that the entire client window is the restricted area
    drawClippedBuff(rect, bitmap, screen);
}

void Compositor::drawClippedBuff(Rect &rect, uint32_t *buff, Rect *area) {
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

// The Window structure is sorted by priority, so the surfaces can be drawn by
// depth, starting with the shallowest depth (Z-Depth Buffering). Once a surface
// at a given depth is drawn, any surface at a deeper depth will be occluded.
// TODO: If a surface is transparent, the surfaces behind it will be blended
void Compositor::drawWindow(Window *win) {
    for (int i = win->numWindows - 1; i >= 0; i--) {
        drawWindow(win->windows[i]);
    }

    // Draw self
    drawBuff(*win->winRect, win->winBuff);

    renderRegion->removeClippedRegion(win->winRect);
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
                renderRegion->addClippedRegion(dirtyAncestor->getWinRect());
                renderRegion->addClippedRegion(dirtyAncestor->getPrevRect());
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
        renderRegion->addClippedRegion(mouse->getPrevRect());

        // New mouse position
        renderRegion->addClippedRegion(mouse->getWinRect());

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