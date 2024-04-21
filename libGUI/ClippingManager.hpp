#ifndef CLIPPINGMANAGER_H
#define CLIPPINGMANAGER_H

#include "Rect.hpp"

namespace GUI {

class ClippingManager {
public:
    ClippingManager(void);
    ~ClippingManager(void);

    // void intersectClippedRect(Rect *rect);

    /// @brief Add Rect to clipped region
    /// @param rect The Rect to add
    void addClippedRegion(Rect *rect);

    /// @brief Remove Rect in clipped region
    /// @param rect 
    void removeClippedRegion(Rect *rect);

    int getNumClipped(void);

    Rect *getClippedRegions(void);

    /// @brief Reset the internal clipped Rect list
    void resetClippedList(void);

private:
    /// @brief Add a clipp Rect to clippedRects list
    void pushRect(Rect *rect);

    /// @brief Delete the clipped Rect at index in clippedRects list
    void deleteRect(int index);

    int numClipped; // Number of clipped Rects in clipped list
    int maxClipped; // Max allowed clipped Rects in clipped list
    Rect *clippedRects; // The clipped Rect list
};

};

#endif // CLIPPINGMANAGER_H