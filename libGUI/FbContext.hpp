#ifndef FBCONTEXT_H
#define FBCONTEXT_H

#include <stdint.h>

#include "ClippingManager.hpp"
#include "Rect.hpp"
#include "libGUI/Window.hpp"

namespace GUI {

typedef struct {
    void *fb_buff;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint32_t fb_bpp;
} FbMeta;

class FbContext {
public:
    FbContext(FbContext& obj) = delete;
    static FbContext *getInstance(void);
    FbMeta *getFbContext(void);
    void *getFbBuff(void) { return fb_meta.fb_buff; }

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
    void createDirtyMouseRegion(vec2 *mouse, vec2 *oldMouse);

    /// @brief Render to final buffer
    /// @param root Top of Window structure
    /// @param mouse Current position of mouse
    /// @param oldMouse Old position of mouse
    void render(Window *root, vec2 *mouse, vec2 *oldMouse);

private:
    FbMeta fb_meta;
    uint32_t *fb_buff;

    Rect *screen;
    ClippingManager *renderRegion; // Regions to render to screen
    ClippingManager *dirtyRegion; // Dirty regions that need to be rendered

    static FbContext *instance;
    FbContext(void);
    ~FbContext(void);

    // Draw a clipped buffer within bounds of Rect area
    void drawClippedBuff(Rect &rect, uint32_t *buff, Rect *area);
    // void drawClippedRegions(void);
};

}

#endif // FBCONTEXT_H