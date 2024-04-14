#include "FbContext.hpp"
#include "libTellur/mem.hpp"
#include "libTellur/syscalls.hpp"

namespace GUI {

FbContext *FbContext::instance = nullptr;

FbContext *FbContext::getInstance() {
    if (!instance) {
        instance = new FbContext();
    }

    return instance;
}

FbContext::FbContext() { syscall_get_fb_context((void *)&fbInfo); }

FbContext::~FbContext() {}

Compositor::Compositor(FbContext *context) : context(context) {
    screen = new Rect(0, 0, context->fbInfo.fb_width, context->fbInfo.fb_height);
    
    renderRegion = new ClippingManager();
    dirtyRegion = new ClippingManager();
}

Compositor::~Compositor() {
    
}

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

    int screenWidth = context->fbInfo.fb_width;
    uint32_t *screenBuff = (uint32_t *)context->fbInfo.fb_buff;
    for (int i = y; i <= yMax; i++) {
        for (int j = x; j <= xMax; j++) {
            screenBuff[i * screenWidth + j] = buff[(i - translateY) * width + (j - translateX)];
        }
    }
}

void Compositor::applyBoundClipping(Window *win) {
    Window *parent = win->parent;

    if (!parent) {
        int nDirtyRegions = dirtyRegion->getNumClipped();
        if (nDirtyRegions) {
            Rect *dirtyRegions = dirtyRegion->getClippedRegions();

            for (int i = 0; i < nDirtyRegions; i++) {
                renderRegion->addClippedRect(&dirtyRegions[i]);
            }

            renderRegion->intersectClippedRect(win->winRect);
        } else {
            renderRegion->addClippedRect(win->winRect);
        }

        return;
    }

    // Reduce drawing to parent window
    applyBoundClipping(parent);

    // Reduce visibility to main drawing area
    renderRegion->intersectClippedRect(win->winRect);

    // Occlude areas of window where siblings overlap on top
    for (int i = win->windowID + 1; i < parent->numWindows; i++) {
        Window *aboveWin = parent->windows[i];
        if (win->intersects(aboveWin)) {
            renderRegion->reshapeRegion(aboveWin->winRect);
        }
    }
}

bool Compositor::rectIntersectsDirty(Rect *rect) {
    Rect *dirtyRegions = dirtyRegion->getClippedRegions();
    int nDirtyRegions = dirtyRegion->getNumClipped();
    for (int j = 0; j < nDirtyRegions; j++) {
        if (rect->intersects(&dirtyRegions[j])) {
            return true;
        }
    }

    return false;
}

void Compositor::drawWindow(Window *win) {
    applyBoundClipping(win);

    // Remove child window clipped rectangles
    for (int i = 0; i < win->numWindows; i++) {
        Window *child = win->windows[i];
        renderRegion->reshapeRegion(child->winRect);
    }

    // Draw self
    drawBuff(*win->getWinRect(), win->getWinBuff());

    // Redraw children if they intersect with dirty region
    for (int i = 0; i < win->numWindows; i++) {
        renderRegion->resetClippedList();
        Window *child = win->windows[i];
        if (rectIntersectsDirty(child->winRect)) {
            drawWindow(child);
        }        
    }
}

/// @brief Recursively traverse the Window tree and add dirty Windows
/// to list of regions that require re-rendering to screen/buffer
/// @param win the current Window being traversing over
/// @param dirtyAncestor stores a pointer to the highest ancestor Window in
/// the tree that is dirty in the current branch being traversed, starts as
/// null because we assume there is nothing dirty
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
                dirtyRegion->addClippedRect(dirtyAncestor->getWinRect());
                dirtyRegion->addClippedRect(dirtyAncestor->getPrevRect());
            }
        }

        // Now update previous Rect to be the current Rent (formaly dirty)
        win->updatePrevRect();
    }

    // Process dirty Windows for children
    for (int i = 0; i < win->getNumChildren(); i++) {
        createDirtyWindowRegions(win->getChild(i), dirtyAncestor);
    }
}

void Compositor::createDirtyMouseRegion(Window *mouse) {
    if (mouse->isDirty()) {
        // Reset dirty flag because it won't be dirty after refresh
        mouse->setDirty(false);

        // Original mouse position
        dirtyRegion->addClippedRect(mouse->getPrevRect());

        // New mouse position
        dirtyRegion->addClippedRect(mouse->getWinRect());

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
    if (dirtyRegion->getNumClipped()) {
        // Draw the windows that intersect the dirty clipped regions
        drawWindow(root);

        // Draw mouse on top of final image, does not use clipped regions
        drawBuffRaw(*mouse->getWinRect(), mouse->getWinBuff());

        // Rendering done, reset dirty/render regions list
        dirtyRegion->resetClippedList();
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

}