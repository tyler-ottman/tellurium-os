#include "Compositor.hpp"

namespace GUI {

Compositor::Compositor(Surface *surface) : fSurface(surface) {    
    fRenderClips = new ClipRegions();
    bRenderClips = new ClipRegions();

    bSurface = new Surface;
    bSurface->rect = surface->rect;
    bSurface->buff = new uint32_t[surface->getSize()];
}

Compositor::~Compositor() {
    delete fRenderClips;
    delete bRenderClips;
    delete bSurface;
}

void Compositor::render(Window *root, Window *mouse) {
    // Add any Window state change to list of regions to re-render
    // for the compositor
    createDirtyWindowRegions(root, nullptr);

    // If mouse position has changes since last refresh, patch old position
    // and draw new position
    createDirtyMouseRegion(mouse);

    // Only refresh if dirty regions generated
    if (bRenderClips->getNumClipped()) {
        fRenderClips->copyRegion(bRenderClips);

        // Draw the windows that intersect the dirty clipped regions
        drawWindow(root);

        // Draw mouse on top of final image, does not use clipped regions
        bSurface->blit(*mouse->surface, &mouse->surface->rect);

        // Copy backbuffer surface to main surface;
        int nRects = fRenderClips->getNumClipped();
        for (int i = 0; i < nRects; i++) {
            fSurface->blit(*bSurface, &fRenderClips->clippedRects[i].rect);
        }

        // Rendering done, reset dirty/render regions list
        fRenderClips->resetClippedList();
        bRenderClips->resetClippedList();
    }
}

void Compositor::createDirtyWindowRegions(Window *win, Window *dirtyAncestor) {
    // If window is marked as dirty, add it to dirty list
    // for the compositor
    if (win->isDirty()) {
        // Reset dirty flag because it won't be dirty after refresh
        win->setDirty(false);

        // This is an optimization, dirty child Windows bounded by a dirty
        // ancestor (parent or above) Window do not need to be added to
        // the dirty list for the compositor, because the ancestor already
        // covers the region
        if (!dirtyAncestor) {
            dirtyAncestor = win;

            // Add new and old location of window to region to render
            bRenderClips->addClippedRegion(dirtyAncestor->getWinRect(), win);
            bRenderClips->addClippedRegion(dirtyAncestor->getPrevRect(), win);
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

        // Add new and old location of mouse to region to render
        bRenderClips->addClippedRegion(mouse->getPrevRect(), mouse);
        // bRenderClips->addClippedRegion(mouse->getWinRect(), mouse);

        // Now store current position of mouse in previous position
        mouse->updatePrevRect();
    }
}

// The Window structure is sorted by priority, so the surfaces can be drawn by
// depth, starting with the shallowest depth (Z-Depth Buffering). Once a surface
// at a given depth is drawn, any surface at a deeper depth will be occluded.
void Compositor::drawWindow(Window *win) {
    for (int i = win->numWindows - 1; i >= 0; i--) {
        drawWindow(win->windows[i]);
    }

    // Window is not marked as visible, do not render
    if (!win->isVisible()) {
        return;
    }

    // Draw self (TODO: put in surface)
    for (int i = 0; i < bRenderClips->numClipped; i++) {
        ClipRect *clippedRect = &bRenderClips->clippedRects[i];

        // The clipped region is marked as transparent, perform alpha blit
        if (clippedRect->win->isTransparent()) {
            bSurface->alphaBlit(*win->surface, &clippedRect->rect);
        }

        // The clipped region is not marked as transparent, perform normal blit
        else {
            bSurface->blit(*win->surface, &clippedRect->rect);
        }
    }

    // The Window just drawn is transparent, existing parts of clipped
    // region must be marked as transparent to indicate to the Compositor
    // that the surface must be alpha blended with the existing surface
    if (win->isTransparent()) {
        bRenderClips->divideClippedRegion(&win->surface->rect, win);
    }

    // If the Window just drawn was opaque, existing parts of the clipped
    // region must be deleted to indicate to the Compositor that surfaces
    // beneath the Window will be occluded
    else {
        bRenderClips->removeClippedRegion(&win->surface->rect);
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