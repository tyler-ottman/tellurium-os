#include "ClippingManager.hpp"

namespace GUI {

ClippingManager::ClippingManager() : numClipped(0), maxClipped(200) {
    clippedRects = new Rect[maxClipped];
}

ClippingManager::~ClippingManager() {
    delete clippedRects;
}

void ClippingManager::intersectClippedRect(Rect *rect) {
    int appendedRects = 0;
    for (int i = 0; i < numClipped; i++) {
        Rect *currentRect = &clippedRects[i];
        Rect intersectRect;

        bool ret = currentRect->getOverlapRect(&intersectRect, rect);
        if (ret) {
            clippedRects[appendedRects++] = intersectRect;
        }
    }

    numClipped = appendedRects;
}

void ClippingManager::addClippedRect(Rect *rect) {
    reshapeRegion(rect);

    appendClippedRect(rect);
}

void ClippingManager::reshapeRegion(Rect *rect) {
    for (int i = 0; i < numClipped; i++) {
        Rect *clipped = &clippedRects[i];

        if (!clipped->intersects(rect)) {
            continue;
        }

        Rect splitRects[4];
        int count = clipped->getSplitRects(splitRects, rect);

        // Update and split selected clipped region
        removeClippedRect(i);

        for (int j = 0; j < count; j++) {
            appendClippedRect(&splitRects[j]);
        }

        i = -1;
    }
}

void ClippingManager::appendClippedRect(Rect *rect) {
    if (numClipped >= maxClipped) {
        return;
    }

    clippedRects[numClipped++] = *rect;
}

void ClippingManager::removeClippedRect(int index) {
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