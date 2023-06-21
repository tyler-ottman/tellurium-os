#pragma once

#include <stdint.h>
#include "Windowing/Rect.hpp"

#define CLIPPED_MAX                         200

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
    void drawVerticalLine(int xPos, int yPos, int length, int color);
    void drawHorizontalLine(int xPos, int yPos, int length, int color);
    void drawOutlinedRect(int xPos, int yPos, int width, int length, int color);

    // Clipped Rectangle Operations
    void addClippedRect(Rect *rect);
    void reshapeRegion(Rect *rect);
    void appendClippedRect(Rect *rect);
    void removeClippedRect(int index);
    void resetClippedList(void);
    void drawClippedRegions(void);

    int getRegions(void) {return numRegions;}
private:
    // Clipped region info
    int numRegions;
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
};

}