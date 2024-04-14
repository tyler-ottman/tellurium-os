#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "ClippingManager.hpp"
#include "Window.hpp"

namespace GUI {

class Compositor {
public:
    Compositor(FbInfo *context);
    ~Compositor(void);

    // Framebuffer operations
    void drawBuff(Rect &rect, uint32_t *buff);
    void drawBuffRaw(Rect &rect, uint32_t *bitmap);

    void applyBoundClipping(Window *win);
    bool rectIntersectsDirty(Rect *rect);
    void drawWindow(Window *win);

    /// @brief Generate dirty regions from Window tree structure
    /// @param win Cur traversed window
    /// @param dirtyAncestor Highest ancestor of win marked as dirty
    void createDirtyWindowRegions(Window *win, Window* dirtyAncestor);

    /// @brief Generate dirty regions for the Mouse
    /// @param mouse Current position of mouse
    /// @param oldMouse Old position of mouse
    void createDirtyMouseRegion(Window *mouse);

    /// @brief Render to final buffer
    /// @param root Top of Window structure
    /// @param mouse Current position of mouse
    /// @param oldMouse Old position of mouse
    void render(Window *root, Window *mouse);

private:
    FbInfo *fbInfo;

    Rect *screen;
    ClippingManager *renderRegion; // Regions to render to screen
    ClippingManager *dirtyRegion; // Dirty regions that need to be rendered    

    // Draw a clipped buffer within bounds of Rect area
    void drawClippedBuff(Rect &rect, uint32_t *buff, Rect *area);
    // void drawClippedRegions(void);
};

};

#endif // COMPOSITOR_H