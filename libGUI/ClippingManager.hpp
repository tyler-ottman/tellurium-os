#ifndef CLIPPINGMANAGER_H
#define CLIPPINGMANAGER_H

#include "Rect.hpp"

namespace GUI {

class ClippingManager {
public:
    ClippingManager(void);
    ~ClippingManager(void);

    void intersectClippedRect(Rect *rect);

    void addClippedRect(Rect *rect);
    void reshapeRegion(Rect *rect);

    // List operations
    void appendClippedRect(Rect *rect);
    void removeClippedRect(int index);
    void resetClippedList(void);

    int getNumClipped(void);

    Rect *getClippedRegions(void);

private:
    int numClipped;
    int maxClipped;
    Rect *clippedRects;
};

};

#endif // CLIPPINGMANAGER_H