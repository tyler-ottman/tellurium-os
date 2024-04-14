#ifndef FBCONTEXT_H
#define FBCONTEXT_H

#include <stdint.h>

#include "ClippingManager.hpp"
#include "Rect.hpp"
#include "libGUI/Window.hpp"

namespace GUI {



class FbContext {
public:
    static FbContext *getInstance(void);

    struct FbInfo {
        void *fb_buff;
        uint32_t fb_width;
        uint32_t fb_height;
        uint32_t fb_pitch;
        uint32_t fb_bpp;
        FbInfo() : fb_buff(nullptr),
              fb_width(0),
              fb_height(0),
              fb_pitch(0),
              fb_bpp(0) {}
    };

    FbInfo fbInfo;

private:
    FbContext(void);
    ~FbContext(void);

    static FbContext *instance;
};

class Compositor {
public:
    Compositor(FbContext *context);
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
    FbContext *context;

    Rect *screen;
    ClippingManager *renderRegion; // Regions to render to screen
    ClippingManager *dirtyRegion; // Dirty regions that need to be rendered    

    // Draw a clipped buffer within bounds of Rect area
    void drawClippedBuff(Rect &rect, uint32_t *buff, Rect *area);
    // void drawClippedRegions(void);
};

}

#endif // FBCONTEXT_H