#pragma once

#include <stdint.h>
#include "Rect.hpp"

#define CLIPPED_MAX                         200
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

namespace GUI {

typedef struct {
    void *fb_buff;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
} FbMeta;

class FbContext {
public:
    FbContext(FbContext& obj) = delete;
    static FbContext *getInstance(void);
    FbMeta *getFbContext(void);

    // Framebuffer operations
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawBuff(int x, int y, int width, int height, uint32_t *buff);
    void drawRectNoRegion(int x, int y, int width, int height, uint32_t color);
    void drawVerticalLine(int xPos, int yPos, int length, int color);
    void drawHorizontalLine(int xPos, int yPos, int length, int color);
    void drawOutlinedRect(int xPos, int yPos, int width, int length, int color);

    // Translations
    uint32_t translateLightColor(uint32_t color);

    // Clipped Rectangle Operations
    void addClippedRect(Rect *rect);
    void reshapeRegion(Rect *rect);
    void drawClippedRegions(void);
    void intersectClippedRect(Rect *rect);

    // Dirty Rectangle Operations
    void moveClippedToDirty(void);

    // List operations
    void appendClippedRect(Rect *rect);
    void removeClippedRect(int index);
    void resetClippedList(void);

    void appendDirtyRect(Rect *rect);
    void removeDirtyRect(int index);
    void resetDirtyList(void);

    int getNumClipped(void) { return numClipped; }
    int getNumDirty(void) { return numDirty; }

    Rect *getDirtyRegions(void) { return dirtyRects; }
private:
    // Dirty region info
    int numDirty;
    Rect *dirtyRects;

    // Clipped region info
    bool clippingEnabled;
    int numClipped;
    Rect *clippedRects;

    FbMeta fb_meta;
    uint32_t *fb_buff;

    Rect *screen;

    static FbContext *instance;
    FbContext(void);
    ~FbContext();

    // Draw a clipped rectangle within the bounds of Rect area
    void drawClippedRect(int x, int y, int width, int height, uint32_t color,
                         Rect *area);

    // Draw a clipped buffer within bounds of Rect area
    void drawClippedBuff(int x, int y, int width, int height, uint32_t *buff,
                         Rect *area);
};
}