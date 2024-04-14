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

    syscall_get_fb_context((void *)&instance->fb_meta);

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

void FbContext::drawBuff(Rect &rect, uint32_t *buff) {
    Rect *renderRects = renderRegion->getClippedRegions();
    int numRenderRects = renderRegion->getNumClipped();

    for (int i = 0; i < numRenderRects; i++) {
        drawClippedBuff(rect, buff, &renderRects[i]);
    }
}

void FbContext::drawBuffRaw(Rect &rect, uint32_t *bitmap) {
    // Not clipped in the sense that the entire screen is the restricted area
    drawClippedBuff(rect, bitmap, screen);
}

void FbContext::drawClippedBuff(Rect &rect, uint32_t *buff, Rect *area) {
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

    for (int i = y; i <= yMax; i++) {
        for (int j = x; j <= xMax; j++) {
            fb_buff[i * fb_meta.fb_width + j] = buff[(i - translateY) * width + (j - translateX)];
        }
    }
}

void FbContext::applyBoundClipping(Window *win) {
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

bool FbContext::rectIntersectsDirty(Rect *rect) {
    Rect *dirtyRegions = dirtyRegion->getClippedRegions();
    int nDirtyRegions = dirtyRegion->getNumClipped();
    for (int j = 0; j < nDirtyRegions; j++) {
        if (rect->intersects(&dirtyRegions[j])) {
            return true;
        }
    }

    return false;
}

void FbContext::drawWindow(Window *win) {
    applyBoundClipping(win);

    // Remove child window clipped rectangles
    for (int i = 0; i < win->numWindows; i++) {
        Window *child = win->windows[i];
        renderRegion->reshapeRegion(child->winRect);
    }

    // Draw self
    win->drawObject();

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
void FbContext::createDirtyWindowRegions(Window *win, Window* dirtyAncestor) {
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

                FbContext *context = FbContext::getInstance();

                // Add new and old location of window to dirty list
                context->dirtyRegion->addClippedRect(dirtyAncestor->getWinRect());
                context->dirtyRegion->addClippedRect(dirtyAncestor->getPrevRect());
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

#define MOUSE_W                             11
#define MOUSE_H                             18
void FbContext::createDirtyMouseRegion(vec2 *mouse, vec2 *oldMouse) {
    if (!oldMouse->equals(mouse)) {
        // Original mouse position
        Rect old(oldMouse->x, oldMouse->y, MOUSE_W, MOUSE_H);
        dirtyRegion->addClippedRect(&old);

        // New mouse position
        Rect newMouse(mouse->x, mouse->y, MOUSE_W, MOUSE_H);
        dirtyRegion->addClippedRect(&newMouse);

        *oldMouse = *mouse;
    }
}

extern uint32_t mouseBitmap[MOUSE_W * MOUSE_H];
void FbContext::render(Window *root, vec2 *mouse, vec2 *oldMouse) {
    // Add any Window state change to list of regions to re-render
    // for the compositor
    createDirtyWindowRegions(root, nullptr);

    // If mouse position has changes since last refresh, patch old position
    // and draw new position
    createDirtyMouseRegion(mouse, oldMouse);

    // Only refresh if dirty regions generated
    if (dirtyRegion->getNumClipped()) {
        // Draw the windows that intersect the dirty clipped regions
        drawWindow(root);

        // Draw mouse on top of final image, does not use clipped regions
        Rect mouseRect(mouse->x, mouse->y, MOUSE_W, MOUSE_H);
        drawBuffRaw(mouseRect, mouseBitmap);

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