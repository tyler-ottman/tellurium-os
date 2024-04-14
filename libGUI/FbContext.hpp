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
    void drawRect(Rect &rect, uint32_t color);
    void drawBuff(Rect &rect, uint32_t *buff);
    void drawRectNoRegion(Rect &rect, uint32_t color);
    void drawBitmapNoRegion(Rect &rect, uint32_t *bitmap);

    // void drawClippedRegions(void);

    // Clipped Region that is rendered to screen
    ClippingManager *renderRegion;

    // Clipped Region that is dirty and needs to be re-rendered
    ClippingManager *dirtyRegion;

    void applyBoundClipping(Window *win);
    bool rectIntersectsDirty(Rect *rect);
    void drawWindow(Window *win);

private:
    FbMeta fb_meta;
    uint32_t *fb_buff;

    Rect *screen;

    static FbContext *instance;
    FbContext(void);
    ~FbContext();

    // Draw a clipped rectangle within the bounds of Rect area
    void drawClippedRect(Rect &rect, uint32_t color, Rect *area);

    // Draw a clipped buffer within bounds of Rect area
    void drawClippedBuff(Rect &rect, uint32_t *buff, Rect *area);

    // Draw a clipped bitmap within bounds of Rect area
    void drawClippedBitmap(Rect &rect, uint32_t *bitmap, Rect *area);

    
    
};

}

#endif // FBCONTEXT_H