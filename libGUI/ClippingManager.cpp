#include "ClippingManager.hpp"

namespace GUI {

ClippingManager::ClippingManager() : numClipped(0), maxClipped(200) {
    clippedRects = new Rect[maxClipped];
}

ClippingManager::~ClippingManager() {
    delete clippedRects;
}

void ClippingManager::removeClippedRegion(Rect *rect) {
    for (int i = 0; i < numClipped; i++) {
        Rect *clipped = &clippedRects[i];

        if (!clipped->intersects(rect)) {
            continue;
        }

        Rect splitRects[4];
        int count = clipped->getSplitRects(splitRects, rect);

        // Update and split selected clipped region
        deleteRect(i);

        for (int j = 0; j < count; j++) {
            pushRect(&splitRects[j]);
        }

        i = -1; // Resets to 0 next iteration
    }
}

void ClippingManager::addClippedRegion(Rect *rect) {
    // First, punch a hole in the clipped list
    removeClippedRegion(rect);

    // Second, now just add the Rect
    pushRect(rect);
}

void ClippingManager::pushRect(Rect *rect) {
    if (numClipped >= maxClipped) {
        return;
    }

    clippedRects[numClipped++] = *rect;
}

void ClippingManager::deleteRect(int index) {
    if (index < 0 || index >= numClipped) {
        return;
    }

    for (int i = index; i < numClipped - 1; i++) {
        clippedRects[i] = clippedRects[i + 1];
    }

    numClipped--;
}

void ClippingManager::resetClippedList() {
    numClipped = 0;
}

int ClippingManager::getNumClipped() { return numClipped; }

Rect *ClippingManager::getClippedRegions() { return clippedRects; }

};